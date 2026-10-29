typedef unsigned char   undefined;

typedef unsigned char    byte;
typedef unsigned int    dword;
typedef unsigned int    uint;
typedef unsigned long    ulong;
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

typedef struct Elf32_Rel Elf32_Rel, *PElf32_Rel;

struct Elf32_Rel {
    dword r_offset; // location to apply the relocation action
    dword r_info; // the symbol table index and the type of relocation
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

typedef ulong size_t;



undefined DAT_bfbf010c;
undefined DAT_bfbf0110;
undefined DAT_bfa110a0;
undefined1[64] _spi_nfi_fdm_value;
undefined1[40] _spi_nfi_conf_info_t;
undefined DAT_bfa11000;
undefined DAT_bfa11004;
undefined DAT_bfa11008;
undefined DAT_bfa1122c;
undefined DAT_bfa11010;
undefined1 _SPI_NFI_DEBUG_FLAG;
undefined DAT_bfa11080;
undefined DAT_bfa1153c;
undefined DAT_bfa11524;
undefined DAT_bfa11538;
undefined DAT_bfa11528;
undefined DAT_bfa11020;
undefined DAT_bfa11014;
undefined DAT_bfa11550;
undefined DAT_bfa11510;
undefined DAT_bfa11514;

// WARNING: Removing unreachable block (ram,0x00010080)
// WARNING: Globals starting with '_' overlap smaller symbols at the same address

void delay1us(int param_1)

{
  int iVar1;
  
  iVar1 = get_SYS_HCLK();
  do {
  } while ((iVar1 / 2) * param_1 != 0);
  return;
}



undefined4 spi_nfi_get_fdm_from_register(void)

{
  undefined1 *puVar1;
  int iVar2;
  undefined4 uVar3;
  undefined4 *puVar4;
  int iVar5;
  uint uVar6;
  uint uVar7;
  uint uVar8;
  undefined1 local_48 [68];
  
  memset(local_48,0xff,0x40);
  memset(_spi_nfi_fdm_value,0xff,0x40);
  puVar1 = local_48;
  puVar4 = (undefined4 *)&DAT_bfa110a0;
  do {
    uVar3 = *puVar4;
    *puVar1 = (char)uVar3;
    puVar1[1] = (char)((uint)uVar3 >> 8);
    puVar1[2] = (char)((uint)uVar3 >> 0x10);
    puVar1[3] = (char)((uint)uVar3 >> 0x18);
    puVar4 = puVar4 + 1;
    puVar1 = puVar1 + 4;
  } while (puVar4 != (undefined4 *)0xbfa110e0);
  uVar8 = (uint)_spi_nfi_conf_info_t[0x18];
  if (uVar8 != 0) {
    uVar7 = (uint)_spi_nfi_conf_info_t[0xc];
    uVar6 = 0;
    iVar5 = 0;
    do {
      iVar2 = iVar5;
      if (uVar7 != 0) {
        do {
          _spi_nfi_fdm_value[iVar2] = local_48[iVar2 + (uVar6 * 8 - iVar5)];
          iVar2 = iVar2 + 1;
        } while ((uint)(iVar2 - iVar5) < uVar7);
      }
      uVar6 = uVar6 + 1;
      iVar5 = iVar2;
    } while (uVar6 < uVar8);
  }
  return 0;
}



undefined4 spi_nfi_set_fdm_into_register(void)

{
  int iVar1;
  byte *pbVar2;
  uint *puVar3;
  int iVar4;
  uint uVar5;
  byte local_48 [68];
  
  memset(local_48,0xff,0x40);
  uVar5 = 0;
  if (_spi_nfi_conf_info_t[0x18] != 0) {
    iVar4 = 0;
    do {
      iVar1 = iVar4;
      if (_spi_nfi_conf_info_t[0xc] != 0) {
        do {
          local_48[iVar1 + (uVar5 * 8 - iVar4)] = _spi_nfi_fdm_value[iVar1];
          iVar1 = iVar1 + 1;
        } while ((uint)(iVar1 - iVar4) < (uint)_spi_nfi_conf_info_t[0xc]);
      }
      uVar5 = uVar5 + 1;
      iVar4 = iVar1;
    } while (uVar5 < _spi_nfi_conf_info_t[0x18]);
  }
  pbVar2 = local_48;
  puVar3 = (uint *)&DAT_bfa110a0;
  do {
    *puVar3 = (uint)pbVar2[1] << 8 | (uint)pbVar2[2] << 0x10 | (uint)*pbVar2 |
              (uint)pbVar2[3] << 0x18;
    puVar3 = puVar3 + 1;
    pbVar2 = pbVar2 + 4;
  } while (puVar3 != (uint *)0xbfa110e0);
  return 0;
}



undefined4 SPI_NFI_Read_SPI_NAND_FDM(void *param_1,size_t param_2)

{
  spi_nfi_get_fdm_from_register();
  memcpy(param_1,_spi_nfi_fdm_value,param_2);
  return 0;
}



undefined4 SPI_NFI_Write_SPI_NAND_FDM(undefined4 *param_1,uint param_2)

{
  undefined4 *puVar1;
  undefined1 *puVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  
  if (param_2 < 0x41) {
    memcpy(_spi_nfi_fdm_value,param_1,param_2);
  }
  else {
    puVar2 = _spi_nfi_fdm_value;
    puVar1 = param_1 + 0x10;
    do {
      uVar5 = param_1[1];
      uVar4 = param_1[2];
      uVar3 = param_1[3];
      *(undefined4 *)puVar2 = *param_1;
      *(undefined4 *)((int)puVar2 + 4) = uVar5;
      *(undefined4 *)((int)puVar2 + 8) = uVar4;
      *(undefined4 *)((int)puVar2 + 0xc) = uVar3;
      param_1 = param_1 + 4;
      puVar2 = (undefined1 *)((int)puVar2 + 0x10);
    } while (param_1 != puVar1);
  }
  spi_nfi_set_fdm_into_register();
  return 0;
}



undefined4 SPI_NFI_Get_Configure(undefined4 *param_1)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  undefined4 uVar3;
  undefined4 *puVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  
  puVar1 = (undefined4 *)_spi_nfi_conf_info_t;
  do {
    puVar4 = param_1;
    puVar2 = puVar1;
    uVar6 = puVar2[1];
    uVar5 = puVar2[2];
    uVar3 = puVar2[3];
    *puVar4 = *puVar2;
    puVar4[1] = uVar6;
    puVar4[2] = uVar5;
    puVar4[3] = uVar3;
    puVar1 = puVar2 + 4;
    param_1 = puVar4 + 4;
  } while (puVar2 + 4 != (undefined4 *)(_spi_nfi_conf_info_t + 0x20));
  uVar3 = puVar2[5];
  puVar4[4] = _spi_nfi_conf_info_t._32_4_;
  puVar4[5] = uVar3;
  return 0;
}



