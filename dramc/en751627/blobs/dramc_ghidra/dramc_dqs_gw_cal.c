typedef unsigned char   undefined;

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



undefined DAT_bfb2003c;
undefined DAT_bfb20040;
undefined DAT_bfb20008;
undefined DAT_bfb203fc;
undefined DAT_bfb203c0;
undefined DAT_bfb200e0;
undefined DAT_bfb20124;
undefined DAT_bfb20094;
undefined DAT_bfb201e4;
undefined DAT_bfb200f0;
undefined DAT_bfb200f4;
pointer dqs_gw_test_cases_1;
undefined DAT_00010564;
undefined1[180] dqs_gw;
undefined4 DAT_00010568;
undefined4 dqs_gw_coarse;
undefined4 DAT_0001056c;
undefined4 dqs_gw_fine;
undefined4 dqs_gw_fine_cnt;
undefined4 score;
undefined DAT_bfb20048;
undefined4 opt_gw_coarse_value;
undefined4 opt_gw_fine_value;

// WARNING: Globals starting with '_' overlap smaller symbols at the same address

undefined4 dramc_ta2(uint param_1,uint param_2)

{
  int iVar1;
  undefined4 uVar2;
  
  _DAT_bfb2003c = (param_1 & 0x1fffffff) >> 8 | 0x55000000;
  _DAT_bfb20040 = param_2 & 0xffffff | 0xaa000000;
  _DAT_bfb20008 = _DAT_bfb20008 | 0x40000000;
  iVar1 = 0x7fffffff;
  do {
    iVar1 = iVar1 + -1;
    if ((_DAT_bfb203fc & 0x400) != 0) break;
  } while (iVar1 != 0);
  pause_polling(1);
  _DAT_bfb20008 = _DAT_bfb20008 & 0xbfffffff;
  uVar2 = 0xffffffff;
  if (_DAT_bfb203c0 == 0x78787878) {
    uVar2 = 0;
  }
  return uVar2;
}



// WARNING: Globals starting with '_' overlap smaller symbols at the same address

void dqsi_gw_dly_coarse_factor_handler(uint param_1)

{
  _DAT_bfb200e0 = ((param_1 & 0x1f) >> 2) << 0x18 | _DAT_bfb200e0 & 0xf8ffffff;
  _DAT_bfb20124 = param_1 & 3 | _DAT_bfb20124 & 0xffffffcc | (param_1 & 3) << 4;
  return;
}



// WARNING: Globals starting with '_' overlap smaller symbols at the same address

void dqsi_gw_dly_fine_factor_handler(uint param_1)

{
  uint uVar1;
  
  uVar1 = param_1 & 0x7f;
  _DAT_bfb20094 = uVar1 | _DAT_bfb20094 & 0x80000000 | uVar1 << 8 | uVar1 << 0x10 | uVar1 << 0x18;
  return;
}



// WARNING: Globals starting with '_' overlap smaller symbols at the same address

void dqs_gw_counter_reset(void)

{
  _DAT_bfb201e4 = _DAT_bfb201e4 & 0xfffffdff;
  _DAT_bfb200f0 = _DAT_bfb200f0 | 0x10000000;
  _DAT_bfb200f4 = _DAT_bfb200f4 | 0x2000000;
  pause_polling(1);
  _DAT_bfb200f0 = _DAT_bfb200f0 & 0xefffffff;
  _DAT_bfb200f4 = _DAT_bfb200f4 & 0xfdffffff;
  return;
}



// WARNING: Globals starting with '_' overlap smaller symbols at the same address

undefined4 dqs_gw_calib(void)

