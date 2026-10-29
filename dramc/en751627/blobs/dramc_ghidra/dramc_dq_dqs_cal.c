typedef unsigned char   undefined;

typedef unsigned char    byte;
typedef unsigned int    dword;
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



undefined DAT_bfb20018;
undefined DAT_bfb20210;
undefined DAT_bfb20214;
undefined DAT_bfb20218;
undefined DAT_bfb2021c;
undefined DAT_bfb2003c;
undefined DAT_bfb20040;
undefined DAT_bfb20008;
undefined DAT_bfb203fc;
undefined DAT_bfb20370;
undefined DAT_bfb20010;
undefined DAT_bfb20200;
undefined DAT_bfb20204;
undefined DAT_bfb20208;
undefined DAT_bfb2020c;
undefined DAT_bfb20014;
undefined4 opt_tx_dqm;
undefined8 opt_tx_dq;
undefined4 opt_tx_dqs;

// WARNING: Globals starting with '_' overlap smaller symbols at the same address

undefined4 do_sw_rx_dq_dqs_calib(void)

{
  int *piVar1;
  int iVar2;
  char *pcVar3;
  byte *pbVar4;
  int *piVar5;
  uint uVar6;
  byte bVar7;
  uint uVar8;
  byte *pbVar9;
  int *piVar10;
  int iVar11;
  char cVar12;
  char *pcVar13;
  byte local_e0 [4];
  undefined4 local_dc;
  int local_cc [16];
  char local_8c [3];
  char local_89;
  byte local_88 [92];
  char acStack_2c [3];
  char acStack_29 [5];
  
  _DAT_bfb20018 = 0;
  _DAT_bfb20210 = 0;
  _DAT_bfb20214 = 0;
  _DAT_bfb20218 = 0;
  _DAT_bfb2021c = 0;
  piVar10 = &local_dc;
  piVar1 = piVar10;
  piVar5 = local_cc;
  do {
    *(char *)piVar1 = '\0';
    *piVar5 = 0;
    piVar1 = (int *)((int)piVar1 + 1);
    piVar5 = piVar5 + 1;
  } while (piVar1 != local_cc);
  iVar11 = 1;
  do {
    _DAT_bfb2003c = 0x55000800;
    _DAT_bfb20040 = 0xaa000100;
    iVar2 = 0x7fffffff;
    do {
      iVar2 = iVar2 + -1;
      if ((_DAT_bfb203fc & 0x400) != 0) break;
    } while (iVar2 != 0);
    _DAT_bfb20008 = _DAT_bfb20008 & 0x7fffffff | 0x40000000;
    iVar2 = 0x7fffffff;
    do {
      iVar2 = iVar2 + -1;
      if ((_DAT_bfb203fc & 0x400) != 0) break;
    } while (iVar2 != 0);
    pause_polling(400);
    _DAT_bfb20008 = _DAT_bfb20008 & 0xbfffffff;
    if ((_DAT_bfb20370 == 0xffff) || (iVar11 == 0x10)) break;
    piVar1 = local_cc;
    uVar6 = 0;
    do {
      if (((1 << (uVar6 & 0x1f) & _DAT_bfb20370) == 0) && (*piVar1 == 0)) {
        *(char *)((int)&local_dc + uVar6) = (char)iVar11;
      }
      else {
        *piVar1 = 1;
      }
      uVar6 = uVar6 + 1;
      piVar1 = piVar1 + 1;
    } while (uVar6 != 0x10);
    piVar5 = (int *)&DAT_bfb20210;
    piVar1 = piVar10;
    do {
      *piVar5 = (int)*(char *)piVar1 + *(char *)((int)piVar1 + 1) * 0x100 +
                *(char *)((int)piVar1 + 2) * 0x10000 + (char)*piVar1 * 0x1000000;
      piVar5 = piVar5 + 1;
      piVar1 = piVar1 + 1;
    } while (piVar5 != (int *)0xbfb20220);
    iVar11 = iVar11 + 1;
  } while (iVar11 != 0x11);
  pcVar13 = local_8c;
  pcVar3 = pcVar13;
  do {
    *pcVar3 = -1;
    pcVar3[1] = -1;
    pcVar3[2] = -1;
    pcVar3[3] = -1;
    pcVar3[4] = '\0';
    pcVar3[5] = *(char *)piVar10;
    pcVar3 = pcVar3 + 6;
    piVar10 = (int *)((int)piVar10 + 1);
  } while (pcVar3 != acStack_2c);
  cVar12 = -1;
  iVar11 = 0;
  do {
    _DAT_bfb20018 = iVar11 * 0x1010101;
    _DAT_bfb20040 = _DAT_bfb20040 & 0xff000000 | 0x100;
    iVar2 = 0x7fffffff;
    do {
      iVar2 = iVar2 + -1;
      if ((_DAT_bfb203fc & 0x400) != 0) break;
    } while (iVar2 != 0);
    _DAT_bfb20008 = _DAT_bfb20008 & 0x7fffffff | 0x40000000;
    iVar2 = 0x7fffffff;
    do {
      iVar2 = iVar2 + -1;
      if ((_DAT_bfb203fc & 0x400) != 0) break;
    } while (iVar2 != 0);
    pause_polling(400);
    _DAT_bfb20008 = _DAT_bfb20008 & 0xbfffffff;
    pcVar3 = local_8c;
    uVar6 = 0;
    do {
      if (((*pcVar3 != -1) ||
          (((_DAT_bfb20370 >> (uVar6 & 0x1f) & 1) == 0 &&
           (*pcVar3 = (char)iVar11, (char)iVar11 != -1)))) &&
         (uVar8 = _DAT_bfb20370 >> (uVar6 & 0x1f), pcVar3[1] == -1)) {
        if ((uVar8 & 1) == 0) {
          if (iVar11 != 0x5f) goto LAB_000103a4;
LAB_00010358:
          if ((uVar8 & 1) != 0) goto LAB_00010368;
          pcVar3[1] = '_';
        }
        else {
          if (iVar11 == 0x5f) goto LAB_00010358;
LAB_00010368:
          pcVar3[1] = cVar12;
        }
        if ((int)pcVar3[3] - (int)pcVar3[2] < (int)pcVar3[1] - (int)*pcVar3) {
          pcVar3[3] = pcVar3[1];
          pcVar3[2] = *pcVar3;
        }
        pcVar3[1] = -1;
        *pcVar3 = -1;
      }
LAB_000103a4:
      uVar6 = uVar6 + 1;
      pcVar3 = pcVar3 + 6;
    } while (uVar6 != 0x10);
    iVar11 = iVar11 + 1;
    cVar12 = cVar12 + '\x01';
    if (iVar11 == 0x60) {
      pcVar3 = &local_89;
      do {
        if ((*pcVar3 != -1) && (pcVar3[-1] != -1)) {
          pcVar3[1] = (char)(((int)*pcVar3 + (int)pcVar3[-1]) / 2);
        }
        pcVar3 = pcVar3 + 6;
      } while (pcVar3 != acStack_29);
      pbVar9 = local_e0;
      do {
        iVar11 = 0;
        bVar7 = 0;
        do {
          iVar2 = iVar11 + 4;
          iVar11 = iVar11 + 6;
          if (bVar7 < (byte)pcVar13[iVar2]) {
            bVar7 = pcVar13[iVar2];
          }
        } while (iVar11 != 0x30);
        *pbVar9 = bVar7;
        pbVar9 = pbVar9 + 1;
        pcVar13 = pcVar13 + 0x30;
      } while (pbVar9 != local_e0 + 2);
      _DAT_bfb20018 = (char)local_e0[1] * 0x100 + (int)(char)local_e0[0];
      pbVar9 = local_88 + 1;
      uVar6 = 0;
      do {
        bVar7 = local_e0[uVar6 >> 3];
        pbVar4 = local_88 + uVar6 * 6;
        iVar11 = 4;
        do {
          uVar8 = ((int)(char)bVar7 - (uint)*pbVar4) + (uint)pbVar4[1];
          if (0xf < uVar8) {
            uVar8 = 0xf;
          }
          pbVar4[1] = (byte)uVar8;
          iVar11 = iVar11 + -1;
          pbVar4 = pbVar4 + 6;
        } while (iVar11 != 0);
        *(uint *)(&DAT_bfb20210 + uVar6) =
             (uint)pbVar9[6] * 0x100 + (uint)pbVar9[0xc] * 0x10000 + (uint)*pbVar9 +
             (uint)pbVar9[0x12] * 0x1000000;
        uVar6 = uVar6 + 4;
        pbVar9 = pbVar9 + 0x18;
      } while (uVar6 != 0x10);
      return 0;
    }
  } while( true );
}