// WARNING: Globals starting with '_' overlap smaller symbols at the same address

undefined4 SPI_NFI_Set_Configure(int *param_1)

{
  int *piVar1;
  int *piVar2;
  int *piVar3;
  int *piVar4;
  uint uVar5;
  uint uVar6;
  uint uVar7;
  int iVar8;
  int iVar9;
  int iVar10;
  
  piVar3 = param_1;
  piVar1 = (int *)_spi_nfi_conf_info_t;
  do {
    piVar4 = piVar1;
    piVar2 = piVar3;
    iVar10 = piVar2[1];
    iVar9 = piVar2[2];
    iVar8 = piVar2[3];
    *piVar4 = *piVar2;
    piVar4[1] = iVar10;
    piVar4[2] = iVar9;
    piVar4[3] = iVar8;
    piVar3 = piVar2 + 4;
    piVar1 = piVar4 + 4;
  } while (piVar3 != param_1 + 8);
  iVar8 = piVar2[5];
  piVar4[4] = *piVar3;
  piVar4[5] = iVar8;
  if (*param_1 == 0) {
    _DAT_bfa11000 = _DAT_bfa11000 & 0xfffffdff;
  }
  if (*param_1 == 1) {
    _DAT_bfa11000 = _DAT_bfa11000 | 0x200;
  }
  if (param_1[1] == 0) {
    _DAT_bfa11000 = _DAT_bfa11000 & 0xfffffeff;
  }
  if (param_1[1] == 1) {
    _DAT_bfa11000 = _DAT_bfa11000 | 0x100;
  }
  if (param_1[2] == 0) {
    _DAT_bfa11000 = _DAT_bfa11000 & 0xfffffffb;
  }
  if (param_1[2] == 1) {
    _DAT_bfa11000 = _DAT_bfa11000 | 4;
  }
  uVar5 = (uint)*(byte *)(param_1 + 3) << 8 | _DAT_bfa11004 & 0xfffff0ff;
  uVar7 = (uint)*(byte *)((int)param_1 + 0xd) << 0xc;
  uVar6 = uVar5 & 0xffff0fff;
  iVar8 = param_1[4];
  if (iVar8 == 0x10) {
    uVar6 = uVar5 & 0xffff0fcf;
    iVar8 = param_1[4];
  }
  _DAT_bfa11004 = uVar7 | uVar6;
  if (iVar8 == 0x1a) {
    _DAT_bfa11004 = uVar7 | uVar6 & 0xffffffcf | 0x10;
  }
  iVar8 = param_1[4];
  if (iVar8 == 0x1b) {
    _DAT_bfa11004 = _DAT_bfa11004 & 0xffffffcf | 0x20;
    iVar8 = param_1[4];
  }
  if (iVar8 == 0x1c) {
    _DAT_bfa11004 = _DAT_bfa11004 | 0x30;
  }
  iVar8 = param_1[5];
  if (iVar8 == 0x200) {
    _DAT_bfa11004 = _DAT_bfa11004 & 0xfffffffc;
    iVar8 = param_1[5];
  }
  if (iVar8 == 0x800) {
    _DAT_bfa11004 = _DAT_bfa11004 & 0xfffffffc | 1;
  }
  if (param_1[5] == 0x1000) {
    _DAT_bfa11004 = _DAT_bfa11004 & 0xfffffffc | 2;
  }
  _DAT_bfa11008 = (uint)*(byte *)(param_1 + 6) << 0xc | _DAT_bfa11008 & 0xffff0fff;
  if (param_1[7] == 0) {
    _DAT_bfa1122c = _DAT_bfa1122c & 0xfffeffff;
  }
  if (param_1[7] == 1) {
    _DAT_bfa1122c = _DAT_bfa1122c | 0x10000;
  }
  _DAT_bfa1122c = _DAT_bfa1122c & 0xffffe000 | param_1[8];
  return 0;
}



