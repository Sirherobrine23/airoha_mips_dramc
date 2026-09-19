// SPDX-License-Identifier: GPL-2.0+
/*
 * EcoNet/Airoha MIPS host-side image utility.
 *
 * Replaces the former Python helpers with one dependency-free C program:
 *   econet-image chainloader --image IN [--output OUT]
 *   econet-image flash --soc SOC --stages FILE --symbols FILE \
 *       --uboot FILE --load ADDR --output FILE [--minfo FILE]
 *   econet-image tcboot --soc SOC --image FILE [--output FILE] [--stock FILE]
 *   econet-image selftest
 */

#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define XMODEM_BLOCK            128u
#define CRC_TABLE_ENTRIES       96u
#define CRC_TABLE_SIZE          (CRC_TABLE_ENTRIES * 4u)

#define FLASH_LIMIT             0x100000u
#define FLASH_PAYLOAD           0x20000u
#define FLASH_CRC               (FLASH_PAYLOAD - 4u)
#define FLASH_MINFO             0xff00u

#define IH_MAGIC                0x27051956u
#define IH_HEADER_SIZE          0x40u
#define BOOT_IMAGE_SIZE         0x100000u
#define TCBOOT_MAGIC_OFFSET     0x0cu
#define CRC1_OFFSET             0x1fffcu
#define CRC2_OFFSET             0x3fdfcu

struct blob {
    uint8_t *data;
    size_t len;
};

struct symbol {
    char name[64];
    uint32_t value;
};

struct tcboot_cfg {
    const char *soc;
    bool big_endian;
    uint32_t spl_offset;
    uint32_t spl_limit;
    uint32_t ddr_offset;
    uint32_t ddr_size;
    uint32_t tcboot_len;
    bool spl_is_bootram;
    uint32_t crc_offsets[2];
    unsigned int crc_count;
    uint32_t minfo_offset;
    uint32_t minfo_size;
    bool extended_header;
};

static const struct tcboot_cfg tcboot_cfgs[] = {
    {
        .soc = "en751221", .big_endian = true,
        .spl_offset = 0x10000, .spl_limit = CRC1_OFFSET,
        .ddr_offset = 0x8000, .ddr_size = 0x4f70,
        .tcboot_len = 0, .spl_is_bootram = true,
        .crc_offsets = { CRC1_OFFSET }, .crc_count = 1,
        .minfo_offset = 0xff00, .minfo_size = 0x100,
        .extended_header = false,
    },
    {
        .soc = "en751627", .big_endian = true,
        .spl_offset = 0x10000, .spl_limit = CRC1_OFFSET,
        .ddr_offset = 0x2000, .ddr_size = 0xd050,
        .tcboot_len = 0x40000, .spl_is_bootram = false,
        .crc_offsets = { CRC1_OFFSET, CRC2_OFFSET }, .crc_count = 2,
        .minfo_offset = 0x3fe00, .minfo_size = 0x200,
        .extended_header = true,
    },
    {
        .soc = "en7528", .big_endian = false,
        .spl_offset = 0x10000, .spl_limit = 0x1fff8,
        .ddr_offset = 0x4000, .ddr_size = 0xb000,
        .tcboot_len = 0x40000, .spl_is_bootram = false,
        .crc_offsets = { CRC1_OFFSET, CRC2_OFFSET }, .crc_count = 2,
        .minfo_offset = 0x3fe00, .minfo_size = 0x200,
        .extended_header = true,
    },
    {
        .soc = "en7580", .big_endian = false,
        .spl_offset = 0x10000, .spl_limit = 0x1fff8,
        .ddr_offset = 0x4000, .ddr_size = 0xb000,
        .tcboot_len = 0x40000, .spl_is_bootram = false,
        .crc_offsets = { CRC1_OFFSET, CRC2_OFFSET }, .crc_count = 2,
        .minfo_offset = 0x3fe00, .minfo_size = 0x200,
        .extended_header = true,
    },
};

