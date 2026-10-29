typedef unsigned char   undefined;

typedef unsigned char    bool;
typedef unsigned char    byte;
typedef unsigned int    dword;
typedef unsigned int    uint;
typedef unsigned char    undefined1;
typedef unsigned int    undefined4;
typedef unsigned short    ushort;
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



undefined4 xmdmBufBase;
undefined4 xmodemOut;
undefined4 xmodemIn;
undefined DAT_bfb00284;
undefined DAT_bfbf010c;
undefined ReadByte;
undefined WriteByte;
undefined WriteBuffer;

undefined4 WriteBuffer(undefined1 *param_1,int param_2)

{
  undefined1 *puVar1;
  undefined1 *puVar2;
  
  puVar2 = param_1 + param_2;
  puVar1 = xmdmBufBase;
  if (0 < param_2) {
    do {
      *puVar1 = *param_1;
      param_1 = param_1 + 1;
      puVar1 = puVar1 + 1;
    } while (param_1 != puVar2);
    xmdmBufBase = xmdmBufBase + param_2;
  }
  return 0;
}



uint ReadByte(void)

{
  int iVar1;
  uint uVar2;
  
  iVar1 = serial_tstc();
  uVar2 = 0xffffffff;
  if (iVar1 != 0) {
    uVar2 = serial_inc();
    uVar2 = uVar2 & 0xff;
  }
  return uVar2;
}



void WriteByte(char param_1)

{
  serial_outc((int)param_1);
  return;
}



void xmodemInit(undefined4 param_1,undefined4 param_2)

{
  xmodemOut = param_1;
  xmodemIn = param_2;
  return;
}



undefined4 xmodemTransmit(void)

{
  return 0;
}



void crc_xmodem_update(ushort param_1,short param_2)

{
  ushort uVar1;
  int iVar2;
  
  uVar1 = param_1 ^ param_2 << 8;
  iVar2 = 8;
  do {
    if ((short)uVar1 < 0) {
      uVar1 = uVar1 << 1 ^ 0x1021;
    }
    else {
      uVar1 = uVar1 << 1;
    }
    iVar2 = iVar2 + -1;
  } while (iVar2 != 0);
  return;
}



bool xmodemCrcCheck(int param_1,byte *param_2,int param_3)

{
  byte bVar1;
  byte bVar2;
  bool bVar3;
  int iVar4;
  byte bVar5;
  
  if (param_1 == 0) {
    bVar5 = 0;
    if (0 < param_3) {
      iVar4 = 0;
      do {
        bVar5 = bVar5 + param_2[iVar4];
        iVar4 = iVar4 + 1;
      } while (iVar4 != param_3);
    }
    bVar3 = param_2[param_3] == bVar5;
  }
  else {
    bVar5 = (param_2 + param_3)[1];
    bVar1 = param_2[param_3];
    iVar4 = 0;
    for (; param_3 != 0; param_3 = param_3 + -1) {
      bVar2 = *param_2;
      param_2 = param_2 + 1;
      iVar4 = crc_xmodem_update((ushort)iVar4,(ushort)bVar2);
    }
    bVar3 = (uint)bVar5 + (uint)bVar1 * 0x100 == iVar4;
  }
  return bVar3;
}



// WARNING: Globals starting with '_' overlap smaller symbols at the same address

uint xmodemInTime(uint param_1)

{
  int iVar1;
  uint uVar2;
  uint uVar3;
  int iVar4;
  uint uVar5;
  uint uVar6;
  
  iVar1 = _DAT_bfbf010c;
  uVar5 = _DAT_bfb00284 & 0x3fffff;
  uVar2 = getTimeTick();
  uVar6 = 0;
  do {
    iVar4 = serial_tstc();
    if (iVar4 != 0) {
      uVar5 = serial_inc();
      return uVar5 & 0xff;
    }
    uVar3 = getTimeTick();
    if (uVar2 < uVar3) {
      uVar6 = (uVar2 + uVar6 + iVar1) - uVar3;
    }
    else {
      uVar6 = (uVar6 + uVar2) - uVar3;
    }
    uVar2 = uVar3;
  } while (uVar6 <= (uVar5 >> 0xd) * (param_1 & 0xffff) * 10000);
  return 0xffffffff;
}



