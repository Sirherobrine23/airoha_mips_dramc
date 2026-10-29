typedef unsigned char   undefined;

typedef unsigned char    byte;
typedef unsigned int    dword;
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


undefined DAT_bfb20004;
undefined DAT_a0000000;
undefined DAT_bfb00074;
undefined DAT_bfb200f4;
undefined4 dram_type;
undefined DAT_bfb20044;
undefined DAT_bfb201e8;
undefined4 PKG_type;
undefined4 dram_size;
undefined $LC2;
undefined $LC5;
undefined $LC6;
undefined DAT_bfb00284;
undefined DAT_bfb20018;
undefined DAT_bfb00040;
undefined DAT_bfbf8214;
undefined DAT_c0080000;
undefined DAT_bfb20618;
undefined DAT_bfb2060c;
undefined DAT_bfb201c0;
undefined $LC8;
undefined DAT_bfb201ec;
undefined DAT_bfa20174;
undefined DAT_bfa2ff28;
undefined DAT_bfb20210;
undefined opt_gw_coarse_value;
undefined opt_gw_fine_value;
undefined opt_dle_value;
undefined $LC24;

undefined4 prom_printf_s(void)

{
  prom_puts();
  return 0;
}



// WARNING: Globals starting with '_' overlap smaller symbols at the same address

undefined4 check_column_bank(void)

{
  bool bVar1;
  int iVar2;
  int iVar3;
  int *piVar4;
  uint uVar5;
  uint uVar6;
  
  if (dram_type != 1) {
    return 0;
  }
  uVar6 = 8;
  while( true ) {
    _DAT_bfb20004 = (uVar6 - 7) * 0x100 | _DAT_bfb20004 & 0xfffffcff;
    pause_polling(2);
    _DAT_a0000000 = 0x12345678;
    pause_polling(2);
    uVar5 = uVar6 + 1;
    piVar4 = (int *)(&DAT_a0000000 + (1 << (uVar5 & 0x1f)));
    *piVar4 = -0x789abcdf;
    pause_polling(2);
    iVar3 = *piVar4;
    iVar2 = _DAT_a0000000;
    while (_DAT_a0000000 = iVar2, iVar3 != -0x789abcdf) {
      *piVar4 = -0x789abcdf;
      iVar2 = _DAT_a0000000;
      iVar3 = *piVar4;
    }
    if (iVar2 != 0x12345678) break;
    bVar1 = true;
    if (uVar5 == 10) goto LAB_00010170;
    _DAT_a0000000 = 0x12345678;
    uVar6 = uVar5;
  }
  if (iVar2 != -0x789abcdf) {
    prom_printf_s();
    prom_printf_s();
    prom_print_hex(&DAT_a0000000,8);
    prom_printf_s();
    prom_print_hex(iVar2,8);
    prom_printf_s();
    return 0xffffffff;
  }
  bVar1 = false;
  uVar5 = uVar6;
LAB_00010170:
  _DAT_bfb20004 = (uVar5 - 8) * 0x100 | _DAT_bfb20004 & 0xfffffcff;
  pause_polling(2);
  if (dram_type == 1) {
    if (!bVar1) {
      return 0;
    }
    _DAT_bfb00074 = _DAT_bfb00074 & 0xfffffffc | 1;
    pause_polling(2);
  }
  else if (!bVar1) {
    return 0;
  }
  _DAT_bfb200f4 = _DAT_bfb200f4 | 0x1000000;
  pause_polling(2);
  _DAT_a0000000 = 0x12345678;
  pause_polling(2);
  piVar4 = (int *)(&DAT_a0000000 + (4 << (uVar5 + 1 & 0x1f)));
  *piVar4 = -0x789abcdf;
  pause_polling(2);
  iVar3 = *piVar4;
  iVar2 = _DAT_a0000000;
  while (_DAT_a0000000 = iVar2, iVar3 != -0x789abcdf) {
    *piVar4 = -0x789abcdf;
    iVar2 = _DAT_a0000000;
    iVar3 = *piVar4;
  }
  if (iVar2 == 0x12345678) {
    iVar3 = 3;
  }
  else {
    iVar3 = 2;
    if (iVar2 != -0x789abcdf) {
      prom_printf_s();
      prom_printf_s();
      prom_print_hex(&DAT_a0000000,8);
      prom_printf_s();
      prom_print_hex(iVar2,8);
      prom_printf_s();
      return 0xffffffff;
    }
  }
  _DAT_bfb200f4 = (iVar3 + -2) * 0x1000000 | _DAT_bfb200f4 & 0xfeffffff;
  pause_polling(2);
  return 0;
}



