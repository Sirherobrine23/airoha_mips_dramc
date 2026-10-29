typedef unsigned char   undefined;

typedef unsigned char    byte;
typedef unsigned int    dword;
typedef unsigned int    uint;
typedef unsigned char    undefined1;
typedef unsigned int    undefined4;
typedef unsigned long long    undefined8;
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



undefined DAT_bfa10028;
undefined DAT_bfa1002c;
undefined DAT_bfa10030;
undefined DAT_bfa10024;
undefined DAT_bfa10034;
undefined DAT_bfa10038;
undefined DAT_bfb00064;
undefined DAT_bfa10130;
undefined DAT_bfa10014;
undefined DAT_bfa10020;
undefined DAT_bfa10004;
undefined DAT_bfa10018;
undefined DAT_bfa10080;
undefined8 _spi_controller_conf_t;
undefined DAT_bfa1003c;
undefined DAT_bfa10044;
undefined DAT_bfa10040;
undefined1 _SPI_CONTROLLER_DEBUG_FLAG;

// WARNING: Globals starting with '_' overlap smaller symbols at the same address

undefined4 spi_controller_set_opfifo(uint param_1,uint param_2)

{
  _DAT_bfa10028 = (param_1 & 0x1f) << 9 | param_2 & 0x1ff;
  do {
  } while (_DAT_bfa1002c != 0);
  _DAT_bfa10030 = 1;
  do {
  } while (_DAT_bfa10024 == 0);
  return 0;
}



// WARNING: Globals starting with '_' overlap smaller symbols at the same address

undefined4 spi_controller_write_data_fifo(int param_1,int param_2)

{
  int iVar1;
  
  iVar1 = 0;
  if (param_2 != 0) {
    do {
      do {
      } while (_DAT_bfa10034 != 0);
      _DAT_bfa10038 = (uint)*(byte *)(param_1 + iVar1);
      iVar1 = iVar1 + 1;
    } while (iVar1 != param_2);
  }
  return 0;
}



// WARNING: Globals starting with '_' overlap smaller symbols at the same address

undefined4 SPI_CONTROLLER_Set_Configure(int *param_1)

{
  _spi_controller_conf_t._0_4_ = *param_1;
  _spi_controller_conf_t._4_4_ = param_1[1];
  if (_spi_controller_conf_t._0_4_ == 0) {
    if (((((_DAT_bfb00064 & 0xffff0000) == 0x90000) || ((_DAT_bfb00064 & 0xffff0000) == 0xb0000)) ||
        ((_DAT_bfb00064 & 0xffff0000) == 0x80000)) || ((_DAT_bfb00064 & 0xffff0000) == 0xa0000)) {
      _DAT_bfa10130 = 0;
    }
    _DAT_bfa10014 = 0;
    _DAT_bfa10020 = 0;
  }
  if (_spi_controller_conf_t._0_4_ == 1) {
    if ((((_DAT_bfb00064 & 0xffff0000) == 0x90000) || ((_DAT_bfb00064 & 0xffff0000) == 0xb0000)) ||
       (((_DAT_bfb00064 & 0xffff0000) == 0x80000 || ((_DAT_bfb00064 & 0xffff0000) == 0xa0000)))) {
      _DAT_bfa10130 = 0;
    }
    _DAT_bfa10004 = 0;
    do {
    } while (_DAT_bfa10018 != 0);
    _DAT_bfa10014 = 9;
    _DAT_bfa10020 = 1;
  }
  if (_spi_controller_conf_t._0_4_ == 2) {
    if ((((_DAT_bfb00064 & 0xffff0000) == 0x90000) || ((_DAT_bfb00064 & 0xffff0000) == 0xb0000)) ||
       (((_DAT_bfb00064 & 0xffff0000) == 0x80000 || ((_DAT_bfb00064 & 0xffff0000) == 0xa0000)))) {
      _DAT_bfa10130 = 1;
    }
    _DAT_bfa10014 = 0;
    _DAT_bfa10020 = 0;
  }
  _DAT_bfa10080 = param_1[1];
  return 0;
}