static void die(const char *msg)
{
    fprintf(stderr, "econet-image: %s\n", msg);
    exit(EXIT_FAILURE);
}

static void die_errno(const char *path)
{
    fprintf(stderr, "econet-image: %s: %s\n", path, strerror(errno));
    exit(EXIT_FAILURE);
}

static uint32_t get_be32(const uint8_t *p)
{
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | p[3];
}

static void put_be32(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)(v >> 24);
    p[1] = (uint8_t)(v >> 16);
    p[2] = (uint8_t)(v >> 8);
    p[3] = (uint8_t)v;
}

static void put_le32(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)v;
    p[1] = (uint8_t)(v >> 8);
    p[2] = (uint8_t)(v >> 16);
    p[3] = (uint8_t)(v >> 24);
}

static void put_native32(uint8_t *p, uint32_t v, bool big_endian)
{
    if (big_endian)
        put_be32(p, v);
    else
        put_le32(p, v);
}

/* IEEE CRC32, same result as Python zlib.crc32(data). */
static uint32_t crc32_ieee(const uint8_t *data, size_t len)
{
    uint32_t crc = 0xffffffffu;
    size_t i;
    unsigned int bit;

    for (i = 0; i < len; i++) {
        crc ^= data[i];
        for (bit = 0; bit < 8; bit++)
            crc = (crc >> 1) ^ (0xedb88320u & (0u - (crc & 1u)));
    }

    return crc ^ 0xffffffffu;
}

static struct blob read_file(const char *path)
{
    FILE *f;
    struct stat st;
    struct blob b = { 0 };

    if (stat(path, &st) < 0)
        die_errno(path);
    if (st.st_size < 0)
        die("negative file size");

    b.len = (size_t)st.st_size;
    b.data = malloc(b.len ? b.len : 1);
    if (!b.data)
        die("out of memory");

    f = fopen(path, "rb");
    if (!f)
        die_errno(path);
    if (b.len && fread(b.data, 1, b.len, f) != b.len) {
        fclose(f);
        die_errno(path);
    }
    if (fclose(f) != 0)
        die_errno(path);

    return b;
}

static void write_file(const char *path, const void *data, size_t len)
{
    FILE *f = fopen(path, "wb");

    if (!f)
        die_errno(path);
    if (len && fwrite(data, 1, len, f) != len) {
        fclose(f);
        die_errno(path);
    }
    if (fclose(f) != 0)
        die_errno(path);
}

static void free_blob(struct blob *b)
{
    free(b->data);
    b->data = NULL;
    b->len = 0;
}

static uint32_t parse_u32(const char *s)
{
    char *end;
    unsigned long v;

    errno = 0;
    v = strtoul(s, &end, 0);
    if (errno || *s == '\0' || *end != '\0' || v > UINT32_MAX)
        die("invalid 32-bit number");
    return (uint32_t)v;
}

static const struct tcboot_cfg *find_tcboot_cfg(const char *soc)
{
    size_t i;

    for (i = 0; i < ARRAY_SIZE(tcboot_cfgs); i++)
        if (!strcmp(tcboot_cfgs[i].soc, soc))
            return &tcboot_cfgs[i];
    return NULL;
}

static int finalize_chainloader(const char *input, const char *output)
{
    struct blob in = read_file(input);
    size_t check_start, chunks, padded_len, i;
    uint8_t *out;

    if (in.len <= CRC_TABLE_SIZE) {
        free_blob(&in);
        fprintf(stderr, "econet-image: chainloader image too short\n");
        return 1;
    }

    check_start = in.len - CRC_TABLE_SIZE;
    chunks = (check_start + XMODEM_BLOCK - 1) / XMODEM_BLOCK;
    if (chunks > CRC_TABLE_ENTRIES) {
        free_blob(&in);
        fprintf(stderr, "econet-image: chainloader CRC table too small\n");
        return 1;
    }

    padded_len = (in.len + XMODEM_BLOCK - 1) & ~(size_t)(XMODEM_BLOCK - 1);
    out = calloc(1, padded_len);
    if (!out)
        die("out of memory");
    memcpy(out, in.data, in.len);
    memset(out + check_start, 0, CRC_TABLE_SIZE);

    for (i = 0; i < chunks; i++) {
        size_t start = i * XMODEM_BLOCK;
        size_t end = start + XMODEM_BLOCK;
        uint32_t crc;

        if (end > check_start)
            end = check_start;
        crc = crc32_ieee(out + start, end - start);
        put_be32(out + check_start + i * 4, crc);
    }

    write_file(output ? output : input, out, padded_len);
    printf("  CHAIN      %s: checked=0x%zx blocks=%zu table=%u size=%zu\n",
           output ? output : input, check_start, chunks,
           CRC_TABLE_ENTRIES, padded_len);

    free(out);
    free_blob(&in);
    return 0;
}