// WARNING: Globals starting with '_' overlap smaller symbols at the same address

undefined4 do_sw_tx_dq_dqs_calib(void)

{
  uint *puVar1;
  int iVar2;
  char *pcVar3;
  byte *pbVar4;
  uint uVar5;
  char cVar7;
  uint uVar6;
  char cVar10;
  uint uVar9;
  uint uVar11;
  uint uVar12;
  uint *puVar13;
  uint uVar14;
  int iVar15;
  undefined4 local_d8;
  char local_d4 [12];
  byte local_c8;
  byte local_be;
  byte local_b4;
  byte local_aa;
  byte local_a0;
  byte local_96;
  byte local_8c;
  byte local_82;
  byte local_78;
  byte local_6e;
  byte local_64;
  byte local_5a;
  byte local_50;
  byte local_46;
  byte local_3c;
  uint local_38 [5];
  byte bVar8;
  
  puVar1 = &local_d8;
  do {
    *(undefined1 *)puVar1 = 0;
    *(undefined1 *)((int)puVar1 + 1) = 0xff;
    *(undefined1 *)((int)puVar1 + 3) = 0;
    *(undefined1 *)(puVar1 + 1) = 0xff;
    *(undefined1 *)((int)puVar1 + 6) = 0;
    *(undefined1 *)((int)puVar1 + 7) = 0;
    *(undefined1 *)((int)puVar1 + 2) = 0;
    *(undefined1 *)((int)puVar1 + 5) = 0;
    puVar1 = (uint *)((int)puVar1 + 10);
  } while (puVar1 != local_38);
  _DAT_bfb20010 = 0;
  _DAT_bfb20200 = 0;
  _DAT_bfb20204 = 0;
  _DAT_bfb20208 = 0;
  _DAT_bfb2020c = 0;
  iVar15 = 0;
  uVar14 = 0xf;
  do {
    _DAT_bfb20014 = uVar14 << 4 | uVar14 << 8 | uVar14 | uVar14 << 0xc | _DAT_bfb20014 & 0xffff0000;
    _DAT_bfb2003c = 0x55000800;
    _DAT_bfb20040 = _DAT_bfb20040 & 0xffffff | 0xaa0003ff;
    iVar2 = 0x7fffffff;
    do {
      iVar2 = iVar2 + -1;
      if ((_DAT_bfb203fc & 0x400) != 0) break;
    } while (iVar2 != 0);
    _DAT_bfb20008 = _DAT_bfb20008 & 0x7fffffff | 0x40000000;
    iVar2 = 0x7fffffff;
    do {
      iVar2 = iVar2 + -1;
      if ((_DAT_bfb203fc & 0x400) != 0) break;
    } while (iVar2 != 0);
    pause_polling(400);
    _DAT_bfb20008 = _DAT_bfb20008 & 0xbfffffff;
    pcVar3 = local_d4;
    uVar5 = 0;
    do {
      if ((*pcVar3 == -1) && ((1 << (uVar5 & 0x1f) & _DAT_bfb20370) == 0)) {
        *pcVar3 = (char)uVar14;
        iVar15 = iVar15 + 1;
      }
      uVar5 = uVar5 + 1;
      pcVar3 = pcVar3 + 10;
    } while (uVar5 != 0x10);
  } while ((iVar15 != 0x10) && (uVar14 = uVar14 - 1, uVar14 != 0xffffffff));
  _DAT_bfb20014 = _DAT_bfb20014 & 0xffff0000;
  iVar15 = 0;
  uVar5 = 0xf;
  uVar14 = 0xf;
  do {
    _DAT_bfb20010 = (uVar5 & 0xf) << 4 | uVar14 << 8 | uVar14 | (uVar5 & 0xf) << 0xc;
    _DAT_bfb20200 = uVar14 << 0x10 | uVar14 << 0x14 | uVar5 << 0x1c | uVar14 << 0x18 | _DAT_bfb20010
    ;
    _DAT_bfb2003c = 0x55000800;
    _DAT_bfb20040 = _DAT_bfb20040 & 0xffffff | 0xaa0003ff;
    iVar2 = 0x7fffffff;
    do {
      iVar2 = iVar2 + -1;
      if ((_DAT_bfb203fc & 0x400) != 0) break;
    } while (iVar2 != 0);
    _DAT_bfb20008 = _DAT_bfb20008 & 0x7fffffff | 0x40000000;
    iVar2 = 0x7fffffff;
    do {
      iVar2 = iVar2 + -1;
      if ((_DAT_bfb203fc & 0x400) != 0) break;
    } while (iVar2 != 0);
    _DAT_bfb20204 = _DAT_bfb20200;
    _DAT_bfb20208 = _DAT_bfb20200;
    _DAT_bfb2020c = _DAT_bfb20200;
    pause_polling(400);
    _DAT_bfb20008 = _DAT_bfb20008 & 0xbfffffff;
    pcVar3 = (char *)((int)&local_d8 + 1);
    uVar14 = 0;
    do {
      if ((*pcVar3 == -1) && ((1 << (uVar14 & 0x1f) & _DAT_bfb20370) == 0)) {
        *pcVar3 = (char)uVar5;
        iVar15 = iVar15 + 1;
      }
      uVar14 = uVar14 + 1;
      pcVar3 = pcVar3 + 10;
    } while (uVar14 != 0x10);
    if (iVar15 == 0x10) break;
    uVar5 = uVar5 - 1;
    uVar14 = uVar5 & 0xf;
  } while (uVar5 != 0xffffffff);
  puVar13 = local_38;
  puVar1 = local_38 + 2;
  uVar14 = 0;
  do {
    uVar5 = uVar14 + 7;
    *puVar13 = 0;
    *puVar1 = 0;
    if (uVar14 <= uVar5) {
      pcVar3 = local_d4 + uVar14 * 10;
      uVar11 = uVar14;
      do {
        cVar10 = (*pcVar3 + '\x01') - pcVar3[-1];
        pcVar3[1] = cVar10;
        cVar7 = (pcVar3[-3] + '\x01') - pcVar3[-4];
        iVar15 = (int)cVar7;
        iVar2 = (int)cVar10;
        pcVar3[-2] = cVar7;
        if (iVar15 == iVar2) {
          pcVar3[3] = '\0';
          pcVar3[2] = '\0';
        }
        else if (iVar2 < iVar15) {
          pcVar3[3] = '\0';
          uVar9 = ((int)pcVar3[-2] - (int)pcVar3[1]) / 2 + (uint)(byte)pcVar3[2];
          pcVar3[2] = (char)uVar9;
          if (0xf < (uVar9 & 0xff)) {
            pcVar3[2] = '\x0f';
          }
        }
        else {
          uVar9 = (iVar2 - iVar15) / 2 + (uint)(byte)pcVar3[3];
          pcVar3[3] = (char)uVar9;
          if (0xf < (uVar9 & 0xff)) {
            pcVar3[3] = '\x0f';
          }
          pcVar3[2] = '\0';
          if (*puVar13 < (uint)(byte)pcVar3[3]) {
            *puVar13 = (uint)(byte)pcVar3[3];
          }
        }
        uVar11 = uVar11 + 1;
        pcVar3 = pcVar3 + 10;
      } while (uVar11 <= uVar5);
      uVar12 = *puVar13;
      uVar9 = *puVar1;
      pbVar4 = (byte *)(local_d4 + uVar14 * 10 + 3);
      uVar11 = uVar14;
      do {
        if (*pbVar4 < uVar12) {
          uVar6 = ((uVar12 & 0xff) + (uint)pbVar4[-1]) - (uint)*pbVar4;
          bVar8 = (byte)uVar6;
          if (0xf < (uVar6 & 0xff)) {
            bVar8 = 0xf;
          }
          pbVar4[-1] = bVar8;
        }
        uVar9 = uVar9 + pbVar4[-1];
        uVar11 = uVar11 + 1;
        pbVar4 = pbVar4 + 10;
      } while (uVar11 <= uVar5);
      *puVar1 = uVar9;
    }
    *puVar1 = *puVar1 >> 3;
    puVar13 = puVar13 + 1;
    uVar14 = uVar14 + 8;
    puVar1 = puVar1 + 1;
  } while (uVar14 != 0x10);
  _DAT_bfb20010 = (local_38[3] & 0xf) << 4 | local_38[2] & 0xf;
  opt_tx_dqm = _DAT_bfb20010;
  _DAT_bfb20200 =
       (uint)local_8c << 0x1c | (byte)local_d4[2] & 0xf | (local_c8 & 0xf) << 4 |
       (local_be & 0xf) << 8 | (local_b4 & 0xf) << 0xc | (local_aa & 0xf) << 0x10 |
       (local_a0 & 0xf) << 0x14 | (local_96 & 0xf) << 0x18;
  opt_tx_dq._0_4_ = _DAT_bfb20200;
  _DAT_bfb20204 =
       (uint)local_3c << 0x1c | local_82 & 0xf | (local_78 & 0xf) << 4 | (local_6e & 0xf) << 8 |
       (local_64 & 0xf) << 0xc | (local_5a & 0xf) << 0x10 | (local_50 & 0xf) << 0x14 |
       (local_46 & 0xf) << 0x18;
  opt_tx_dq._4_4_ = _DAT_bfb20204;
  _DAT_bfb20014 = local_38[1] << 4 | local_38[0];
  opt_tx_dqs = _DAT_bfb20014;
  return 0;
}


