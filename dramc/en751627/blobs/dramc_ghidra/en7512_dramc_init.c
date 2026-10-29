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



undefined DAT_bfb00284;
undefined DAT_bfb00040;
undefined DAT_bfb200fc;
undefined DAT_bfb200b8;
undefined $LC3;
undefined DAT_bfb200bc;
undefined DAT_bfb2007c;
undefined DAT_bfb0005c;
undefined DAT_bfb20640;
undefined DAT_bfb20604;
undefined DAT_bfb20600;
undefined PKG_type;
undefined DAT_bfb20618;
undefined DAT_bfb20624;
undefined DAT_bfb20630;
undefined DAT_bfb20648;
undefined DAT_bfb20614;
undefined DAT_bfb20608;
undefined DAT_bfb2060c;
undefined DAT_bfb20048;
undefined DAT_bfb200d8;
undefined DAT_bfb200e4;
undefined DAT_bfb2008c;
undefined DAT_bfb20090;
undefined DAT_bfb20094;
undefined DAT_bfb200dc;
undefined DAT_bfb200e0;
undefined DAT_bfb200f0;
undefined DAT_bfb200f4;
undefined DAT_bfb20168;
undefined DAT_bfb20130;
undefined DAT_bfb20004;
undefined DAT_bfb20124;
undefined DAT_bfb201c0;
undefined DAT_bfb20028;
undefined DAT_bfb201e0;
undefined DAT_bfb20158;
undefined DAT_bfb20110;
undefined DAT_bfb20088;
undefined DAT_bfb201e4;
undefined DAT_bfb203b8;
undefined DAT_bfb201dc;
undefined DAT_bfb2000c;
undefined DAT_bfb20000;
undefined DAT_bfb20044;
undefined DAT_bfb201e8;
undefined DAT_bfb20008;
undefined DAT_bfb20010;
undefined DAT_bfb200f8;
undefined DAT_bfb201d8;
undefined DAT_bfb20018;
undefined DAT_bfb00074;
undefined DAT_bfa20174;
undefined DAT_bfa2ff28;

// WARNING: Globals starting with '_' overlap smaller symbols at the same address

undefined4 en7512_dramc_init(void)