void xmodemInFlush(void)

{
  uint uVar1;
  
  do {
    uVar1 = xmodemInTime(100);
  } while (-1 < (int)uVar1);
  return;
}



int xmodemReceive(undefined *param_1)

{
  undefined3 extraout_var;
  uint uVar1;
  bool bVar2;
  int iVar3;
  int iVar4;
  char cVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  byte local_43f [1031];
  int local_38;
  int local_34;
  int local_30;
  uint local_2c;
  
  local_30 = 0;
  iVar6 = 0;
  iVar3 = 0x43;
  iVar7 = 0x80;
  local_2c = 1;
LAB_00010300:
  cVar5 = ' ';
  do {
    (*xmodemOut)(iVar3);
    uVar1 = xmodemInTime(100);
    if ((int)uVar1 < 0) {
      cVar5 = cVar5 + -1;
    }
    else {
      if (uVar1 == 2) {
        iVar7 = 0x400;
      }
      else if ((int)uVar1 < 3) {
        if (uVar1 == 1) {
          iVar7 = 0x80;
        }
      }
      else {
        if (uVar1 == 4) {
          xmodemInFlush();
          (*xmodemOut)(6);
          return local_30;
        }
        if ((uVar1 == 0x18) && (uVar1 = xmodemInTime(100), uVar1 == 0x18)) {
          xmodemInFlush();
          (*xmodemOut)(6);
          return -1;
        }
      }
      if (iVar3 == 0x43) {
        iVar6 = 1;
      }
      iVar8 = iVar7 + iVar6 + 3;
      local_38 = iVar7;
      local_34 = iVar6;
      if (iVar7 + iVar6 != -3) {
        iVar4 = 0;
        do {
          uVar1 = xmodemInTime(100);
          if ((int)uVar1 < 0) {
            cVar5 = cVar5 + -1;
            xmodemInFlush();
            iVar3 = 0x15;
            if (iVar4 < iVar8) goto LAB_000104e8;
            break;
          }
          iVar3 = iVar4 + 1;
          local_43f[iVar4] = (byte)uVar1;
          iVar4 = iVar3;
        } while (iVar3 != iVar8);
      }
      uVar1 = (uint)local_43f[0];
      if ((uVar1 == (~(uint)local_43f[1] & 0xff)) &&
         (bVar2 = xmodemCrcCheck(local_34,local_43f + 2,local_38), CONCAT31(extraout_var,bVar2) != 0
         )) {
        if (local_2c == uVar1) break;
        iVar3 = 6;
        if (uVar1 != (local_2c - 1 & 0xff)) {
          xmodemInFlush();
          (*xmodemOut)(0x18);
          (*xmodemOut)(0x18);
          (*xmodemOut)(0x18);
          return -2;
        }
      }
      else {
        cVar5 = cVar5 + -1;
        xmodemInFlush();
        iVar3 = 0x15;
      }
    }
LAB_000104e8:
    if (cVar5 < '\x01') {
      xmodemInFlush();
      (*xmodemOut)(0x18);
      (*xmodemOut)(0x18);
      (*xmodemOut)(0x18);
      return -3;
    }
  } while( true );
  (*(code *)param_1)(local_43f + 2,local_38);
  local_30 = local_30 + iVar7;
  local_2c = local_2c + 1 & 0xff;
  iVar3 = 6;
  goto LAB_00010300;
}



void XModemReceive(undefined4 param_1)

{
  xmodemInit(WriteByte,ReadByte);
  xmdmBufBase = param_1;
  xmodemReceive(WriteBuffer);
  return;
}


