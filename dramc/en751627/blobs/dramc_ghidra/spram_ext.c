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



undefined DAT_bfb40004;
undefined DAT_bfbf0100;
uint null;
undefined DAT_bfb00064;
undefined DAT_bfa10114;
undefined DAT_bfb00284;
undefined $LC2;
undefined DAT_80020028;
undefined DAT_9fa40000;
undefined DAT_bfb00090;
undefined DAT_bfb00040;

// WARNING: Globals starting with '_' overlap smaller symbols at the same address

void disable_all_hw_int(void)

{
  setCopReg(0,Status,Status & 0xffff00ff);
  _DAT_bfb40004 = 0;
  _DAT_bfbf0100 = 0;
  return;
}



void init_autobench(void)

{
  return;
}



void read_efuse_macro(undefined4 *param_1)

{
  *param_1 = 8;
  do {
  } while ((param_1[3] & 4) == 0);
  *param_1 = 0xd;
  do {
  } while ((param_1[3] & 2) == 0);
  *param_1 = 0;
  return;
}



void tlb_1g_config(void)

{
  int iVar1;
  int iVar2;
  
  local_flush_tlb_all();
  iVar2 = 0;
  iVar1 = -0x40000000;
  do {
    update_tlb_wired(iVar1,iVar1 + -0x40000000,iVar2,0x1a,0x7ffe000);
    iVar1 = iVar1 + 0x8000000;
    iVar2 = iVar2 + 1;
  } while (iVar1 != 0);
  return;
}



// WARNING: Globals starting with '_' overlap smaller symbols at the same address

void load_bootext(void)

{
  int iVar1;
  
  prom_puts("done\r\n");
  do {
    do {
      iVar1 = serial_tstc();
    } while (iVar1 == 0);
    iVar1 = serial_inc();
    if (iVar1 == 0x78) {
      iVar1 = XModemReceive(0x80020000,0);
      prom_puts("\nreceived len=0x");
      prom_print_hex(iVar1,8);
      prom_puts(&_LC2);
      if (iVar1 < 1) {
        prom_puts("Received error.\n");
      }
      else {
        if (((((((_DAT_bfb00064 & 0xffff0000) == 0x90000) ||
               ((_DAT_bfb00064 & 0xffff0000) == 0xb0000)) ||
              ((_DAT_bfb00064 & 0xffff0000) == 0xa0000)) ||
             (((_DAT_bfb00064 & 0xffff0000) == 0x70000 || ((_DAT_bfb00064 & 0xffff0000) == 0x80000))
             )) || (((_DAT_bfb00064 & 0xffff0000) == 0x40000 ||
                    ((_DAT_bfb00064 & 0xffff0000) == 0x50000)))) &&
           (((_DAT_bfa10114 & 2) != 0 && (_DAT_bfb00284 << 8 < 0)))) {
          _DAT_9fa40000 = _DAT_80020028;
        }
        flash_init(0);
        iVar1 = FlashBlockDFU(0,0x80020000,iVar1);
        if (iVar1 == 0) {
          prom_puts("System Reboot...\n");
          pause_polling(1000000);
          disable_all_hw_int();
          _DAT_bfb00090 = _DAT_bfb00090 & 0xffdfffff | 0x100000;
          _DAT_bfb00040 = 0x80000000;
        }
      }
    }
  } while( true );
}



undefined4 spram_preprocess(void)

{
  uart_init();
  init_system(1);
  timerSet(1,10000,1,1,0);
  read_efuse_macro((undefined4 *)0xbfbf8200);
  read_efuse_macro((undefined4 *)0xbfaa0000);
  tlb_1g_config();
  return 0;
}



// WARNING: Control flow encountered bad instruction data

void spram_postprocess(void)

{
  int extraout_at;
  undefined1 uVar1;
  
  local_flush_tlb_all();
  uVar1 = 0x50;
  load_bootext();
  *(undefined1 *)(extraout_at + 0xfc) = uVar1;
                    // WARNING: Bad instruction - Truncating control flow here
  halt_baddata();
}