static bool read_symbols(const char *path, struct symbol **out_syms, size_t *out_count)
{
    FILE *f = fopen(path, "r");
    struct symbol *syms = NULL;
    size_t count = 0, cap = 0;
    char line[512];

    if (!f)
        die_errno(path);

    while (fgets(line, sizeof(line), f)) {
        char addr[64], type[16], name[128], *end;
        unsigned long value;
        struct symbol *tmp;

        if (sscanf(line, "%63s %15s %127s", addr, type, name) != 3)
            continue;
        errno = 0;
        value = strtoul(addr, &end, 16);
        if (errno || *end || value > UINT32_MAX)
            continue;
        if (count == cap) {
            cap = cap ? cap * 2 : 32;
            tmp = realloc(syms, cap * sizeof(*syms));
            if (!tmp)
                die("out of memory");
            syms = tmp;
        }
        if (strlen(name) >= sizeof(syms[count].name))
            continue;
        memcpy(syms[count].name, name, strlen(name) + 1);
        syms[count].value = (uint32_t)value;
        count++;
    }

    if (fclose(f) != 0)
        die_errno(path);
    *out_syms = syms;
    *out_count = count;
    return true;
}

static bool lookup_symbol(const struct symbol *syms, size_t count,
                          const char *name, uint32_t *value)
{
    size_t i;

    for (i = 0; i < count; i++) {
        if (!strcmp(syms[i].name, name)) {
            *value = syms[i].value;
            return true;
        }
    }
    return false;
}

static int stage_interval(const struct symbol *syms, size_t nsyms,
                          const char *name, size_t stages_len,
                          uint32_t *start, uint32_t *end)
{
    char sym_start[96], sym_end[96];
    uint32_t a, b;

    snprintf(sym_start, sizeof(sym_start), "__%s_start", name);
    snprintf(sym_end, sizeof(sym_end), "__%s_end", name);
    if (!lookup_symbol(syms, nsyms, sym_start, &a) ||
        !lookup_symbol(syms, nsyms, sym_end, &b)) {
        fprintf(stderr, "econet-image: missing %s interval symbols\n", name);
        return -1;
    }
    if (a < 0xbfc00000u || b < 0xbfc00000u) {
        fprintf(stderr, "econet-image: invalid %s interval\n", name);
        return -1;
    }
    a -= 0xbfc00000u;
    b -= 0xbfc00000u;
    if (a < 0x60 || a >= b || b > stages_len) {
        fprintf(stderr, "econet-image: invalid %s interval\n", name);
        return -1;
    }
    *start = a;
    *end = b;
    return 0;
}

