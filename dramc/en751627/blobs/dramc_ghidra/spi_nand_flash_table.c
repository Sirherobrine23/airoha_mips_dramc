typedef unsigned char   undefined;

typedef unsigned char    byte;
typedef unsigned int    dword;
typedef unsigned char    undefined1;
typedef unsigned int    undefined4;
typedef unsigned short    word;
typedef enum Elf32_GPRMask_MIPS {
    gpr_zero=1,
    gpr_at=2,
    gpr_v0=4,
    gpr_v1=8,
    gpr_a0=16,
    gpr_a1=32,
    gpr_a2=64,
    gpr_a3=128,
    gpr_t0=256,
    gpr_t1=512,
    gpr_t2=1024,
    gpr_t3=2048,
    gpr_t4=4096,
    gpr_t5=8192,
    gpr_t6=16384,
    gpr_t7=32768,
    gpr_s0=65536,
    gpr_s1=131072,
    gpr_s2=262144,
    gpr_s3=524288,
    gpr_s4=1048576,
    gpr_s5=2097152,
    gpr_s6=4194304,
    gpr_s7=8388608,
    gpr_t8=16777216,
    gpr_t9=33554432,
    gpr_k0=67108864,
    gpr_k1=134217728,
    gpr_gp=268435456,
    gpr_sp=536870912,
    gpr_fp=1073741824,
    gpr_ra=2147483648
} Elf32_GPRMask_MIPS;

typedef struct Elf32_Shdr Elf32_Shdr, *PElf32_Shdr;

typedef enum Elf_SectionHeaderType_MIPS {
    SHT_NULL=0,
    SHT_PROGBITS=1,
    SHT_SYMTAB=2,
    SHT_STRTAB=3,
    SHT_RELA=4,
    SHT_HASH=5,
    SHT_DYNAMIC=6,
    SHT_NOTE=7,
    SHT_NOBITS=8,
    SHT_REL=9,
    SHT_SHLIB=10,
    SHT_DYNSYM=11,
    SHT_INIT_ARRAY=14,
    SHT_FINI_ARRAY=15,
    SHT_PREINIT_ARRAY=16,
    SHT_GROUP=17,
    SHT_SYMTAB_SHNDX=18,
    SHT_ANDROID_REL=1610612737,
    SHT_ANDROID_RELA=1610612738,
    SHT_GNU_ATTRIBUTES=1879048181,
    SHT_GNU_HASH=1879048182,
    SHT_GNU_LIBLIST=1879048183,
    SHT_CHECKSUM=1879048184,
    SHT_SUNW_move=1879048186,
    SHT_SUNW_COMDAT=1879048187,
    SHT_SUNW_syminfo=1879048188,
    SHT_GNU_verdef=1879048189,
    SHT_GNU_verneed=1879048190,
    SHT_GNU_versym=1879048191,
    SHT_MIPS_LIBLIST=1879048192,
    SHT_MIPS_MSYM=1879048193,
    SHT_MIPS_CONFLICT=1879048194,
    SHT_MIPS_GPTAB=1879048195,
    SHT_MIPS_UCODE=1879048196,
    SHT_MIPS_DEBUG=1879048197,
    SHT_MIPS_REGINFO=1879048198,
    SHT_MIPS_PACKAGE=1879048199,
    SHT_MIPS_PACKSYM=1879048200,
    SHT_MIPS_RELD=1879048201,
    =1879048203,
    SHT_MIPS_CONTENT=1879048204,
    SHT_MIPS_OPTIONS=1879048205,
    SHT_MIPS_SHDR=1879048208,
    SHT_MIPS_FDESC=1879048209,
    SHT_MIPS_EXTSYM=1879048210,
    SHT_MIPS_DENSE=1879048211,
    SHT_MIPS_PDESC=1879048212,
    SHT_MIPS_LOCSYM=1879048213,
    SHT_MIPS_AUXSYM=1879048214,
    SHT_MIPS_OPTSYM=1879048215,
    SHT_MIPS_LOCSTR=1879048216,
    SHT_MIPS_LINE=1879048217,
    SHT_MIPS_RFDESC=1879048218,
    SHT_MIPS_DELTASYM=1879048219,
    SHT_MIPS_DELTAINST=1879048220,
    SHT_MIPS_DELTACLASS=1879048221,
    SHT_MIPS_DWARF=1879048222,
    SHT_MIPS_DELTADECL=1879048223,
    SHT_MIPS_SYMBOL_LIB=1879048224,
    SHT_MIPS_EVENTS=1879048225,
    SHT_MIPS_TRANSLATE=1879048226,
    SHT_MIPS_PIXIE=1879048227,
    SHT_MIPS_XLATE=1879048228,
    SHT_MIPS_XLATE_DEBUG=1879048229,
    SHT_MIPS_WHIRL=1879048230,
    SHT_MIPS_EH_REGION=1879048231,
    SHT_MIPS_XLATE_OLD=1879048232,
    SHT_MIPS_PDR_EXCEPTION=1879048233,
    SHT_MIPS_ABIFLAGS=1879048234,
    SHT_MIPS_PSPREL=1879048352
} Elf_SectionHeaderType_MIPS;

