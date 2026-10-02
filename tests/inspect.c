// SPDX-License-Identifier: GPL-2.0+
/* Independent serialized fixtures and CLI/output checks; no Python required. */
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static unsigned checks;
static void require(int ok, const char *message)
{
	if (!ok) { fprintf(stderr, "inspect test: %s\n", message); exit(1); }
}

static void put32(uint8_t *p, uint32_t value, int big)
{
	unsigned i;
	for (i = 0; i < 4; i++) p[big ? 3 - i : i] = (uint8_t)(value >> (i * 8));
}

static uint32_t crc(const uint8_t *p, size_t n, int ieee)
{
	uint32_t value = 0xffffffffu;
	unsigned i;
	while (n--) {
		value ^= *p++;
		for (i = 0; i < 8; i++) value = (value >> 1) ^ ((value & 1) ? 0xedb88320u : 0);
	}
	return ieee ? ~value : value;
}

static void secure(uint8_t *h, unsigned version, size_t payload, int big)
{
	size_t size = version <= 1 ? 276 : 2048;
	memcpy(h, "ECNT", 4); put32(h + 4, version, big);
	if (version == 2) put32(h + 8, 2048, big);
	put32(h + (version <= 1 ? 0x108 : 12), (uint32_t)payload + 4, big);
	put32(h + size - 4, crc(h, size - 4, 0), big);
}

static void check(const char *tool, const uint8_t *data, size_t size, int expected,
		  const char *match, const char *option, const char *value)
{
	char name[] = "/tmp/econet-inspect-XXXXXX", output[8192];
	FILE *capture = tmpfile();
	int fd = mkstemp(name), status;
	FILE *file;
	pid_t pid;
	size_t n;
	require(fd >= 0 && capture, "temporary file");
	file = fdopen(fd, "wb"); require(file != NULL, "fdopen");
	require(fwrite(data, 1, size, file) == size && !fclose(file), "write fixture");
	pid = fork(); require(pid >= 0, "fork");
	if (!pid) {
		char *args[] = { (char *)tool, "inspect", "--image", name,
				 (char *)option, (char *)value, NULL };
		if (dup2(fileno(capture), STDOUT_FILENO) < 0 || dup2(fileno(capture), STDERR_FILENO) < 0) _exit(126);
		execv(tool, args); _exit(127);
	}
	require(waitpid(pid, &status, 0) == pid, "waitpid");
	rewind(capture); n = fread(output, 1, sizeof(output) - 1, capture); output[n] = 0;
	fclose(capture); unlink(name);
	if (!WIFEXITED(status) || WEXITSTATUS(status) != expected || !strstr(output, match)) {
		fprintf(stderr, "inspect check %u failed: status=%d expected=%d match=%s\n%s", checks + 1, status, expected, match, output);
		exit(1);
	}
	checks++;
}

