typedef unsigned char   undefined;

typedef unsigned char    byte;
typedef unsigned int    dword;
typedef unsigned int    uint;
typedef unsigned char    undefined1;
typedef unsigned short    undefined2;
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



undefined DAT_bfa10004;
undefined DAT_bfa10028;
undefined DAT_bfa1002c;
undefined DAT_bfa10030;
undefined DAT_bfa10024;
undefined DAT_bfa10034;
undefined DAT_bfa10038;
undefined DAT_bfa10018;
undefined DAT_bfa10014;
undefined DAT_bfa10020;
undefined DAT_bfa1003c;
undefined DAT_bfa10044;
undefined DAT_bfa10040;
undefined1 is_S25FL256S;
undefined1 is_N25Q256;
undefined1[12] mtd;
undefined4 cnt.2844;
undefined4 program_speed;
undefined DAT_bfb00284;
undefined DAT_bfb00064;
undefined DAT_bfa2011c;
undefined DAT_bfa1001c;
undefined2 flash_tables;
undefined1 is_W25Q256;
undefined1 is_MX25L256;
undefined DAT_bfa1008c;
undefined DAT_bfa10084;
undefined DAT_bfa10000;
undefined DAT_bfa1009c;
undefined4 DAT_000116dc;
undefined4 DAT_000116e0;
undefined4 DAT_000116e4;

// WARNING: Globals starting with '_' overlap smaller symbols at the same address

undefined4
spiflash_read_internal(int *param_1,int param_2,int param_3,int *param_4,undefined4 param_5)

{
  undefined4 uVar1;
  
  uVar1 = 0;
  if ((param_3 != 0) && (uVar1 = 0xffffffff, (uint)(param_3 + param_2) <= (uint)param_1[1])) {
    *param_4 = param_3;
    _DAT_bfa10004 = 1;
    memcpy4(param_5,param_2 + *param_1);
    uVar1 = 0;
  }
  return uVar1;
}



// WARNING: Globals starting with '_' overlap smaller symbols at the same address

void send_single_opcode_cmd(uint param_1)

{
  do {
  } while (_DAT_bfa1002c != 0);
  do {
  } while (_DAT_bfa10024 == 0);
  do {
  } while (_DAT_bfa10024 == 0);
  do {
  } while (_DAT_bfa10024 == 0);
  do {
  } while (_DAT_bfa10034 != 0);
  _DAT_bfa10038 = param_1 & 0xff;
  do {
  } while (_DAT_bfa10024 == 0);
  _DAT_bfa10028 = 0x405;
  _DAT_bfa10030 = 1;
  do {
  } while (_DAT_bfa10024 == 0);
  return;
}



// WARNING: Globals starting with '_' overlap smaller symbols at the same address

void sf_manual_begin(void)

{
  _DAT_bfa10004 = 0;
  do {
  } while (_DAT_bfa10018 != 0);
  _DAT_bfa10014 = 9;
  _DAT_bfa10020 = 1;
  return;
}



// WARNING: Globals starting with '_' overlap smaller symbols at the same address

void sf_manual_end(void)

{
  _DAT_bfa10014 = 0;
  _DAT_bfa10020 = 0;
  _DAT_bfa10004 = 1;
  return;
}



void sf_manual_wren_begin(void)

{
  sf_manual_begin();
  send_single_opcode_cmd(6);
  return;
}



// WARNING: Globals starting with '_' overlap smaller symbols at the same address

void sf_manual_wren_end(void)

{
  do {
    do {
    } while (_DAT_bfa1002c != 0);
    do {
    } while (_DAT_bfa10024 == 0);
    do {
    } while (_DAT_bfa10024 == 0);
    do {
    } while (_DAT_bfa10024 == 0);
    do {
    } while (_DAT_bfa10034 != 0);
    _DAT_bfa10038 = 5;
    do {
    } while (_DAT_bfa10024 == 0);
    do {
    } while (_DAT_bfa1003c != 0);
    _DAT_bfa10040 = 1;
    do {
    } while (_DAT_bfa10024 == 0);
    _DAT_bfa10028 = 0x405;
    _DAT_bfa10030 = 1;
    do {
    } while (_DAT_bfa10024 == 0);
  } while ((_DAT_bfa10044 & 1) != 0);
  sf_manual_end();
  return;
}



// WARNING: Globals starting with '_' overlap smaller symbols at the same address

void enter_4Byte_mode(void)