// WARNING: Globals starting with '_' overlap smaller symbols at the same address

void SPI_NFI_Reset(void)

{
  _DAT_bfa11008 = 3;
  return;
}



// WARNING: Globals starting with '_' overlap smaller symbols at the same address

undefined4 SPI_NFI_Init(void)

{
  _DAT_bfa11010 = _DAT_bfa11010 | 0x40;
  return 0;
}



void SPI_NFI_DEBUG_ENABLE(void)

{
  _SPI_NFI_DEBUG_FLAG = 1;
  return;
}



void SPI_NFI_DEBUG_DISABLE(void)

{
  _SPI_NFI_DEBUG_FLAG = 0;
  return;
}



// WARNING: Globals starting with '_' overlap smaller symbols at the same address

void SPI_NFI_TRIGGER(int param_1)

{
  if (param_1 == 0) {
    _DAT_bfa11008 = _DAT_bfa11008 & 0xfffffeff | 0x100;
    return;
  }
  _DAT_bfa11008 = _DAT_bfa11008 & 0xfffffdff | 0x200;
  return;
}



// WARNING: Globals starting with '_' overlap smaller symbols at the same address

undefined1 SPI_NFI_Write_SPI_NAND_page(int param_1,uint param_2,uint param_3,undefined4 param_4)