// WARNING: Globals starting with '_' overlap smaller symbols at the same address

void set_TRFC(void)

{
  if (PKG_type == 3) {
    if (dram_size == 0x40) {
      _DAT_bfb20044 = 0xa88d0481;
      _DAT_bfb201e8 = 0x600;
    }
    else if (dram_size == 0x80) {
      _DAT_bfb20044 = 0xa8830481;
      _DAT_bfb201e8 = 0x610;
    }
    else if (dram_size == 0x100) {
      _DAT_bfb20044 = 0xa8800481;
      _DAT_bfb201e8 = 0x620;
    }
    else if (dram_size == 0x200) {
      _DAT_bfb20044 = 0xa8850481;
      _DAT_bfb201e8 = 0x640;
    }
  }
  else if (dram_size == 0x40) {
    _DAT_bfb20044 = 0xa8830481;
    _DAT_bfb201e8 = 0x610;
  }
  else if (dram_size == 0x80) {
    _DAT_bfb20044 = 0xa88a0481;
    _DAT_bfb201e8 = 0x610;
  }
  else if (dram_size == 0x100) {
    _DAT_bfb20044 = 0xa88b0481;
    _DAT_bfb201e8 = 0x620;
  }
  else if (dram_size == 0x200) {
    _DAT_bfb20044 = 0xa8890481;
    _DAT_bfb201e8 = 0x650;
  }
  prom_puts("Set new TRFC.\n");
  return;
}



void dramc_reg_dump(void)

{
  int iVar1;
  
  iVar1 = 0;
  do {
    prom_puts(&_LC5);
    prom_print_hex(iVar1,3);
    prom_puts(&_LC6);
    prom_print_hex(*(undefined4 *)(iVar1 + -0x404e0000),8);
    prom_puts(&_LC2);
    iVar1 = iVar1 + 4;
  } while (iVar1 != 0x400);
  iVar1 = 0x600;
  do {
    prom_puts(&_LC5);
    prom_print_hex(iVar1,3);
    prom_puts(&_LC6);
    prom_print_hex(*(undefined4 *)(iVar1 + -0x404e0000),8);
    prom_puts(&_LC2);
    iVar1 = iVar1 + 4;
  } while (iVar1 != 0x650);
  return;
}



void dramc_calib(void)

{
  int iVar1;
  
  iVar1 = do_dqs_gw_calib_1();
  if (-1 < iVar1) {
    dle_factor_handler(8);
    iVar1 = do_sw_rx_dq_dqs_calib();
    if ((-1 < iVar1) && (iVar1 = do_dle_calib(), -1 < iVar1)) {
      do_sw_tx_dq_dqs_calib();
    }
  }
  return;
}



// WARNING: Globals starting with '_' overlap smaller symbols at the same address

undefined4 main(void)

