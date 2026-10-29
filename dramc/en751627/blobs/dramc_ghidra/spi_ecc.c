typedef unsigned char   undefined;

typedef unsigned char    bool;
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



undefined DAT_bfa1200c;
undefined DAT_bfa12034;
undefined1[12] _spi_ecc_encode_conf_info_t;
undefined DAT_bfa12004;
undefined DAT_bfa12000;
undefined DAT_bfa12030;
undefined DAT_bfa1210c;
undefined DAT_bfa12144;
undefined DAT_bfa12114;
undefined DAT_bfa12118;
undefined1[12] _spi_ecc_decode_conf_info_t;
undefined DAT_bfa12104;
undefined DAT_bfa12100;
undefined DAT_bfa12140;
undefined1 _SPI_ECC_DEBUG_FLAG;

undefined4 SPI_ECC_Regs_Dump(void)

{
  bool bVar1;
  int iVar2;
  
  iVar2 = 0x53;
  do {
    bVar1 = iVar2 != 0;
    iVar2 = iVar2 + -1;
  } while (bVar1);
  return 0;
}



// WARNING: Globals starting with '_' overlap smaller symbols at the same address

undefined4 SPI_ECC_Encode_Check_Idle(undefined4 *param_1)

{
  if (_DAT_bfa1200c == 0) {
    *param_1 = 1;
  }
  if (_DAT_bfa1200c == 1) {
    *param_1 = 0;
  }
  return 0;
}



// WARNING: Globals starting with '_' overlap smaller symbols at the same address

undefined4 SPI_ECC_Encode_Check_Done(undefined4 *param_1)

{
  if (_DAT_bfa12034 == 0) {
    *param_1 = 1;
  }
  if (_DAT_bfa12034 == 1) {
    *param_1 = 2;
  }
  return 0;
}



undefined4 SPI_ECC_Encode_Get_Configure(undefined4 *param_1)

{
  undefined4 uVar1;
  undefined4 uVar2;
  
  uVar2 = _spi_ecc_encode_conf_info_t._8_4_;
  uVar1 = _spi_ecc_encode_conf_info_t._4_4_;
  *param_1 = _spi_ecc_encode_conf_info_t._0_4_;
  param_1[1] = uVar1;
  param_1[2] = uVar2;
  return 0;
}



// WARNING: Globals starting with '_' overlap smaller symbols at the same address

undefined4 SPI_ECC_Encode_Set_Configure(undefined4 *param_1)

{
  uint uVar1;
  int iVar2;
  
  _spi_ecc_encode_conf_info_t._0_4_ = *param_1;
  _spi_ecc_encode_conf_info_t._4_4_ = param_1[1];
  _spi_ecc_encode_conf_info_t._8_4_ = param_1[2];
  uVar1 = _DAT_bfa12004 & 0xe007ffff;
  iVar2 = param_1[2];
  if (iVar2 == 4) {
    uVar1 = _DAT_bfa12004 & 0xe007fff8;
    iVar2 = param_1[2];
  }
  _DAT_bfa12004 = param_1[1] << 0x13 | uVar1;
  if (iVar2 == 6) {
    _DAT_bfa12004 = param_1[1] << 0x13 | uVar1 & 0xfffffff8 | 1;
  }
  iVar2 = param_1[2];
  if (iVar2 == 8) {
    _DAT_bfa12004 = _DAT_bfa12004 & 0xfffffff8 | 2;
    iVar2 = param_1[2];
  }
  if (iVar2 == 10) {
    _DAT_bfa12004 = _DAT_bfa12004 & 0xfffffff8 | 3;
  }
  iVar2 = param_1[2];
  if (iVar2 == 0xc) {
    _DAT_bfa12004 = _DAT_bfa12004 & 0xfffffff8 | 4;
    iVar2 = param_1[2];
  }
  if (iVar2 == 0xe) {
    _DAT_bfa12004 = _DAT_bfa12004 & 0xfffffff8 | 5;
  }
  if (param_1[2] == 0x10) {
    _DAT_bfa12004 = _DAT_bfa12004 & 0xfffffff8 | 6;
  }
  return 0;
}



// WARNING: Globals starting with '_' overlap smaller symbols at the same address

undefined4 SPI_ECC_Encode_Enable(void)

{
  _DAT_bfa12000 = _DAT_bfa12000 | 1;
  return 0;
}



// WARNING: Globals starting with '_' overlap smaller symbols at the same address

undefined4 SPI_ECC_Encode_Disable(void)

{
  _DAT_bfa12000 = _DAT_bfa12000 & 0xfffffffe;
  return 0;
}



// WARNING: Globals starting with '_' overlap smaller symbols at the same address

undefined4 SPI_ECC_Encode_Init(void)

{
  _DAT_bfa12004 = _DAT_bfa12004 & 0xffffffcf | 0x10;
  _DAT_bfa12030 = _DAT_bfa12030 | 1;
  return 0;
}