static int validate_uboot(const struct blob *uboot, uint32_t load, bool quiet)
{
    uint8_t header[64];
    uint32_t magic, hcrc, size, addr, entry, dcrc;

    if (uboot->len < 64 || uboot->len > FLASH_LIMIT - FLASH_PAYLOAD) {
        if (!quiet)
            fprintf(stderr, "econet-image: U-Boot must fit in the remaining 896 KiB\n");
        return -1;
    }

    magic = get_be32(uboot->data + 0);
    hcrc = get_be32(uboot->data + 4);
    size = get_be32(uboot->data + 12);
    addr = get_be32(uboot->data + 16);
    entry = get_be32(uboot->data + 20);
    dcrc = get_be32(uboot->data + 24);

    memcpy(header, uboot->data, sizeof(header));
    memset(header + 4, 0, 4);
    if (magic != IH_MAGIC || crc32_ieee(header, sizeof(header)) != hcrc) {
        if (!quiet)
            fprintf(stderr, "econet-image: invalid U-Boot legacy header/CRC\n");
        return -1;
    }
    if (size != uboot->len - 64 || crc32_ieee(uboot->data + 64, uboot->len - 64) != dcrc) {
        if (!quiet)
            fprintf(stderr, "econet-image: invalid U-Boot payload size/CRC\n");
        return -1;
    }
    if (addr != load || entry != load ||
        uboot->data[29] != 5 || uboot->data[30] != 5 || uboot->data[31] != 0) {
        if (!quiet)
            fprintf(stderr, "econet-image: expected uncompressed MIPS firmware at configured TEXT_BASE\n");
        return -1;
    }
    if ((uint64_t)load + size > 0x82000000ull) {
        if (!quiet)
            fprintf(stderr, "econet-image: payload exceeds the minimum 32 MiB DRAM window\n");
        return -1;
    }
    return 0;
}

static int pack_flash_image(const char *soc, const char *stages_path,
                            const char *symbols_path, const char *uboot_path,
                            uint32_t load, const char *minfo_path,
                            const char *output)
{
    struct blob stages = read_file(stages_path);
    struct blob uboot = read_file(uboot_path);
    struct blob minfo = { 0 };
    struct symbol *syms = NULL;
    size_t nsyms = 0;
    uint8_t *image = NULL;
    bool big_endian;
    uint32_t move_s, move_e, boot2_s, boot2_e;
    uint32_t loader_s, loader_e, ddr_s, ddr_e;
    int ret = 1;

    if (!strcmp(soc, "en751221") || !strcmp(soc, "en751627"))
        big_endian = true;
    else if (!strcmp(soc, "en7528"))
        big_endian = false;
    else {
        fprintf(stderr, "econet-image: unsupported flash SoC: %s\n", soc);
        goto out;
    }

    if (load < 0x81000000u || load >= 0x82000000u) {
        fprintf(stderr, "econet-image: load address must be in 0x81000000..0x81ffffff\n");
        goto out;
    }
    if (stages.len >= FLASH_MINFO || stages.len < 0x60) {
        fprintf(stderr, "econet-image: initial stages overlap mi.conf or are incomplete\n");
        goto out;
    }
    if (validate_uboot(&uboot, load, false) < 0)
        goto out;

    read_symbols(symbols_path, &syms, &nsyms);
    if (stage_interval(syms, nsyms, "move_data", stages.len, &move_s, &move_e) < 0 ||
        stage_interval(syms, nsyms, "boot2", stages.len, &boot2_s, &boot2_e) < 0 ||
        stage_interval(syms, nsyms, "lzma", stages.len, &loader_s, &loader_e) < 0 ||
        stage_interval(syms, nsyms, "spram", stages.len, &ddr_s, &ddr_e) < 0)
        goto out;

    if (move_e >= 0x800 || !(move_e <= boot2_s && boot2_s < boot2_e &&
                              boot2_e <= loader_s && loader_s < loader_e &&
                              loader_e <= ddr_s)) {
        fprintf(stderr, "econet-image: invalid first-page placement or overlapping stages\n");
        goto out;
    }

    if (minfo_path) {
        minfo = read_file(minfo_path);
        if (minfo.len != 0x100) {
            fprintf(stderr, "econet-image: mi.conf must be exactly 256 bytes\n");
            goto out;
        }
    }

    image = malloc(FLASH_LIMIT);
    if (!image)
        die("out of memory");
    memset(image, 0xff, FLASH_LIMIT);
    memcpy(image, stages.data, stages.len);

    if (!strcmp(soc, "en751221")) {
        if (get_be32(image) != 0x0bf00012u || memcmp(image + 0x40, "\0\0\0\0\0\0\0\0", 8)) {
            fprintf(stderr, "econet-image: unexpected EN751221 SDK reset/header layout\n");
            goto out;
        }
        put_be32(image, 0x0bf00010u);
    } else {
        memset(image + 8, 0, 0x58);
    }

#define PUT_WORD(off, val) put_native32(image + (off), (val), big_endian)
    if (strcmp(soc, "en751221"))
        PUT_WORD(8, FLASH_PAYLOAD);
    memcpy(image + 12, "6578", 4);
    PUT_WORD(0x10, loader_s);
    PUT_WORD(0x14, loader_e);
    PUT_WORD(0x18, FLASH_PAYLOAD);
    PUT_WORD(0x1c, FLASH_PAYLOAD + (uint32_t)uboot.len);

    if (strcmp(soc, "en751221")) {
        PUT_WORD(0x28, 0x00040010u);
        PUT_WORD(0x30, 0x9fa30000u);
        PUT_WORD(0x34, 0x80000000u);
        if (!strcmp(soc, "en7528"))
            PUT_WORD(0x40, 0x035a3c96u);
        PUT_WORD(0x50, boot2_s);
        PUT_WORD(0x54, boot2_e);
        PUT_WORD(0x58, ddr_s);
        PUT_WORD(0x5c, ddr_e);
    }

    if (!strcmp(soc, "en751627")) {
        static const uint8_t zeros[8] = { 0 };
        if (memcmp(image + ddr_s, zeros, sizeof(zeros))) {
            fprintf(stderr, "econet-image: EN751627 DDR entry prefix is not reserved zero space\n");
            goto out;
        }
        put_be32(image + ddr_s, 0x0be8c0a0u);
        put_be32(image + ddr_s + 4, 0);
    }

    if (minfo_path)
        memcpy(image + FLASH_MINFO, minfo.data, 0x100);
    else
        memset(image + FLASH_MINFO, 0, 0x100);

    PUT_WORD(0xffb0, 0x50414745u);
    if (!strcmp(soc, "en7528"))
        PUT_WORD(0x1fff8, 0x50414745u);

    memcpy(image + FLASH_PAYLOAD, uboot.data, uboot.len);
    PUT_WORD(FLASH_CRC, crc32_ieee(image, FLASH_CRC) ^ 0xffffffffu);
#undef PUT_WORD

    write_file(output, image, FLASH_LIMIT);
    ret = 0;
out:
    free(image);
    free(syms);
    free_blob(&minfo);
    free_blob(&uboot);
    free_blob(&stages);
    return ret;
}

