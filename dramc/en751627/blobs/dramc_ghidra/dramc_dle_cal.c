typedef unsigned char   undefined;

typedef unsigned char    bool;
typedef unsigned char    byte;
typedef unsigned int    dword;
typedef unsigned int    uint;
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



undefined DAT_bfb30000;
undefined DAT_bfb30004;
undefined DAT_bfb3000c;
undefined DAT_bfb30008;
undefined DAT_bfb30204;
undefined DAT_bfb20040;
undefined DAT_bfb20008;
undefined DAT_bfb203fc;
undefined DAT_bfb2007c;
undefined DAT_bfb200e4;
undefined4 DAT_000103f4;
undefined4 DAT_000103f8;
undefined4 global_dle_value;
undefined4 DAT_000103fc;
pointer dle_test_cases;
undefined1[64] dle_result;
undefined4 score;
undefined4 opt_dle_idx;
undefined4 opt_dle_value;

// WARNING: Globals starting with '_' overlap smaller symbols at the same address

undefined4 dramc_dma_test(undefined4 param_1,uint param_2)

{
  undefined4 uVar1;
  uint uVar2;
  uint uVar3;
  uint *puVar4;
  uint uVar5;
  int local_8;
  
  uVar5 = param_2 >> 2;
  if (uVar5 != 0) {
    uVar2 = 0;
    do {
      ((int *)(param_2 + 0xc0080000))[uVar2] = 0;
      uVar2 = uVar2 + 1;
    } while (uVar2 != uVar5);
    uVar2 = 0;
    do {
      *(uint *)(uVar2 * 4 + -0x3ff80000) = uVar2;
      uVar2 = uVar2 + 1;
    } while (uVar2 != uVar5);
  }
  _DAT_bfb30000 = 0x80000;
  _DAT_bfb30004 = param_2 + 0x80000;
  _DAT_bfb3000c = _DAT_bfb3000c | 4;
  _DAT_bfb30008 = _DAT_bfb30008 & 0xffc4 | 0x23 | param_2 << 0x10;
  local_8 = 0;
  do {
    if ((_DAT_bfb30204 & 1) != 0) break;
    local_8 = local_8 + 1;
  } while (local_8 != 0x7fffffff);
  _DAT_bfb30204 = 1;
  uVar1 = 0;
  if ((uVar5 != 0) && (uVar1 = 0xffffffff, *(int *)(param_2 + 0xc0080000) == 0)) {
    uVar2 = 0;
    puVar4 = (uint *)(param_2 + 0xc0080004);
    do {
      uVar2 = uVar2 + 1;
      if (uVar2 == uVar5) {
        _DAT_bfb30000 = 0x80000;
        _DAT_bfb30204 = 1;
        return 0;
      }
      uVar3 = *puVar4;
      puVar4 = puVar4 + 1;
    } while (uVar2 == uVar3);
    uVar1 = 0xffffffff;
  }
  return uVar1;
}



// WARNING: Globals starting with '_' overlap smaller symbols at the same address

undefined4 dramc_ta1(undefined4 param_1,uint param_2,int param_3)

{
  int iVar1;
  undefined4 uVar2;
  
  _DAT_bfb20040 = param_2 | _DAT_bfb20040 & 0xff000000;
  _DAT_bfb20008 = _DAT_bfb20008 | 0x20000000;
  iVar1 = 0x7fffffff;
  do {
    iVar1 = iVar1 + -1;
    if ((_DAT_bfb203fc & 0x400) != 0) break;
  } while (iVar1 != 0);
  pause_polling(1);
  uVar2 = 0xbfb20000;
  if ((param_3 != 0) && (uVar2 = 0xffffffff, (_DAT_bfb203fc & 0x4000) == 0)) {
    uVar2 = 0;
  }
  _DAT_bfb20008 = _DAT_bfb20008 & 0xdfffffff;
  return uVar2;
}



// WARNING: Globals starting with '_' overlap smaller symbols at the same address

void dle_factor_handler(uint param_1)

{
  _DAT_bfb2007c = (param_1 & 7) << 4 | _DAT_bfb2007c & 0xffffff8f;
  _DAT_bfb200e4 = ((param_1 & 0xf) >> 3) << 4 | _DAT_bfb200e4 & 0xffffffef;
  return;
}



bool dle_test(void)

{
  int iVar1;
  
  iVar1 = (*(code *)dle_test_cases)(DAT_000103f4,DAT_000103f8,DAT_000103fc);
  if (iVar1 != 0) {
    *(undefined4 *)(dle_result + global_dle_value * 4) = 0;
  }
  else {
    *(undefined4 *)(dle_result + global_dle_value * 4) = 1;
  }
  global_dle_value = global_dle_value + 1;
  return iVar1 == 0;
}



void dle_calib_reset(void)

{
  return;
}



undefined4 dle_calib(void)

{
  undefined3 extraout_var;
  bool bVar1;
  uint uVar2;
  
  uVar2 = 0;
  do {
    dle_factor_handler(uVar2);
    uVar2 = uVar2 + 1;
    bVar1 = dle_test();
    score = CONCAT31(extraout_var,bVar1) + score;
  } while (uVar2 != 0x10);
  return 1;
}



undefined4 do_dle_calib(void)

{
  undefined1 *puVar1;
  undefined4 uVar2;
  uint uVar3;
  
  global_dle_value = 0;
  dle_calib();
  puVar1 = dle_result;
  uVar3 = 1;
  while ((*(int *)puVar1 != 1 || (*(int *)((int)puVar1 + 4) != 1))) {
    uVar3 = uVar3 + 1;
    puVar1 = (undefined1 *)((int)puVar1 + 4);
    if (uVar3 == 0x10) {
LAB_000103b0:
      opt_dle_value = opt_dle_idx;
      if (opt_dle_idx == 0) {
        prom_puts("%cannot find opt_dle value\n");
        uVar2 = 0xffffffff;
      }
      else {
        dle_factor_handler(opt_dle_idx);
        uVar2 = 0;
      }
      return uVar2;
    }
  }
  if ((int)opt_dle_idx < (int)uVar3) {
    opt_dle_idx = uVar3;
  }
  goto LAB_000103b0;
}