// WARNING: Globals starting with '_' overlap smaller symbols at the same address

undefined4 SPI_ECC_Decode_Check_Idle(undefined4 *param_1)

{
  if (_DAT_bfa1210c == 0) {
    *param_1 = 1;
  }
  if (_DAT_bfa1210c == 1) {
    *param_1 = 0;
  }
  return 0;
}



// WARNING: Globals starting with '_' overlap smaller symbols at the same address

undefined4 SPI_ECC_Decode_Check_Done(undefined4 *param_1)

{
  if (_DAT_bfa12144 != 0) {
    *param_1 = 2;
  }
  return 0;
}



// WARNING: Globals starting with '_' overlap smaller symbols at the same address

bool SPI_ECC_DECODE_Check_Correction_Status(void)

{
  return (_DAT_bfa12118 & 0xf8000) == 0xf8000 ||
         ((_DAT_bfa12118 & 0x7fff) >> 10 == 0x1f ||
         ((_DAT_bfa12118 & 0x3ff) >> 5 == 0x1f ||
         ((_DAT_bfa12118 & 0x1f) == 0x1f ||
         ((_DAT_bfa12114 & 0xf8000) == 0xf8000 ||
         ((_DAT_bfa12114 & 0x7fff) >> 10 == 0x1f ||
         ((_DAT_bfa12114 & 0x3ff) >> 5 == 0x1f || (_DAT_bfa12114 & 0x1f) == 0x1f))))));
}



undefined4 SPI_ECC_Decode_Get_Configure(undefined4 *param_1)

{
  undefined4 uVar1;
  undefined4 uVar2;
  
  uVar2 = _spi_ecc_decode_conf_info_t._8_4_;
  uVar1 = _spi_ecc_decode_conf_info_t._4_4_;
  *param_1 = _spi_ecc_decode_conf_info_t._0_4_;
  param_1[1] = uVar1;
  param_1[2] = uVar2;
  return 0;
}



// WARNING: Globals starting with '_' overlap smaller symbols at the same address

undefined4 SPI_ECC_Decode_Set_Configure(undefined4 *param_1)

{
  uint uVar1;
  int iVar2;
  
  _spi_ecc_decode_conf_info_t._0_4_ = *param_1;
  _spi_ecc_decode_conf_info_t._4_4_ = param_1[1];
  _spi_ecc_decode_conf_info_t._8_4_ = param_1[2];
  uVar1 = _DAT_bfa12104 & 0xe000ffff;
  iVar2 = param_1[2];
  if (iVar2 == 4) {
    uVar1 = _DAT_bfa12104 & 0xe000fff8;
    iVar2 = param_1[2];
  }
  _DAT_bfa12104 = param_1[1] << 0x10 | uVar1;
  if (iVar2 == 6) {
    _DAT_bfa12104 = param_1[1] << 0x10 | uVar1 & 0xfffffff8 | 1;
  }
  iVar2 = param_1[2];
  if (iVar2 == 8) {
    _DAT_bfa12104 = _DAT_bfa12104 & 0xfffffff8 | 2;
    iVar2 = param_1[2];
  }
  if (iVar2 == 10) {
    _DAT_bfa12104 = _DAT_bfa12104 & 0xfffffff8 | 3;
  }
  iVar2 = param_1[2];
  if (iVar2 == 0xc) {
    _DAT_bfa12104 = _DAT_bfa12104 & 0xfffffff8 | 4;
    iVar2 = param_1[2];
  }
  if (iVar2 == 0xe) {
    _DAT_bfa12104 = _DAT_bfa12104 & 0xfffffff8 | 5;
  }
  if (param_1[2] == 0x10) {
    _DAT_bfa12104 = _DAT_bfa12104 & 0xfffffff8 | 6;
  }
  return 0;
}



// WARNING: Globals starting with '_' overlap smaller symbols at the same address

undefined4 SPI_ECC_Decode_Enable(void)

{
  _DAT_bfa12100 = _DAT_bfa12100 | 1;
  return 0;
}



// WARNING: Globals starting with '_' overlap smaller symbols at the same address

undefined4 SPI_ECC_Decode_Disable(void)

{
  _DAT_bfa12100 = _DAT_bfa12100 & 0xfffffffe;
  return 0;
}



// WARNING: Globals starting with '_' overlap smaller symbols at the same address

undefined4 SPI_ECC_Decode_Init(void)

{
  _DAT_bfa12104 = _DAT_bfa12104 & 0x7fffffcf | 0x3010;
  _DAT_bfa12140 = _DAT_bfa12140 | 1;
  return 0;
}



void SPI_ECC_DEBUG_ENABLE(void)

{
  _SPI_ECC_DEBUG_FLAG = 1;
  return;
}



void SPI_ECC_DEBUG_DISABLE(void)

{
  _SPI_ECC_DEBUG_FLAG = 0;
  return;
}