static int finalize_tcboot(const char *soc, const char *image_path,
                           const char *output_path, const char *stock_path)
{
    const struct tcboot_cfg *cfg = find_tcboot_cfg(soc);
    struct blob image = { 0 }, stock = { 0 };
    uint32_t payload_size, spl_end;
    unsigned int i;
    int ret = 1;

    if (!cfg) {
        fprintf(stderr, "econet-image: unsupported SoC '%s'\n", soc);
        return 1;
    }

    image = read_file(image_path);
    if (image.len != BOOT_IMAGE_SIZE) {
        fprintf(stderr, "econet-image: combined image must be exactly 0x%x bytes, got 0x%zx\n",
                BOOT_IMAGE_SIZE, image.len);
        goto out;
    }
    if (memcmp(image.data + TCBOOT_MAGIC_OFFSET, "6578", 4)) {
        fprintf(stderr, "econet-image: TCBoot magic '6578' is missing at offset 0x0c\n");
        goto out;
    }

    if (stock_path) {
        stock = read_file(stock_path);
        if (stock.len < (size_t)cfg->minfo_offset + cfg->minfo_size) {
            fprintf(stderr, "econet-image: stock bootloader is too small for manufacturing data\n");
            goto out;
        }
        memcpy(image.data + cfg->minfo_offset, stock.data + cfg->minfo_offset,
               cfg->minfo_size);
    }

    if (cfg->spl_offset + IH_HEADER_SIZE > image.len ||
        get_be32(image.data + cfg->spl_offset) != IH_MAGIC) {
        fprintf(stderr, "econet-image: invalid SPL legacy image header at 0x%x\n",
                cfg->spl_offset);
        goto out;
    }
    payload_size = get_be32(image.data + cfg->spl_offset + 12);
    if (!payload_size) {
        fprintf(stderr, "econet-image: SPL legacy image has an empty payload\n");
        goto out;
    }
    spl_end = cfg->spl_offset + IH_HEADER_SIZE + payload_size;
    if (spl_end > cfg->spl_limit) {
        fprintf(stderr, "econet-image: SPL ends at 0x%x, beyond TCBoot limit 0x%x\n",
                spl_end, cfg->spl_limit);
        goto out;
    }

    put_native32(image.data + 0x08, cfg->tcboot_len, cfg->big_endian);
    if (cfg->spl_is_bootram) {
        put_native32(image.data + 0x10, 0, cfg->big_endian);
        put_native32(image.data + 0x14, 0, cfg->big_endian);
        put_native32(image.data + 0x18, cfg->spl_offset, cfg->big_endian);
        put_native32(image.data + 0x1c, spl_end, cfg->big_endian);
    } else {
        put_native32(image.data + 0x10, cfg->spl_offset, cfg->big_endian);
        put_native32(image.data + 0x14, spl_end, cfg->big_endian);
        put_native32(image.data + 0x18, 0, cfg->big_endian);
        put_native32(image.data + 0x1c, 0, cfg->big_endian);
    }

    if (cfg->extended_header) {
        put_native32(image.data + 0x58, cfg->ddr_offset, cfg->big_endian);
        put_native32(image.data + 0x5c, cfg->ddr_offset + cfg->ddr_size,
                     cfg->big_endian);
    }

    for (i = 0; i < cfg->crc_count; i++) {
        uint32_t off = cfg->crc_offsets[i];
        uint32_t crc = ~crc32_ieee(image.data, off);
        put_native32(image.data + off, crc, cfg->big_endian);
    }

    write_file(output_path ? output_path : image_path, image.data, image.len);
    ret = 0;
out:
    free_blob(&stock);
    free_blob(&image);
    return ret;
}