{
  if (is_S25FL256S == '\0') {
    if (is_N25Q256 == '\0') {
      sf_manual_begin();
      send_single_opcode_cmd(0xb7);
      sf_manual_end();
    }
    else {
      sf_manual_wren_begin();
      send_single_opcode_cmd(0xb7);
      sf_manual_wren_end();
    }
  }
  else {
    sf_manual_wren_begin();
    do {
    } while (_DAT_bfa1002c != 0);
    do {
    } while (_DAT_bfa10024 == 0);
    do {
    } while (_DAT_bfa10024 == 0);
    do {
    } while (_DAT_bfa10024 == 0);
    do {
    } while (_DAT_bfa10034 != 0);
    do {
    } while (_DAT_bfa10024 == 0);
    _DAT_bfa10038 = 0x80;
    do {
    } while (_DAT_bfa10024 == 0);
    _DAT_bfa10028 = 0x405;
    _DAT_bfa10030 = 1;
    do {
    } while (_DAT_bfa10024 == 0);
    sf_manual_wren_end();
    sf_manual_begin();
    send_single_opcode_cmd(4);
    sf_manual_end();
  }
  return;
}



// WARNING: Globals starting with '_' overlap smaller symbols at the same address

undefined4 spiflash_erase(uint param_1,uint param_2)

{
  undefined4 uVar1;
  
  if ((param_1 & mtd._8_4_ - 1) != 0) {
    param_1 = param_1 & -mtd._8_4_;
  }
  if ((param_2 & mtd._8_4_ - 1) != 0) {
    param_2 = (param_2 & -mtd._8_4_) + mtd._8_4_;
  }
  uVar1 = 0xffffffff;
  if ((param_2 + param_1 <= (uint)mtd._4_4_) && (uVar1 = 0, param_2 != 0)) {
    do {
      if (mtd._8_4_ != 0x10000) {
        return 0xffffffff;
      }
      sf_manual_wren_begin();
      do {
      } while (_DAT_bfa1002c != 0);
      do {
      } while (_DAT_bfa10024 == 0);
      do {
      } while (_DAT_bfa10024 == 0);
      do {
      } while (_DAT_bfa10024 == 0);
      do {
      } while (_DAT_bfa10034 != 0);
      do {
      } while (_DAT_bfa10024 == 0);
      _DAT_bfa10038 = param_1 & 0xff;
      do {
      } while (_DAT_bfa10024 == 0);
      _DAT_bfa10028 = 0x405;
      _DAT_bfa10030 = 1;
      do {
      } while (_DAT_bfa10024 == 0);
      sf_manual_wren_end();
      param_2 = param_2 - mtd._8_4_;
      param_1 = param_1 + mtd._8_4_;
      if (param_2 == 0) {
        return 0;
      }
      uVar1 = 0xffffffff;
    } while (param_2 + param_1 <= (uint)mtd._4_4_);
  }
  return uVar1;
}



// WARNING: Globals starting with '_' overlap smaller symbols at the same address

undefined4 spiflash_read(uint param_1,uint param_2,undefined4 *param_3,undefined1 *param_4)

{
  uint uVar1;
  
  uVar1 = param_1;
  if (param_1 + 0x44000000 < 0x4000000) {
    uVar1 = param_1 & 0x3ffffff;
    if ((param_1 & 0xbfc00000) == 0xbfc00000) {
      uVar1 = param_1 & 0x3fffff;
    }
  }
  if ((param_2 != 0) && (uVar1 + param_2 <= (uint)mtd._4_4_)) {
    sf_manual_begin();
    do {
    } while (_DAT_bfa1002c != 0);
    do {
    } while (_DAT_bfa10024 == 0);
    do {
    } while (_DAT_bfa10024 == 0);
    do {
    } while (_DAT_bfa10024 == 0);
    do {
    } while (_DAT_bfa10034 != 0);
    do {
    } while (_DAT_bfa10024 == 0);
    do {
    } while (_DAT_bfa10024 == 0);
    _DAT_bfa10038 = 0;
    do {
      if ((param_2 & 0xff) == 0) {
        do {
        } while (_DAT_bfa1002c != 0);
        do {
        } while (_DAT_bfa10024 == 0);
        do {
          do {
          } while (_DAT_bfa1003c != 0);
          *param_4 = (char)_DAT_bfa10044;
          param_4 = param_4 + 1;
          param_2 = param_2 - 1;
        } while ((param_2 & 0xff) != 0);
      }
      else {
        do {
        } while (_DAT_bfa1002c != 0);
        do {
        } while (_DAT_bfa10024 == 0);
        do {
          do {
          } while (_DAT_bfa1003c != 0);
          *param_4 = (char)_DAT_bfa10044;
          param_4 = param_4 + 1;
          param_2 = param_2 - 1;
        } while ((param_2 & 0xff) != 0);
      }
      _DAT_bfa10040 = 1;
    } while (param_2 != 0);
    do {
    } while (_DAT_bfa1002c != 0);
    do {
    } while (_DAT_bfa10024 == 0);
    _DAT_bfa10028 = 0x405;
    _DAT_bfa10030 = 1;
    do {
    } while (_DAT_bfa10024 == 0);
    sf_manual_end();
    *param_3 = 0;
  }
  return 0;
}