struct Elf32_Shdr {
    dword sh_name;
    enum Elf_SectionHeaderType_MIPS sh_type;
    dword sh_flags;
    dword sh_addr;
    dword sh_offset;
    dword sh_size;
    dword sh_link;
    dword sh_info;
    dword sh_addralign;
    dword sh_entsize;
};

typedef struct Elf32_RegInfo_MIPS Elf32_RegInfo_MIPS, *PElf32_RegInfo_MIPS;

struct Elf32_RegInfo_MIPS {
    enum Elf32_GPRMask_MIPS ri_gprmask;
    dword field1_0x4[4];
    dword ri_gp_value;
};

typedef struct Elf32_Sym Elf32_Sym, *PElf32_Sym;

struct Elf32_Sym {
    dword st_name;
    dword st_value;
    dword st_size;
    byte st_info;
    byte st_other;
    word st_shndx;
};

typedef struct Elf32_Rel Elf32_Rel, *PElf32_Rel;

struct Elf32_Rel {
    dword r_offset; // location to apply the relocation action
    dword r_info; // the symbol table index and the type of relocation
};

typedef struct Elf32_Ehdr Elf32_Ehdr, *PElf32_Ehdr;

struct Elf32_Ehdr {
    byte e_ident_magic_num;
    char e_ident_magic_str[3];
    byte e_ident_class;
    byte e_ident_data;
    byte e_ident_version;
    byte e_ident_osabi;
    byte e_ident_abiversion;
    byte e_ident_pad[7];
    word e_type;
    word e_machine;
    dword e_version;
    dword e_entry;
    dword e_phoff;
    dword e_shoff;
    dword e_flags;
    word e_ehsize;
    word e_phentsize;
    word e_phnum;
    word e_shentsize;
    word e_shnum;
    word e_shstrndx;
};



pointer PTR_s__SPI_NAND_DEVICE_ID_GD5F1GQ4UB_00011684;
undefined4 DAT_00011688;
undefined4 DAT_0001168c;
undefined1 spi_nand_flash_tables;
undefined4 DAT_00011690;
undefined4 DAT_00011694;
undefined4 DAT_00011698;
undefined4 DAT_0001169c;
pointer PTR_ooblayout_type2_000116a0;
undefined1 DAT_000116a4;
undefined4 DAT_000116a8;
undefined4 DAT_000116ac;
undefined1 DAT_000116b0;
undefined1 DAT_000116b1;
undefined4 DAT_000116b4;
undefined1 DAT_000116b8;
undefined1 DAT_000116b9;
undefined1 DAT_000116ba;
undefined1 DAT_000116bb;
undefined1 DAT_000116bc;
undefined1 DAT_000116bd;
undefined1 DAT_000116be;

undefined4 scan_spi_nand_table(char *param_1)

{
  char *pcVar1;
  int iVar2;
  int iVar3;
  
  pcVar1 = &spi_nand_flash_tables;
  iVar2 = 0;
  while ((*pcVar1 != *param_1 || (param_1[1] != pcVar1[1]))) {
    iVar2 = iVar2 + 1;
    pcVar1 = pcVar1 + 0x40;
    if (iVar2 == 0x31) {
      return 1;
    }
  }
  iVar3 = iVar2 * 0x40;
  *(undefined4 *)(param_1 + 8) = (&DAT_00011688)[iVar2 * 0x10];
  *(undefined4 *)(param_1 + 0x10) = (&DAT_00011690)[iVar2 * 0x10];
  *(undefined4 *)(param_1 + 0xc) = (&DAT_0001168c)[iVar2 * 0x10];
  *(undefined4 *)(param_1 + 0x14) = (&DAT_00011694)[iVar2 * 0x10];
  *(undefined4 *)(param_1 + 0x18) = (&DAT_00011698)[iVar2 * 0x10];
  *(undefined4 *)(param_1 + 0x1c) = (&DAT_0001169c)[iVar2 * 0x10];
  *(undefined4 *)(param_1 + 0x28) = (&DAT_000116a8)[iVar2 * 0x10];
  *(undefined **)(param_1 + 4) = (&PTR_s__SPI_NAND_DEVICE_ID_GD5F1GQ4UB_00011684)[iVar2 * 0x10];
  *(undefined **)(param_1 + 0x20) = (&PTR_ooblayout_type2_000116a0)[iVar2 * 0x10];
  *(undefined4 *)(param_1 + 0x2c) = (&DAT_000116ac)[iVar2 * 0x10];
  param_1[0x24] = (&DAT_000116a4)[iVar3];
  param_1[0x30] = (&DAT_000116b0)[iVar3];
  param_1[0x31] = (&DAT_000116b1)[iVar3];
  *(undefined4 *)(param_1 + 0x34) = (&DAT_000116b4)[iVar2 * 0x10];
  param_1[0x38] = (&DAT_000116b8)[iVar3];
  param_1[0x39] = (&DAT_000116b9)[iVar3];
  param_1[0x3a] = (&DAT_000116ba)[iVar3];
  param_1[0x3b] = (&DAT_000116bb)[iVar3];
  param_1[0x3c] = (&DAT_000116bc)[iVar3];
  param_1[0x3d] = (&DAT_000116bd)[iVar3];
  param_1[0x3e] = (&DAT_000116be)[iVar3];
  return 0;
}