static void usage(FILE *f)
{
    fprintf(f,
        "usage:\n"
        "  econet-image chainloader --image FILE [--output FILE]\n"
        "  econet-image flash --soc SOC --stages FILE --symbols FILE --uboot FILE\\\n\n"
        "      --load ADDR --output FILE [--minfo FILE]\n"
        "  econet-image tcboot --soc SOC --image FILE [--output FILE] [--stock FILE]\n"
        "  econet-image selftest\n");
}

static const char *next_arg(int *i, int argc, char **argv, const char *opt)
{
    if (*i + 1 >= argc) {
        fprintf(stderr, "econet-image: %s requires an argument\n", opt);
        exit(2);
    }
    return argv[++*i];
}

static int cmd_chainloader(int argc, char **argv)
{
    const char *image = NULL, *output = NULL;
    int i;

    for (i = 0; i < argc; i++) {
        if (!strcmp(argv[i], "--image"))
            image = next_arg(&i, argc, argv, "--image");
        else if (!strcmp(argv[i], "--output"))
            output = next_arg(&i, argc, argv, "--output");
        else {
            usage(stderr);
            return 2;
        }
    }
    if (!image) {
        usage(stderr);
        return 2;
    }
    return finalize_chainloader(image, output);
}

static int cmd_flash(int argc, char **argv)
{
    const char *soc = NULL, *stages = NULL, *symbols = NULL;
    const char *uboot = NULL, *output = NULL, *minfo = NULL;
    uint32_t load = 0;
    bool have_load = false;
    int i;

    for (i = 0; i < argc; i++) {
        if (!strcmp(argv[i], "--soc"))
            soc = next_arg(&i, argc, argv, "--soc");
        else if (!strcmp(argv[i], "--stages"))
            stages = next_arg(&i, argc, argv, "--stages");
        else if (!strcmp(argv[i], "--symbols"))
            symbols = next_arg(&i, argc, argv, "--symbols");
        else if (!strcmp(argv[i], "--uboot"))
            uboot = next_arg(&i, argc, argv, "--uboot");
        else if (!strcmp(argv[i], "--load")) {
            load = parse_u32(next_arg(&i, argc, argv, "--load"));
            have_load = true;
        } else if (!strcmp(argv[i], "--output"))
            output = next_arg(&i, argc, argv, "--output");
        else if (!strcmp(argv[i], "--minfo"))
            minfo = next_arg(&i, argc, argv, "--minfo");
        else {
            usage(stderr);
            return 2;
        }
    }

    if (!soc || !stages || !symbols || !uboot || !output || !have_load) {
        usage(stderr);
        return 2;
    }
    return pack_flash_image(soc, stages, symbols, uboot, load, minfo, output);
}