{
  uint uVar1;
  undefined1 uVar2;
  uint local_10;
  
  _DAT_bfa11528 = param_3 & 0xffff;
  if (_spi_nfi_conf_info_t._28_4_ == 0) {
    _DAT_bfa1153c =
         (uint)_spi_nfi_conf_info_t[0x18] * (_spi_nfi_conf_info_t._16_4_ + 0x200) * 0x10000 |
         _DAT_bfa1153c & 0xe000ffff;
  }
  if (_spi_nfi_conf_info_t._28_4_ == 1) {
    _DAT_bfa1153c =
         (uint)_spi_nfi_conf_info_t[0x18] * _spi_nfi_conf_info_t._32_4_ * 0x10000 |
         _DAT_bfa1153c & 0xe000ffff;
  }
  _DAT_bfa11524 = (param_2 & 0xff) << 8;
  _DAT_bfa11538 = param_1 << 0x10;
  _DAT_bfa11000 = _DAT_bfa11000 & 0xffff8ffd | 0x3001;
  _DAT_bfa11020 = 0x80;
  _DAT_bfa11080 = param_4;
  SPI_NFI_TRIGGER(1);
  delay1us(1);
  local_10 = 0;
  do {
    if ((_DAT_bfa11014 & 0x40) != 0) break;
    local_10 = local_10 + 1;
  } while (local_10 < 1000000);
  uVar1 = local_10;
  local_10 = 0;
  if (-1 < (int)(_DAT_bfa11550 << 5)) {
    do {
      local_10 = local_10 + 1;
      if (999999 < local_10) goto LAB_00010958;
    } while ((_DAT_bfa11550 & 0x4000000) == 0);
  }
  _DAT_bfa11550 = _DAT_bfa11550 | 0x4000000;
LAB_00010958:
  uVar2 = 2;
  if (local_10 != 1000000) {
    uVar2 = uVar1 == 1000000;
  }
  return uVar2;
}



// WARNING: Globals starting with '_' overlap smaller symbols at the same address

undefined4 SPI_NFI_Read_SPI_NAND_Page(int param_1,uint param_2,uint param_3,undefined4 param_4)

{
  int iVar1;
  int iVar2;
  undefined4 uVar3;
  
  _DAT_bfa11514 = param_3 & 0xffff;
  if (_spi_nfi_conf_info_t._28_4_ == 0) {
    _DAT_bfa1153c =
         (uint)_spi_nfi_conf_info_t[0x18] * (_spi_nfi_conf_info_t._16_4_ + 0x200) |
         _DAT_bfa1153c & 0xffffe000;
  }
  if (_spi_nfi_conf_info_t._28_4_ == 1) {
    _DAT_bfa1153c =
         (uint)_spi_nfi_conf_info_t[0x18] * _spi_nfi_conf_info_t._32_4_ | _DAT_bfa1153c & 0xffffe000
    ;
  }
  _DAT_bfa11510 = param_2 & 0xff;
  _DAT_bfa11538 = param_1 << 0x10;
  _DAT_bfa11000 = _DAT_bfa11000 & 0xffff8fff | 0x6003;
  _DAT_bfa11020 = 0;
  _DAT_bfa11080 = param_4;
  SPI_NFI_TRIGGER(0);
  iVar1 = 1;
  if ((int)(_DAT_bfa11550 << 6) < 0) {
    iVar1 = 0;
LAB_00010ab0:
    _DAT_bfa11550 = _DAT_bfa11550 | 0x2000000;
    uVar3 = 3;
    if (iVar1 != 1000000) {
      uVar3 = 0;
    }
  }
  else {
    do {
      iVar2 = iVar1 + 1;
      if ((_DAT_bfa11550 & 0x2000000) != 0) goto LAB_00010ab0;
      uVar3 = 3;
      iVar1 = iVar2;
    } while (iVar2 != 1000000);
  }
  iVar1 = 1;
  if ((_DAT_bfa11014 & 0x40) == 0) {
    do {
      if ((_DAT_bfa11014 & 0x40) != 0) {
        if (iVar1 == 1000000) {
          uVar3 = 1;
        }
        goto LAB_00010b38;
      }
      iVar1 = iVar1 + 1;
    } while (iVar1 != 1000000);
    uVar3 = 1;
  }
LAB_00010b38:
  delay1us(1);
  return uVar3;
}



// WARNING: Globals starting with '_' overlap smaller symbols at the same address

void SPI_NFI_DMA_WR_BYTE_SWAP(int param_1)

{
  _DAT_bfa11000 = param_1 << 3 | _DAT_bfa11000 & 0xfffffff7;
  return;
}



// WARNING: Globals starting with '_' overlap smaller symbols at the same address

void SPI_NFI_ECC_DATA_SOURCE_INV(int param_1)

{
  _DAT_bfa11000 = param_1 << 5 | _DAT_bfa11000 & 0xffffffdf;
  return;
}