{
  int *piVar1;
  int iVar2;
  int iVar3;
  
  if (_PKG_type == 3) {
    prom_puts("QFP IC\n");
  }
  else {
    prom_puts("BGA IC\n");
  }
  if (_DAT_bfb00284 << 9 < 0) {
    piVar1 = (int *)&DAT_bfa2ff28;
  }
  else {
    piVar1 = (int *)&DAT_bfa20174;
  }
  iVar2 = -((*piVar1 << 10) >> 0x1f);
  if (_DAT_bfb00284 << 9 < 0) {
    piVar1 = (int *)&DAT_bfa2ff28;
  }
  else {
    piVar1 = (int *)&DAT_bfa20174;
  }
  iVar3 = *piVar1;
  prom_puts("Xtal:");
  prom_print_dec(iVar2);
  prom_puts(&_LC3);
  prom_puts("DDR3 init.\n");
  _DAT_bfb00040 = 0;
  pause_polling(200);
  if (_PKG_type == 3) {
    _DAT_bfb200fc = 0x7110000;
    _DAT_bfb200b8 = 0xcc00cc00;
    _DAT_bfb200bc = 0xcc00cc00;
    _DAT_bfb2007c = 0xb48711ed;
  }
  else {
    _DAT_bfb200fc = 0x7010000;
    _DAT_bfb200b8 = 0xaa33aa33;
    _DAT_bfb200bc = 0xaa33aa33;
    _DAT_bfb2007c = 0xc38722ed;
  }
  if ((_DAT_bfb0005c & 0xffff) == 1) {
    if (_PKG_type == 3) {
      _DAT_bfb20640 = _DAT_bfb20640 & 0xfffffdcf;
    }
    else {
      _DAT_bfb20640 = _DAT_bfb20640 & 0xfffffdc8;
    }
    _DAT_bfb20640 = _DAT_bfb20640 | 0x220;
    _DAT_bfb20604 = _DAT_bfb20604 & 0xffffffe1 | 0x12;
    if (iVar2 == 0) {
      _DAT_bfb20600 = _DAT_bfb20600 & 0xfffc0808 | 0x181;
    }
    else {
      _DAT_bfb20600 = _DAT_bfb20600 & 0xfffc0808 | 0x131;
    }
    pause_polling(1);
    _DAT_bfb20600 = _DAT_bfb20600 | 0x2000000;
    pause_polling(0x14);
    _DAT_bfb20604 = _DAT_bfb20604 | 0x100;
    if (_PKG_type == 3) {
      _DAT_bfb20618 = _DAT_bfb20618 & 0xf3cf0000 | 0x88a;
      _DAT_bfb20624 = _DAT_bfb20624 & 0xf3cf0000 | 0x88a;
      _DAT_bfb20630 = _DAT_bfb20630 & 0xf3cf0000 | 0x88a;
    }
    else {
      _DAT_bfb20618 = _DAT_bfb20618 & 0xf3cf0000 | 0x8ca;
      _DAT_bfb20624 = _DAT_bfb20624 & 0xf3cf0000 | 0x8ca;
      _DAT_bfb20630 = _DAT_bfb20630 & 0xf3cf0000 | 0x8ca;
    }
    _DAT_bfb20648 = 0xc0004ab2;
    pause_polling(1);
    _DAT_bfb20614 = _DAT_bfb20614 & 0xf01fffff | 0x2800000;
  }
  else if (_PKG_type == 3) {
    _DAT_bfb20640 = _DAT_bfb20640 & 0xfffffdcf | 0x220;
    _DAT_bfb20604 = _DAT_bfb20604 & 0xffffffe1 | 0x12;
    _DAT_bfb20600 = _DAT_bfb20600 & 0xfbfc0808 | 0x4000135;
    _DAT_bfb20608 = _DAT_bfb20608 & 0x1ffff | 0x2b020000;
    _DAT_bfb2060c = _DAT_bfb2060c & 0xffff0000 | 0x687;
    pause_polling(1);
    _DAT_bfb20600 = _DAT_bfb20600 | 0x2000000;
    pause_polling(0x14);
    _DAT_bfb2060c = _DAT_bfb2060c | 0x80000;
    pause_polling(1);
    _DAT_bfb2060c = _DAT_bfb2060c | 0x10000;
    pause_polling(0x14);
    _DAT_bfb20604 = _DAT_bfb20604 | 0x100;
    _DAT_bfb20618 = _DAT_bfb20618 & 0xf3cf0000 | 0x8aa;
    _DAT_bfb20624 = _DAT_bfb20624 & 0xf3cf0000 | 0x8aa;
    _DAT_bfb20630 = _DAT_bfb20630 & 0xf3cf0000 | 0x8aa;
    _DAT_bfb20648 = 0xc0004ab2;
    pause_polling(1);
    _DAT_bfb20614 = _DAT_bfb20614 & 0xf01fffff | 0x4000000;
  }
  else {
    _DAT_bfb20640 = _DAT_bfb20640 & 0xfffffdc8 | 0x220;
    _DAT_bfb20604 = _DAT_bfb20604 & 0xffffffe1 | 0x12;
    _DAT_bfb20600 = _DAT_bfb20600 & 0xfbfc0808 | 0x4000135;
    if (iVar2 == 0) {
      _DAT_bfb20608 = _DAT_bfb20608 & 0x1ffff | 0x3d700000;
      _DAT_bfb2060c = _DAT_bfb2060c & 0xffff0000 | 0xb54;
    }
    else {
      _DAT_bfb20608 = _DAT_bfb20608 & 0x1ffff | 0x7df20000;
      _DAT_bfb2060c = _DAT_bfb2060c & 0xffff0000 | 0x8a9;
    }
    pause_polling(1);
    _DAT_bfb20600 = _DAT_bfb20600 | 0x2000000;
    pause_polling(0x14);
    _DAT_bfb2060c = _DAT_bfb2060c | 0x80000;
    pause_polling(1);
    _DAT_bfb2060c = _DAT_bfb2060c | 0x10000;
    pause_polling(0x14);
    _DAT_bfb20604 = _DAT_bfb20604 | 0x100;
    _DAT_bfb20618 = _DAT_bfb20618 & 0xf3cf0000 | 0x8aa;
    _DAT_bfb20624 = _DAT_bfb20624 & 0xf3cf0000 | 0x8aa;
    _DAT_bfb20630 = _DAT_bfb20630 & 0xf3cf0000 | 0x8aa;
    _DAT_bfb20648 = 0xc0004ab2;
    pause_polling(1);
    _DAT_bfb20614 = _DAT_bfb20614 & 0xf01fffff | 0x4000000;
  }
  _DAT_bfb20618 = _DAT_bfb20618 | 0x2000000;
  _DAT_bfb20624 = _DAT_bfb20624 | 0x2000000;
  _DAT_bfb20630 = _DAT_bfb20630 | 0x2000000;
  pause_polling(0x14);
  _DAT_bfb20640 = _DAT_bfb20640 | 0x10;
  pause_polling(1);
  _DAT_bfb20048 = 0x1e00d10d;
  if (_PKG_type == 3) {
    _DAT_bfb200d8 = 0xc0100900;
  }
  else if (iVar3 << 0x1b < 0) {
    _DAT_bfb200d8 = 0x40100900;
  }
  else {
    _DAT_bfb200d8 = 0x80100900;
  }
  pause_polling(200);
  _DAT_bfb200e4 = 0xa3;
  pause_polling(500);
  _DAT_bfb2008c = 1;
  _DAT_bfb20090 = 0;
  _DAT_bfb20094 = 0x10101010;
  _DAT_bfb200dc = 0x83080080;
  _DAT_bfb200e0 = 0x12080080;
  if (_PKG_type == 3) {
    _DAT_bfb200f0 = 0x80000000;
    _DAT_bfb200f4 = 0x1000000;
  }
  else {
    _DAT_bfb200f0 = 0;
    _DAT_bfb200f4 = 0x11000000;
  }
  _DAT_bfb20168 = 0x80;
  _DAT_bfb20130 = 0x30000000;
  if (_PKG_type == 3) {
    _DAT_bfb200d8 = 0xc0300900;
  }
  else if (iVar3 << 0x1b < 0) {
    _DAT_bfb200d8 = 0x40300900;
  }
  else {
    _DAT_bfb200d8 = 0x80300900;
  }
  pause_polling(1);
  _DAT_bfb20124 = 0x80000011;
  _DAT_bfb20094 = 0x10101010;
  _DAT_bfb201c0 = 0xc8b8;
  if (_PKG_type == 3) {
    _DAT_bfb2007c = 0xb48711e5;
  }
  else {
    _DAT_bfb2007c = 0xc38722e5;
  }
  _DAT_bfb20028 = 0xf1200f01;
  _DAT_bfb201e0 = 0x88000000;
  _DAT_bfb20158 = 0;
  _DAT_bfb20110 = 0x111190;
  if (_PKG_type == 3) {
    _DAT_bfb20004 = 0xf0740642;
  }
  else {
    _DAT_bfb20004 = 0xf0740682;
  }
  _DAT_bfb200e4 = 0xa7;
  pause_polling(1);
  if (_PKG_type == 3) {
    _DAT_bfb20088 = 0x4208;
  }
  else {
    _DAT_bfb20088 = 0x4210;
  }
  _DAT_bfb201e4 = 1;
  iVar2 = 0xffff;
  do {
    iVar2 = iVar2 + -1;
    if ((_DAT_bfb203b8 & 1) != 0) {
      pause_polling(1);
      _DAT_bfb20088 = 0x6000;
      _DAT_bfb201e4 = 1;
      iVar2 = 0xffff;
      goto LAB_0001092c;
    }
  } while (iVar2 != 0);
  goto LAB_00010ca0;
  while (iVar2 != 0) {
LAB_0001092c:
    iVar2 = iVar2 + -1;
    if ((_DAT_bfb203b8 & 1) != 0) {
      pause_polling(1);
      if (_PKG_type == 3) {
        _DAT_bfb20088 = 0x2006;
      }
      else {
        _DAT_bfb20088 = 0x2004;
      }
      _DAT_bfb201e4 = 1;
      iVar2 = 0xffff;
      goto LAB_0001099c;
    }
  }
  goto LAB_00010ca0;
  while (iVar2 != 0) {
LAB_0001099c:
    iVar2 = iVar2 + -1;
    if ((_DAT_bfb203b8 & 1) != 0) {
      pause_polling(1);
      if (_PKG_type == 3) {
        _DAT_bfb20088 = 0x1931;
      }
      else {
        _DAT_bfb20088 = 0x1b51;
      }
      _DAT_bfb201e4 = 1;
      iVar2 = 0xffff;
      goto LAB_00010a0c;
    }
  }
  goto LAB_00010ca0;
  while (iVar2 != 0) {
LAB_00010a0c:
    iVar2 = iVar2 + -1;
    if ((_DAT_bfb203b8 & 1) != 0) {
      pause_polling(1);
      if (_PKG_type == 0) goto LAB_00010a80;
      _DAT_bfb20088 = 0x400;
      _DAT_bfb201e4 = 0x10;
      iVar2 = 0xffff;
      goto LAB_00010a60;
    }
  }
  goto LAB_00010ca0;
  while (iVar2 != 0) {
LAB_00010a60:
    iVar2 = iVar2 + -1;
    if ((_DAT_bfb203b8 & 0x10) != 0) {
      pause_polling(6);
LAB_00010a80:
      _DAT_bfb201e4 = 0x1100;
      pause_polling(1);
      _DAT_bfb200e4 = 0x22a7;
      _DAT_bfb201e0 = 0x88000000;
      _DAT_bfb20088 = 0xffff;
      _DAT_bfb201e4 = 0x20;
      iVar2 = 0xffff;
      do {
        iVar2 = iVar2 + -1;
        if ((_DAT_bfb203b8 & 0x20) != 0) {
          pause_polling(1);
          _DAT_bfb201e4 = 0;
          _DAT_bfb201dc = 0x104d2842;
          if (_PKG_type == 3) {
            _DAT_bfb20004 = 0xf0740642;
            _DAT_bfb2007c = 0xb48711ed;
          }
          else {
            _DAT_bfb20004 = 0xf0740682;
            _DAT_bfb2007c = 0xc38722ed;
          }
          pause_polling(1);
          _DAT_bfb2000c = 0;
          if (_PKG_type == 3) {
            _DAT_bfb20000 = 0x33684552;
          }
          else {
            _DAT_bfb20000 = 0x447a4694;
          }
          pause_polling(1);
          if (_PKG_type == 3) {
            _DAT_bfb20044 = 0xa8830481;
            _DAT_bfb201e8 = 0x650;
            _DAT_bfb20008 = 0xf5f3f;
          }
          else {
            _DAT_bfb20044 = 0xa88a0481;
            _DAT_bfb201e8 = 0x660;
            _DAT_bfb20008 = 0xf5f50;
          }
          _DAT_bfb20010 = 0;
          _DAT_bfb200f8 = 0xedcb000f;
          _DAT_bfb201d8 = 0xc80008;
          _DAT_bfb20018 = 0x31313131;
          _DAT_bfb00074 = 0x105;
          pause_polling(2);
          prom_puts("DRAMC init done. \n");
          return 0;
        }
      } while (iVar2 != 0);
      break;
    }
  }
LAB_00010ca0:
  prom_puts("DRAMC Init Fail!\r\n");
  return 0xffffffff;
}