{
  bool bVar1;
  int *piVar2;
  int iVar3;
  int iVar4;
  undefined4 *puVar5;
  uint uVar6;
  undefined4 uVar7;
  
  dram_type = 0;
  if ((int)(_DAT_bfb00284 << 9) < 0) {
    piVar2 = (int *)&DAT_bfa2ff28;
  }
  else {
    piVar2 = (int *)&DAT_bfa20174;
  }
  PKG_type = 3;
  if (-1 < *piVar2 << 0x10) {
    PKG_type = 1;
  }
  pause_polling(200);
  iVar3 = en7512_dramc_init();
  if (iVar3 < 0) {
    return 0;
  }
  uVar7 = 0;
  iVar3 = dramc_calib();
  if (iVar3 < 0) {
    prom_puts("%dqs_gw (coarse/fine): ");
    prom_print_dec(_opt_gw_coarse_value);
    prom_puts(&_LC8);
    prom_print_dec(_opt_gw_fine_value);
    prom_puts(&_LC2);
    prom_puts("%DQS input dly:\n");
    prom_print_hex(_DAT_bfb20018,8);
    prom_puts(&_LC2);
    prom_puts("%DQ input dly:\n");
    puVar5 = (undefined4 *)&DAT_bfb20210;
    do {
      prom_print_hex(*puVar5,8);
      prom_puts(&_LC2);
      puVar5 = puVar5 + 1;
    } while (puVar5 != (undefined4 *)0xbfb20220);
    prom_puts("%dle: ");
    prom_print_dec(_opt_dle_value);
    prom_puts(&_LC2);
    prom_puts("%DRAMC calibration fail\n\r");
    _DAT_bfb00040 = _DAT_bfb00040 | 0x80000000;
    uVar7 = 1;
  }
  if (PKG_type == 0) {
    if ((_DAT_bfbf8214 & 0x40) == 0) {
      iVar3 = _DAT_bfbf8214 << 6;
    }
    else {
      iVar3 = _DAT_bfbf8214 << 5;
    }
    if (iVar3 < 0) {
      _DAT_bfb20004 = _DAT_bfb20004 & 0xfffffcff | 0x100;
      goto LAB_000107e8;
    }
  }
  check_column_bank();
LAB_000107e8:
  prom_puts("Calculate size.\n");
  uVar6 = 0x2000000;
  while( true ) {
    piVar2 = (int *)(&DAT_c0080000 + uVar6);
    _DAT_c0080000 = 0x12345678;
    pause_polling(2);
    *piVar2 = -0x789abcdf;
    pause_polling(2);
    iVar4 = *piVar2;
    iVar3 = _DAT_c0080000;
    while (_DAT_c0080000 = iVar3, iVar4 != -0x789abcdf) {
      *piVar2 = -0x789abcdf;
      iVar3 = _DAT_c0080000;
      iVar4 = *piVar2;
    }
    if (iVar3 != 0x12345678) break;
    uVar6 = uVar6 << 1;
    bVar1 = false;
    if (0x3fffffff < uVar6) {
LAB_000108c0:
      if (bVar1) {
        prom_printf_s();
        prom_print_dec(uVar6 >> 0x14);
        prom_printf_s();
        dram_size = uVar6 >> 0x14;
      }
      else {
        prom_printf_s();
        dram_size = uVar6 >> 0x14;
      }
LAB_00010900:
      _DAT_bfb00284 = _DAT_bfb00284 & 0xfffff000 | dram_size;
      set_TRFC();
      if ((_DAT_bfb20618 & 0xff) == 0xaa) {
        if ((_DAT_bfb2060c & 0xfff) == 0x687) {
          prom_puts("ddr-1066\n");
        }
        else if ((_DAT_bfb2060c & 0xfff) == 0x8a9) {
          prom_puts("ddr-1333\n");
        }
      }
      else if ((_DAT_bfb20618 & 0xff) == 0x8a) {
        prom_puts("ddr-800\n");
      }
      else if ((_DAT_bfb20618 & 0xff) == 0xca) {
        prom_puts("ddr-1200\n");
      }
      else if ((_DAT_bfb20618 & 0xff) == 0x4a) {
        prom_puts("ddr-400\n");
      }
      _DAT_bfb201c0 = _DAT_bfb201c0 | 0x80000000;
      _DAT_bfb201ec = _DAT_bfb201ec | 0x710;
      prom_puts("\r\n7516DRAMC V1.0 (");
      prom_print_dec(uVar7);
      prom_puts(&_LC24);
      return 0;
    }
  }
  bVar1 = true;
  if (iVar3 != -0x789abcdf) {
    prom_printf_s();
    prom_printf_s();
    prom_print_hex(&DAT_c0080000,8);
    prom_printf_s();
    prom_print_hex(iVar3,8);
    prom_printf_s();
    dram_size = 0;
    goto LAB_00010900;
  }
  goto LAB_000108c0;
}