{
  int iVar1;
  uint uVar2;
  uint uVar3;
  
  uVar3 = 6;
  do {
    dqsi_gw_dly_coarse_factor_handler(uVar3);
    uVar2 = 0;
    do {
      dqsi_gw_dly_fine_factor_handler(uVar2);
      dqs_gw_counter_reset();
      _DAT_00010564 = 0xc0080000;
      iVar1 = (*(code *)dqs_gw_test_cases_1)(0xc0080000,DAT_00010568,DAT_0001056c);
      if (-1 < iVar1) {
        *(uint *)(dqs_gw + dqs_gw_coarse * 4) =
             1 << (dqs_gw_fine & 0x1f) | *(uint *)(dqs_gw + dqs_gw_coarse * 4);
      }
      dqs_gw_fine = dqs_gw_fine + 1;
      dqs_gw_fine_cnt = dqs_gw_fine_cnt + 1;
      if (dqs_gw_fine < 0x20) {
        if (0x5f < dqs_gw_fine_cnt) {
LAB_000102a0:
          dqs_gw_fine_cnt = 0;
          goto LAB_000102a4;
        }
      }
      else {
        if (0x5f < dqs_gw_fine_cnt) goto LAB_000102a0;
LAB_000102a4:
        dqs_gw_coarse = dqs_gw_coarse + 1;
        dqs_gw_fine = 0;
      }
      score = (uint)(-1 < iVar1) + score;
      uVar2 = uVar2 + 1;
    } while (uVar2 != 0x60);
    dqs_gw_fine = 0;
    uVar3 = uVar3 + 1;
    dqs_gw_fine_cnt = 0;
    if (uVar3 == 0x15) {
      dqs_gw_coarse = 0;
      dqs_gw_counter_reset();
      return 1;
    }
  } while( true );
}



// WARNING: Globals starting with '_' overlap smaller symbols at the same address

undefined4 do_dqs_gw_calib_1(void)

{
  undefined1 *puVar1;
  uint uVar2;
  undefined4 uVar3;
  int iVar4;
  uint uVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  int iVar9;
  int iVar10;
  int unaff_s0;
  uint unaff_s1;
  
  dqs_gw_coarse = 0;
  dqs_gw_fine = 0;
  dqs_gw_fine_cnt = 0;
  puVar1 = dqs_gw;
  do {
    *(undefined4 *)puVar1 = 0;
    puVar1 = (undefined1 *)((int)puVar1 + 4);
  } while ((uint *)puVar1 != &opt_gw_coarse_value);
  _DAT_bfb20048 = _DAT_bfb20048 & 0xffff7fff;
  _DAT_bfb200e0 = _DAT_bfb200e0 | 0x10000000;
  _DAT_bfb201e4 = _DAT_bfb201e4 | 0x100;
  dqs_gw_calib();
  iVar8 = 0;
  iVar10 = 0;
  iVar9 = 0;
  do {
    iVar7 = 0;
    iVar6 = 0;
    do {
      iVar4 = 0;
      uVar2 = 0;
      do {
        uVar5 = uVar2 & 0x1f;
        uVar2 = uVar2 + 1;
        iVar4 = iVar4 + (uint)((1 << uVar5 & *(uint *)(dqs_gw + (iVar8 + iVar6) * 4)) != 0);
      } while (uVar2 != 0x20);
      iVar7 = iVar7 + iVar4;
      iVar6 = iVar6 + 1;
    } while (iVar6 != 3);
    if (iVar10 <= iVar7) {
      unaff_s0 = iVar9;
      iVar10 = iVar7;
    }
    iVar9 = iVar9 + 1;
    iVar8 = iVar8 + 3;
  } while (iVar9 != 0xf);
  uVar3 = 0xffffffff;
  if (iVar10 != 0) {
    iVar8 = 0;
    do {
      uVar2 = 1;
      if ((*(uint *)(dqs_gw + (iVar8 + unaff_s0 * 3) * 4) & 1) != 0) {
        uVar2 = 0;
LAB_000104e4:
        unaff_s1 = iVar8 * 0x20 + uVar2 + iVar10 / 2;
        break;
      }
      uVar5 = 2;
      do {
        if ((uVar5 & *(uint *)(dqs_gw + (iVar8 + unaff_s0 * 3) * 4)) != 0) {
          if (uVar2 != 0xffffffff) goto LAB_000104e4;
          break;
        }
        uVar2 = uVar2 + 1;
        uVar5 = 1 << (uVar2 & 0x1f);
      } while (uVar2 != 0x20);
      iVar8 = iVar8 + 1;
    } while (iVar8 != 3);
    opt_gw_coarse_value = unaff_s0 + 6;
    opt_gw_fine_value = unaff_s1;
    dqsi_gw_dly_coarse_factor_handler(opt_gw_coarse_value);
    dqsi_gw_dly_fine_factor_handler(opt_gw_fine_value);
    uVar3 = 0;
  }
  _DAT_bfb201e4 = _DAT_bfb201e4 & 0xfffffeff;
  _DAT_bfb20048 = _DAT_bfb20048 | 0x8000;
  return uVar3;
}