undefined4 SPI_CONTROLLER_Get_Configure(undefined4 *param_1)

{
  undefined4 uVar1;
  
  uVar1 = _spi_controller_conf_t._4_4_;
  *param_1 = _spi_controller_conf_t._0_4_;
  param_1[1] = uVar1;
  return 0;
}



undefined4 SPI_CONTROLLER_Enable_Manual_Mode(void)

{
  int local_10 [3];
  
  local_10[1] = 1;
  local_10[0] = 1;
  SPI_CONTROLLER_Set_Configure(local_10);
  return 0;
}



undefined4 SPI_CONTROLLER_Write_One_Byte_With_Cmd(uint param_1,undefined1 param_2)

{
  undefined1 local_res4 [12];
  
  local_res4[0] = param_2;
  spi_controller_set_opfifo(param_1 & 0xff,1);
  spi_controller_write_data_fifo((int)local_res4,1);
  return 0;
}



void SPI_CONTROLLER_Write_One_Byte(undefined1 param_1)

{
  SPI_CONTROLLER_Write_One_Byte_With_Cmd(8,param_1);
  return;
}



undefined4 SPI_CONTROLLER_Write_NByte(int param_1,uint param_2,int param_3)

{
  uint uVar1;
  uint uVar2;
  uint uVar3;
  
  uVar3 = 9;
  uVar1 = param_2;
  if (param_3 != 1) {
    if (param_3 == 0) {
      uVar3 = 8;
    }
    else if (param_3 == 2) {
      uVar3 = 10;
    }
  }
  for (; uVar1 != 0; uVar1 = uVar1 - uVar2) {
    uVar2 = 0x1ff;
    if (uVar1 < 0x200) {
      uVar2 = uVar1;
    }
    spi_controller_set_opfifo(uVar3,uVar2);
    spi_controller_write_data_fifo(param_1 + (param_2 - uVar1),uVar2);
  }
  return 0;
}



// WARNING: Globals starting with '_' overlap smaller symbols at the same address

undefined4 SPI_CONTROLLER_Read_NByte(int param_1,uint param_2,int param_3)

{
  uint uVar1;
  uint uVar2;
  uint uVar3;
  uint uVar4;
  
  uVar4 = 0xe;
  uVar1 = param_2;
  if (param_3 != 1) {
    if (param_3 == 0) {
      uVar4 = 0xc;
    }
    else if (param_3 == 2) {
      uVar4 = 0xf;
    }
  }
  for (; uVar1 != 0; uVar1 = uVar1 - uVar2) {
    uVar2 = 0x1ff;
    if (uVar1 < 0x200) {
      uVar2 = uVar1;
    }
    spi_controller_set_opfifo(uVar4,uVar2);
    if (uVar2 != 0) {
      uVar3 = 0;
      do {
        do {
        } while (_DAT_bfa1003c != 0);
        *(char *)(param_1 + (param_2 - uVar1) + uVar3) = (char)_DAT_bfa10044;
        _DAT_bfa10040 = 1;
        uVar3 = uVar3 + 1;
      } while (uVar3 != uVar2);
    }
  }
  return 0;
}



undefined4 SPI_CONTROLLER_Chip_Select_Low(void)

{
  spi_controller_set_opfifo(1,1);
  return 0;
}



undefined4 SPI_CONTROLLER_Chip_Select_High(void)

{
  spi_controller_set_opfifo(0,1);
  return 0;
}



void SPI_CONTROLLER_DEBUG_ENABLE(void)

{
  _SPI_CONTROLLER_DEBUG_FLAG = 1;
  return;
}



void SPI_CONTROLLER_DEBUG_DISABLE(void)

{
  _SPI_CONTROLLER_DEBUG_FLAG = 0;
  return;
}