int main(int argc, char **argv)
{
	uint8_t *data = calloc(1, 0x40000);
	unsigned version, big;
	require(argc == 2 && data, "usage: test-inspect /absolute/path/econet-image");
	for (big = 0; big < 2; big++) {
		for (version = 0; version < 3; version++) {
			size_t header = 256 + (version == 1 ? 276 : version == 2 ? 2048 : 0), total = header + 32;
			memset(data, 0, total);
			put32(data, 0x32524448, big); put32(data + 4, (uint32_t)header, big);
			put32(data + 8, (uint32_t)total, big); put32(data + 80, 32, big);
			memset(data + 16, 'A', 32); data[17] = 0x1b;
			put32(data + 12, crc(data + header, 32, 0), big);
			if (version) secure(data + 256, version, 32, big);
			check(argv[1], data, total, 0, "Payload CRC:", NULL, NULL);
			check(argv[1], data, total, 0, "\\x1b", NULL, NULL);
			check(argv[1], data, total, 1, "wrong endian", "--endian", big ? "little" : "big");
			check(argv[1], data, header - 1, 1, version ? "length" : "truncated TRX", NULL, NULL);
			data[header] ^= 1;
			check(argv[1], data, total, 1, "[BAD]", NULL, NULL);
			data[header] ^= 1;
			put32(data + 80, 33, big);
			check(argv[1], data, total, 1, "component lengths", NULL, NULL);
			put32(data + 80, 32, big);
			if (version) {
				check(argv[1], data + 256, total - 256, 0, "NOT CHECKED", NULL, NULL);
				check(argv[1], data, total, 0, "Secure header", "--offset", "256");
				check(argv[1], data + 256, 16, 1, "truncated", "--format", "sheader");
				data[header - 4] ^= 1;
				check(argv[1], data + 256, total - 256, 1, "[BAD]", NULL, NULL);
				data[header - 4] ^= 1;
				put32(data + 256 + (version == 1 ? 0x108 : 12), 0xffffffffu, big);
				check(argv[1], data + 256, total - 256, 1, "outside", "--format", "sheader");
			}
		}
		memset(data, 0, 0x40000);
		secure(data, 2, 0x1fffc - 2048, big);
		secure(data + 0x20000, 2, 0x3fdfc - 0x20000 - 2048, big);
		put32(data + 0x1fffc, crc(data, 0x1fffc, 0), big);
		put32(data + 0x3fdfc, crc(data, 0x3fdfc, 0), big);
		check(argv[1], data, 0x40000, 0, "Boot CRC at 0x3fdfc", NULL, NULL);
		data[0x1fffc] ^= 1;
		check(argv[1], data, 0x40000, 1, "[BAD]", NULL, NULL);
		/* Real EN7580 dumps use the V1 layout with version field zero. */
		memset(data, 0, 0x40000);
		secure(data, 0, 0x1fffc - 276, big);
		secure(data + 0x20000, 0, 0x3fdfc - 0x20000 - 276, big);
		put32(data + 0x1fffc, crc(data, 0x1fffc, 0), big);
		put32(data + 0x3fdfc, crc(data, 0x3fdfc, 0), big);
		check(argv[1], data, 0x40000, 0, "version field=0; legacy", NULL, NULL);
		check(argv[1], data + 0x20000, 0x20000, 0, big ? "big endian" : "little endian", "--endian", big ? "big" : "little");
		data[0x110] ^= 1;
		check(argv[1], data, 0x40000, 1, "[BAD]", NULL, NULL);
		memset(data, 0, 564);
		put32(data, 0x32524448, big); put32(data + 4, 532, big);
		put32(data + 8, 564, big); put32(data + 80, 32, big);
		put32(data + 12, crc(data + 532, 32, 0), big);
		secure(data + 256, 0, 32, big);
		check(argv[1], data, 564, 0, "version field=0; legacy", NULL, NULL);
	}
	for (version = 1; version <= 2; version++) {
		size_t header = version == 1 ? 32 : 48;
		memset(data, 0, 80); memcpy(data, "ECNT", 4);
		put32(data + 4, version, 1); put32(data + 8, (uint32_t)header, 1);
		put32(data + 12, 32, 1); put32(data + 16, 0x81000000, 1); put32(data + 20, 0x81000000, 1);
		put32(data + 24, crc(data + header, 32, 1), 1);
		if (version == 2) {
			put32(data + 32, 1, 1); put32(data + 36, 64, 1); put32(data + 40, 123, 1);
		}
		put32(data + 28, crc(data, header, 1), 1);
		check(argv[1], data, header + 32, 0, version == 1 ? "Compression: none" : "Compression: gzip", NULL, NULL);
		check(argv[1], data, header + 32, 1, "big endian", "--endian", "little");
		data[header] ^= 1;
		check(argv[1], data, header + 32, 1, "[BAD]", NULL, NULL);
		check(argv[1], data, header, 1, "outside", NULL, NULL);
	}
	memset(data, 0, 0x40000); memcpy(data + 12, "6578", 4);
	put32(data + 0x1fffc, crc(data, 0x1fffc, 0), 0);
	put32(data + 0x3fdfc, crc(data, 0x3fdfc, 0), 0);
	check(argv[1], data, 0x40000, 0, "Boot CRC at 0x3fdfc", "--soc", "en7580");
	check(argv[1], data, 0x40000, 1, "requires --soc", NULL, NULL);
	check(argv[1], data, 3, 1, "truncated magic", NULL, NULL);
	check(argv[1], data, 32, 1, "offset outside", "--offset", "0xffffffff");
	memset(data, 0, 32);
	check(argv[1], data, 32, 1, "unknown format", NULL, NULL);
	free(data);
	printf("inspect: %u CLI/format/corruption checks passed\n", checks);
	return 0;
}
