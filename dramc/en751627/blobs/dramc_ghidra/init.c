typedef unsigned char   undefined;

typedef unsigned char    byte;
typedef unsigned int    dword;
typedef unsigned int    uint;
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



undefined DAT_bfbf0014;
undefined DAT_bfbf0000;
undefined DAT_bfb00284;
undefined DAT_bfbf0008;
undefined DAT_bfbf0010;
undefined DAT_bfbf0024;
undefined DAT_bfbf0004;
undefined DAT_bfbf000c;
undefined DAT_bfb00064;
undefined DAT_bfbf002c;

// WARNING: Globals starting with '_' overlap smaller symbols at the same address

void serial_outc(char param_1)

{
  do {
  } while ((_DAT_bfbf0014 & 0x20) == 0);
  _DAT_bfbf0000 = (int)param_1;
  return;
}



// WARNING: Globals starting with '_' overlap smaller symbols at the same address

int serial_inc(void)

{
  do {
  } while ((_DAT_bfbf0014 & 1) == 0);
  return (int)(char)_DAT_bfbf0000;
}



// WARNING: Globals starting with '_' overlap smaller symbols at the same address

uint get_SYS_HCLK(void)

{
  return (_DAT_bfb00284 & 0x3fffff) >> 0xc;
}



// WARNING: Globals starting with '_' overlap smaller symbols at the same address

void uart_init(void)

{
  int iVar1;
  uint uVar2;
  
  _DAT_bfbf0008 = 0xf;
  _DAT_bfbf0010 = 0;
  _DAT_bfbf0024 = 0;
  _DAT_bfbf0004 = 1;
  _DAT_bfbf000c = 0x80;
  iVar1 = 0xea00;
  if (((((_DAT_bfb00064 & 0xffff0000) != 0x40000) && ((_DAT_bfb00064 & 0xffff0000) != 0x50000)) &&
      ((_DAT_bfb00064 & 0xffff0000) != 0x60000)) &&
     ((((_DAT_bfb00064 & 0xffff0000) != 0x70000 && ((_DAT_bfb00064 & 0xffff0000) != 0x80000)) &&
      (((_DAT_bfb00064 & 0xffff0000) != 0x90000 && ((_DAT_bfb00064 & 0xffff0000) != 0xb0000)))))) {
    uVar2 = 0xea000000;
    if ((_DAT_bfb00064 & 0xffff0000) == 0xa0000) goto LAB_0001016c;
    uVar2 = get_SYS_HCLK();
    if (uVar2 == 0) {
      trap(7);
    }
    iVar1 = 0x57c00 / uVar2 << 1;
  }
  uVar2 = iVar1 << 0x10;
LAB_0001016c:
  _DAT_bfbf002c = uVar2 | 65000;
  if ((((_DAT_bfb00064 & 0xffff0000) == 0x40000) || ((_DAT_bfb00064 & 0xffff0000) == 0x50000)) ||
     (((_DAT_bfb00064 & 0xffff0000) == 0x60000 ||
      (((((_DAT_bfb00064 & 0xffff0000) == 0x70000 || ((_DAT_bfb00064 & 0xffff0000) == 0x80000)) ||
        ((_DAT_bfb00064 & 0xffff0000) == 0x90000)) ||
       (((_DAT_bfb00064 & 0xffff0000) == 0xb0000 || ((_DAT_bfb00064 & 0xffff0000) == 0xa0000))))))))
  {
    _DAT_bfbf0000 = 1;
  }
  else {
    _DAT_bfbf0000 = 3;
  }
  _DAT_bfbf0004 = 0;
  _DAT_bfbf000c = 3;
  return;
}



// WARNING: Globals starting with '_' overlap smaller symbols at the same address

uint serial_tstc(void)

{
  return _DAT_bfbf0014 & 1;
}



void prom_puts(char *param_1)

{
  char cVar1;
  
  cVar1 = *param_1;
  while (cVar1 != '\0') {
    param_1 = param_1 + 1;
    if (cVar1 == '\n') {
      serial_outc('\r');
    }
    serial_outc(cVar1);
    cVar1 = *param_1;
  }
  return;
}



void prom_printf(char *param_1)

{
  char cVar1;
  
  cVar1 = *param_1;
  while (cVar1 != '\0') {
    param_1 = param_1 + 1;
    if (cVar1 == '\n') {
      serial_outc('\r');
    }
    serial_outc(cVar1);
    cVar1 = *param_1;
  }
  return;
}



void prom_print_dec(uint param_1)

{
  bool bVar1;
  uint uVar2;
  int iVar3;
  uint uVar4;
  uint uVar5;
  
  iVar3 = 10;
  uVar2 = 1000000000;
  bVar1 = true;
  do {
    uVar5 = param_1 / uVar2;
    param_1 = param_1 % uVar2;
    if (uVar2 == 0) {
      trap(7);
    }
    if ((uVar5 == 0) && (uVar2 != 1)) {
      uVar4 = (uint)((ulonglong)uVar2 * 0xcccccccd >> 0x20);
      if (!bVar1) goto LAB_000103d8;
    }
    else {
      bVar1 = false;
LAB_000103d8:
      serial_outc((char)uVar5 + '0');
      uVar4 = (uint)((ulonglong)uVar2 * 0xcccccccd >> 0x20);
    }
    iVar3 = iVar3 + -1;
    uVar2 = uVar4 >> 3;
    if (iVar3 == 0) {
      return;
    }
  } while( true );
}



void prom_print_hex(uint param_1,int param_2)

{
  char cVar1;
  uint uVar2;
  char cVar3;
  int iVar4;
  uint uVar5;
  
  iVar4 = param_2 + -1;
  if (-1 < iVar4) {
    uVar5 = iVar4 * 4;
    do {
      uVar2 = param_1 >> (uVar5 & 0x1f) & 0xf;
      cVar1 = (char)uVar2;
      cVar3 = cVar1 + '0';
      if (9 < uVar2) {
        cVar3 = cVar1 + 'W';
      }
      iVar4 = iVar4 + -1;
      serial_outc(cVar3);
      uVar5 = uVar5 - 4;
    } while (iVar4 != -1);
  }
  return;
}



void show_array(byte *param_1,uint param_2,char *param_3)

{
  uint uVar1;
  
  prom_puts("\n");
  prom_puts(param_3);
  prom_puts(":");
  uVar1 = 0;
  if (param_2 != 0) {
    do {
      if ((uVar1 & 7) == 0) {
        if (uVar1 != 0) {
          prom_puts("  ");
          goto LAB_0001052c;
        }
LAB_00010534:
        prom_puts("\nidx:0x");
        prom_print_hex(uVar1,4);
        prom_puts(", addr:0x");
        prom_print_hex((uint)param_1,8);
        prom_puts(": ");
      }
      else {
LAB_0001052c:
        if ((uVar1 & 0xf) == 0) goto LAB_00010534;
      }
      prom_print_hex((uint)*param_1,2);
      prom_puts(" ");
      uVar1 = uVar1 + 1;
      param_1 = param_1 + 1;
    } while (uVar1 != param_2);
  }
  prom_puts("\n\n");
  return;
}