// WARNING: Globals starting with '_' overlap smaller symbols at the same address

undefined4 spiflash_write(uint param_1,uint param_2,undefined4 *param_3,byte *param_4)

{
  int iVar1;
  undefined4 uVar2;
  uint uVar3;
  byte *pbVar4;
  uint uVar5;
  
  iVar1 = program_speed;
  *param_3 = 0;
  uVar2 = 0;
  if ((param_2 != 0) && (uVar2 = 0xffffffff, param_2 + param_1 <= (uint)mtd._4_4_)) {
    do {
      cnt_2844 = cnt_2844 + 1;
      if ((param_1 & 0xff) == 0) {
        uVar3 = 0x100;
        if (param_2 < 0x101) {
          uVar3 = param_2;
        }
      }
      else {
        uVar3 = 0x100 - (param_1 & 0xff);
        if (param_2 < uVar3) {
          uVar3 = param_2;
        }
      }
      param_2 = param_2 - uVar3;
      if (iVar1 == 0) {
        sf_manual_wren_begin();
        do {
        } while (_DAT_bfa1002c != 0);
        do {
        } while (_DAT_bfa10024 == 0);
        do {
        } while (_DAT_bfa10024 == 0);
        do {
        } while (_DAT_bfa10024 == 0);
        do {
        } while (_DAT_bfa10034 != 0);
        do {
        } while (_DAT_bfa10024 == 0);
        do {
          pbVar4 = param_4;
          uVar5 = uVar3;
        } while (_DAT_bfa10024 == 0);
        do {
          _DAT_bfa10038 = (uint)*pbVar4;
          uVar5 = uVar5 - 1;
          pbVar4 = pbVar4 + 1;
        } while (uVar5 != 0);
        do {
        } while (_DAT_bfa10024 == 0);
        _DAT_bfa10028 = 0x405;
        _DAT_bfa10030 = 1;
        do {
        } while (_DAT_bfa10024 == 0);
        sf_manual_wren_end();
      }
      else {
        sf_manual_wren_begin();
        do {
        } while (_DAT_bfa1002c != 0);
        do {
        } while (_DAT_bfa10024 == 0);
        do {
        } while (_DAT_bfa10024 == 0);
        do {
        } while (_DAT_bfa10024 == 0);
        do {
        } while (_DAT_bfa10034 != 0);
        do {
        } while (_DAT_bfa10024 == 0);
        do {
          pbVar4 = param_4;
          uVar5 = uVar3;
        } while (_DAT_bfa10024 == 0);
        do {
          _DAT_bfa10038 = (uint)*pbVar4;
          uVar5 = uVar5 - 1;
          pbVar4 = pbVar4 + 1;
        } while (uVar5 != 0);
        do {
        } while (_DAT_bfa10024 == 0);
        _DAT_bfa10028 = 0x405;
        _DAT_bfa10030 = 1;
        do {
        } while (_DAT_bfa10024 == 0);
        sf_manual_wren_end();
      }
      param_4 = param_4 + uVar3;
      param_1 = param_1 + uVar3;
    } while (param_2 != 0);
    uVar2 = 0;
  }
  return uVar2;
}



undefined1 ReadSPICache(uint param_1)

{
  int local_10;
  undefined2 local_c [4];
  
  local_10 = 0;
  local_c[0] = 0;
  spiflash_read_internal((int *)mtd,param_1 & 0x3ffffff,1,&local_10,local_c);
  return local_c[0]._0_1_;
}



undefined1 ReadSPIByte(uint param_1)

{
  int local_10;
  undefined2 local_c [4];
  
  local_10 = 0;
  local_c[0] = 0;
  spiflash_read_internal((int *)mtd,param_1 & 0x3ffffff,1,&local_10,local_c);
  return local_c[0]._0_1_;
}



undefined4 ReadSPIDWord(uint param_1)

{
  int local_10;
  undefined4 local_c [2];
  
  local_10 = 0;
  local_c[0] = 0;
  spiflash_read_internal((int *)mtd,param_1 & 0x3ffffff,4,&local_10,local_c);
  return local_c[0];
}



void Quad_Mode_Enable(void)

{
  return;
}



// WARNING: Globals starting with '_' overlap smaller symbols at the same address

void spi_nor_set_clock_speed(int param_1)