static int cmd_tcboot(int argc, char **argv)
{
    const char *soc = NULL, *image = NULL, *output = NULL, *stock = NULL;
    int i;

    for (i = 0; i < argc; i++) {
        if (!strcmp(argv[i], "--soc"))
            soc = next_arg(&i, argc, argv, "--soc");
        else if (!strcmp(argv[i], "--image"))
            image = next_arg(&i, argc, argv, "--image");
        else if (!strcmp(argv[i], "--output"))
            output = next_arg(&i, argc, argv, "--output");
        else if (!strcmp(argv[i], "--stock"))
            stock = next_arg(&i, argc, argv, "--stock");
        else {
            usage(stderr);
            return 2;
        }
    }
    if (!soc || !image) {
        usage(stderr);
        return 2;
    }
    return finalize_tcboot(soc, image, output, stock);
}

/* Small dependency-free regression suite for the host-side image formats. */
struct flash_fixture {
    uint8_t stages[0x9000];
    struct symbol syms[8];
    uint8_t header[64];
    uint8_t data[256];
    size_t data_len;
};

static void fixture_init(struct flash_fixture *f)
{
    static const struct {
        const char *name;
        uint32_t a, b;
    } ranges[] = {
        { "move_data", 0x280, 0x768 },
        { "boot2", 0x768, 0x1660 },
        { "lzma", 0x1660, 0x2000 },
        { "spram", 0x2000, 0x9000 },
    };
    size_t i;

    memset(f, 0, sizeof(*f));
    f->stages[0] = 0x0b;
    f->stages[1] = 0xf0;
    f->stages[2] = 0x00;
    f->stages[3] = 0x18;

    for (i = 0; i < ARRAY_SIZE(ranges); i++) {
        snprintf(f->syms[i * 2].name, sizeof(f->syms[i * 2].name),
                 "__%s_start", ranges[i].name);
        f->syms[i * 2].value = 0xbfc00000u + ranges[i].a;
        snprintf(f->syms[i * 2 + 1].name, sizeof(f->syms[i * 2 + 1].name),
                 "__%s_end", ranges[i].name);
        f->syms[i * 2 + 1].value = 0xbfc00000u + ranges[i].b;
    }

    f->data_len = 17 * 15;
    for (i = 0; i < f->data_len; i++)
        f->data[i] = "U-Boot payload\0"[i % 15];

    put_be32(f->header + 0, IH_MAGIC);
    put_be32(f->header + 12, (uint32_t)f->data_len);
    put_be32(f->header + 16, 0x81000000u);
    put_be32(f->header + 20, 0x81000000u);
    put_be32(f->header + 24, crc32_ieee(f->data, f->data_len));
    f->header[28] = 17;
    f->header[29] = 5;
    f->header[30] = 5;
    f->header[31] = 0;
    put_be32(f->header + 4, 0);
    put_be32(f->header + 4, crc32_ieee(f->header, sizeof(f->header)));
}

static int selftest_crc(void)
{
    static const char s[] = "123456789";
    return crc32_ieee((const uint8_t *)s, 9) == 0xcbf43926u ? 0 : -1;
}

static int selftest(void)
{
    struct flash_fixture f;
    int passed = 0, total = 7;
    uint32_t move_s, move_e, boot2_s, boot2_e, loader_s, loader_e, ddr_s, ddr_e;

#define TEST(cond, name) do { \
    if (!(cond)) { fprintf(stderr, "not ok - %s\n", name); return 1; } \
    printf("ok %d - %s\n", ++passed, name); \
} while (0)

    TEST(selftest_crc() == 0, "crc32 implementation");

    fixture_init(&f);
    TEST(stage_interval(f.syms, ARRAY_SIZE(f.syms), "move_data", sizeof(f.stages), &move_s, &move_e) == 0 &&
         stage_interval(f.syms, ARRAY_SIZE(f.syms), "boot2", sizeof(f.stages), &boot2_s, &boot2_e) == 0 &&
         stage_interval(f.syms, ARRAY_SIZE(f.syms), "lzma", sizeof(f.stages), &loader_s, &loader_e) == 0 &&
         stage_interval(f.syms, ARRAY_SIZE(f.syms), "spram", sizeof(f.stages), &ddr_s, &ddr_e) == 0 &&
         move_e < 0x800 && move_e <= boot2_s && boot2_e <= loader_s && loader_e <= ddr_s,
         "stage interval/layout validation");

    {
        struct blob u = { 0 };
        uint8_t buf[64 + sizeof(f.data)];
        memcpy(buf, f.header, 64);
        memcpy(buf + 64, f.data, f.data_len);
        u.data = buf;
        u.len = 64 + f.data_len;
        TEST(validate_uboot(&u, 0x81000000u, true) == 0, "valid U-Boot legacy image");
    }

    {
        struct blob u = { 0 };
        uint8_t buf[64 + sizeof(f.data)];
        memcpy(buf, f.header, 64);
        memcpy(buf + 64, f.data, f.data_len);
        buf[64] ^= 1;
        u.data = buf;
        u.len = 64 + f.data_len;
        TEST(validate_uboot(&u, 0x81000000u, true) < 0, "bad payload CRC rejected");
    }

    {
        struct blob u = { 0 };
        uint8_t buf[64 + sizeof(f.data)];
        memcpy(buf, f.header, 64);
        memcpy(buf + 64, f.data, f.data_len);
        buf[4] ^= 1;
        u.data = buf;
        u.len = 64 + f.data_len;
        TEST(validate_uboot(&u, 0x81000000u, true) < 0, "bad header CRC rejected");
    }

    {
        struct blob u = { 0 };
        uint8_t buf[64 + sizeof(f.data)];
        memcpy(buf, f.header, 64);
        memcpy(buf + 64, f.data, f.data_len);
        buf[31] = 3;
        memset(buf + 4, 0, 4);
        put_be32(buf + 4, crc32_ieee(buf, 64));
        u.data = buf;
        u.len = 64 + f.data_len;
        TEST(validate_uboot(&u, 0x81000000u, true) < 0, "compressed payload rejected");
    }

    TEST(sizeof(f.stages) < FLASH_MINFO && FLASH_PAYLOAD + 64 + f.data_len < FLASH_LIMIT,
         "flash image size limits");

    printf("1..%d\n", total);
#undef TEST
    return 0;
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        usage(stderr);
        return 2;
    }
    if (!strcmp(argv[1], "chainloader"))
        return cmd_chainloader(argc - 2, argv + 2);
    if (!strcmp(argv[1], "flash"))
        return cmd_flash(argc - 2, argv + 2);
    if (!strcmp(argv[1], "tcboot"))
        return cmd_tcboot(argc - 2, argv + 2);
    if (!strcmp(argv[1], "selftest"))
        return selftest();

    usage(stderr);
    return 2;
}