{
  uint uVar1;
  
  if (-1 < _DAT_bfb00284 << 9) {
    uVar1 = 400;
    if (((((_DAT_bfb00064 & 0xffff0000) != 0x80000) && ((_DAT_bfb00064 & 0xffff0000) != 0x90000)) &&
        ((_DAT_bfb00064 & 0xffff0000) != 0xb0000)) &&
       (uVar1 = 400, (_DAT_bfb00064 & 0xffff0000) != 0xa0000)) {
      uVar1 = 500;
    }
    if (param_1 << 1 == 0) {
      trap(7);
    }
    _DAT_bfa2011c = _DAT_bfa2011c & 0xffff0000 | 1 | uVar1 / (uint)(param_1 << 1) << 8;
  }
  return;
}



// WARNING: Globals starting with '_' overlap smaller symbols at the same address

undefined4 spiflash_init(undefined4 param_1)

{
  ushort *puVar1;
  int iVar2;
  uint uVar3;
  uint uVar4;
  int iVar5;
  uint uVar6;
  
  mtd._4_4_ = 0;
  _DAT_bfa1001c = 1;
  mtd._0_4_ = param_1;
  sf_manual_begin();
  do {
  } while (_DAT_bfa1002c != 0);
  do {
  } while (_DAT_bfa10024 == 0);
  do {
  } while (_DAT_bfa10024 == 0);
  do {
  } while (_DAT_bfa10024 == 0);
  do {
  } while (_DAT_bfa10034 != 0);
  _DAT_bfa10038 = 0x9f;
  do {
  } while (_DAT_bfa10024 == 0);
  do {
  } while (_DAT_bfa1003c != 0);
  uVar3 = _DAT_bfa10044 & 0xff;
  uVar4 = _DAT_bfa10044 & 0xff;
  uVar6 = _DAT_bfa10044 & 0xff;
  _DAT_bfa10040 = 1;
  do {
  } while (_DAT_bfa10024 == 0);
  _DAT_bfa10028 = 0x405;
  _DAT_bfa10030 = 1;
  do {
  } while (_DAT_bfa10024 == 0);
  sf_manual_end();
  puVar1 = &flash_tables;
  iVar5 = 0;
  uVar6 = uVar4 << 8 | uVar6;
  while ((uVar3 != *puVar1 || (uVar6 != puVar1[1]))) {
    iVar5 = iVar5 + 1;
    puVar1 = puVar1 + 0xc;
    if (iVar5 == 0x2d) {
      return 0xffffffff;
    }
  }
  if ((uVar3 == 0xef) && (uVar6 == 0x4019)) {
    is_W25Q256 = 1;
  }
  if ((uVar3 == 0xc2) && (uVar6 == 0x2019)) {
    is_MX25L256 = 1;
  }
  if ((uVar3 == 1) && (uVar6 == 0x219)) {
    is_S25FL256S = 1;
  }
  if ((uVar3 == 0x20) && (uVar6 == 0xba19)) {
    is_N25Q256 = 1;
  }
  if (iVar5 < 0) {
    return 0xffffffff;
  }
  iVar2 = (&DAT_000116dc)[iVar5 * 6];
  if (0x1ffffff < iVar2) {
    enter_4Byte_mode();
  }
  _DAT_bfa1008c = 1;
  _DAT_bfa10084 = (uint)(0x1ffffff < iVar2);
  _DAT_bfa10000 = 0;
  sf_manual_wren_begin();
  do {
  } while (_DAT_bfa1002c != 0);
  do {
  } while (_DAT_bfa10024 == 0);
  do {
  } while (_DAT_bfa10024 == 0);
  do {
  } while (_DAT_bfa10024 == 0);
  do {
  } while (_DAT_bfa10034 != 0);
  do {
  } while (_DAT_bfa10024 == 0);
  _DAT_bfa10038 = 0;
  do {
  } while (_DAT_bfa10024 == 0);
  _DAT_bfa10028 = 0x405;
  _DAT_bfa10030 = 1;
  do {
  } while (_DAT_bfa10024 == 0);
  sf_manual_wren_end();
  spi_nor_set_clock_speed(0x19);
  _DAT_bfa1009c = 9;
  iVar2 = (&DAT_000116e4)[iVar5 * 6];
  if (iVar2 == 5) {
    (&DAT_000116e4)[iVar5 * 6] = 3;
  }
  else if (iVar2 == 6) {
    (&DAT_000116e4)[iVar5 * 6] = 3;
  }
  else if (iVar2 == 4) {
    (&DAT_000116e4)[iVar5 * 6] = 2;
  }
  mtd._4_4_ = (&DAT_000116dc)[iVar5 * 6];
  mtd._8_4_ = (&DAT_000116e0)[iVar5 * 6];
  return 0;
}


