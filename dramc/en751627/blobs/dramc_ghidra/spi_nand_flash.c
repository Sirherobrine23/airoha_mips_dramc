typedef unsigned char   undefined;

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



undefined1[64] _current_flash_info_t;
undefined1 _plane_select_bit;
undefined4 dma_write_page;
undefined4 _current_cache_page_oob_mapping;
undefined4 _current_page_num;
undefined4 otp_status;
undefined1 _parallel_nand_mode;
undefined1 _die_id;
undefined DAT_bfb00284;
undefined DAT_bfb00064;
undefined DAT_bfa2011c;
undefined4 _spi_dma_mode;
undefined1 _ondie_ecc_flag;
undefined DAT_bfa2015c;
undefined $LC2;
undefined4 print_dot;
undefined DAT_bfa10114;
undefined4 _current_cache_page_oob;
undefined4 _current_cache_page_data;
undefined4 dma_read_page;
undefined4 _current_cache_page;
undefined ooblayout_spi_controller_ecc;
undefined4 nand_probe;
undefined DAT_9fa40000;
undefined4 tmp_dma_read_page;
undefined4 tmp_dma_write_page;
undefined spi_nand_probe;

undefined4 spi_nand_dma_program_load(ushort param_1,undefined4 param_2,int param_3)

{
  undefined4 uVar1;
  undefined4 uVar2;
  undefined4 local_58;
  undefined4 local_54;
  int local_50;
  int local_4c;
  byte local_43;
  int local_28;
  int local_24;
  
  if ((_current_flash_info_t._44_4_ & 1) != 0) {
    if (_plane_select_bit == '\0') {
      param_1 = param_1 & 0xefff;
    }
    else if (_plane_select_bit == '\x01') {
      param_1 = param_1 | 0x1000;
    }
  }
  uVar1 = 2;
  if (param_3 != 1) {
    uVar1 = 0;
  }
  uVar2 = 2;
  if (param_3 == 1) {
    uVar2 = 0x32;
  }
  SPI_CONTROLLER_Get_Configure(&local_58);
  local_54 = 0;
  local_58 = 2;
  SPI_CONTROLLER_Set_Configure(&local_58);
  SPI_NFI_Reset();
  SPI_NFI_Get_Configure(&local_50);
  SPI_NFI_Set_Configure(&local_50);
  SPI_ECC_Encode_Get_Configure(&local_28);
  SPI_ECC_Encode_Set_Configure(&local_28);
  if (local_4c == 1) {
    SPI_ECC_Encode_Get_Configure(&local_28);
    if (local_28 == 1) {
      local_24 = local_43 + 0x200;
      SPI_ECC_Encode_Set_Configure(&local_28);
      SPI_ECC_Encode_Disable();
      SPI_ECC_Decode_Disable();
      SPI_ECC_Encode_Enable();
    }
  }
  if (local_50 == 1) {
    SPI_NFI_Write_SPI_NAND_FDM(_current_cache_page_oob_mapping,param_2);
  }
  uVar1 = SPI_NFI_Write_SPI_NAND_page(uVar1,uVar2,param_1,dma_write_page & 0x1fffffff);
  SPI_CONTROLLER_Enable_Manual_Mode();
  return uVar1;
}



void spi_nand_protocol_set_otp(undefined4 param_1)

{
  _current_page_num = 0xffffffff;
  otp_status = param_1;
  return;
}



undefined4 spi_nand_protocol_get_feature(undefined1 param_1,undefined4 param_2)

{
  if (_parallel_nand_mode == '\0') {
    SPI_CONTROLLER_Chip_Select_Low();
    SPI_CONTROLLER_Write_One_Byte(0xf);
    SPI_CONTROLLER_Write_One_Byte_With_Cmd(0x11,param_1);
    SPI_CONTROLLER_Read_NByte(param_2,1,0);
    SPI_CONTROLLER_Chip_Select_High();
  }
  return 0;
}



int spi_nand_protocol_chk_status(uint param_1,uint param_2)

{
  int iVar1;
  int iVar2;
  byte local_18 [8];
  
  iVar2 = 0xf4241;
  while( true ) {
    iVar1 = spi_nand_protocol_get_feature(0xc0,local_18);
    iVar2 = iVar2 + -1;
    if (iVar1 != 0) {
      return iVar1;
    }
    if (iVar2 == 0) break;
    if ((param_1 & local_18[0]) == param_2) {
      return 0;
    }
  }
  return 10;
}



int spi_nand_protocol_die_select_1(undefined4 param_1)

{
  int iVar1;
  
  iVar1 = spi_nand_protocol_chk_status(1,0);
  if (iVar1 == 0) {
    SPI_CONTROLLER_Chip_Select_Low();
    SPI_CONTROLLER_Write_One_Byte(0xc2);
    SPI_CONTROLLER_Write_One_Byte(param_1);
    SPI_CONTROLLER_Chip_Select_High();
  }
  return iVar1;
}



int spi_nand_protocol_program_load(uint param_1,undefined4 param_2,undefined4 param_3,int param_4)

{
  int iVar1;
  
  iVar1 = spi_nand_protocol_chk_status(1,0);
  if (iVar1 == 0) {
    SPI_CONTROLLER_Chip_Select_Low();
    if (param_4 == 0) {
      SPI_CONTROLLER_Write_One_Byte(2);
    }
    else if (param_4 == 1) {
      SPI_CONTROLLER_Write_One_Byte(0x32);
    }
    if ((_current_flash_info_t._44_4_ & 1) == 0) {
      SPI_CONTROLLER_Write_One_Byte((param_1 & 0xffff) >> 8);
    }
    else {
      if (_plane_select_bit == '\0') {
        SPI_CONTROLLER_Write_One_Byte((param_1 & 0xffff) >> 8 & 0xef);
      }
      if (_plane_select_bit == '\x01') {
        SPI_CONTROLLER_Write_One_Byte(param_1 >> 8 & 0xff | 0x10);
      }
    }
    SPI_CONTROLLER_Write_One_Byte(param_1 & 0xff);
    if (param_4 == 0) {
      SPI_CONTROLLER_Write_NByte(param_2,param_3,0);
    }
    else if (param_4 == 1) {
      SPI_CONTROLLER_Write_NByte(param_2,param_3,2);
    }
    SPI_CONTROLLER_Chip_Select_High();
  }
  return iVar1;
}



int spi_nand_protocol_set_feature(undefined1 param_1,undefined1 param_2)

{
  int iVar1;
  
  iVar1 = spi_nand_protocol_chk_status(1,0);
  if (iVar1 == 0) {
    SPI_CONTROLLER_Chip_Select_Low();
    SPI_CONTROLLER_Write_One_Byte(0x1f);
    SPI_CONTROLLER_Write_One_Byte(param_1);
    SPI_CONTROLLER_Write_One_Byte(param_2);
    SPI_CONTROLLER_Chip_Select_High();
  }
  return iVar1;
}



void spi_nand_protocol_die_select_2(int param_1)

{
  int iVar1;
  byte local_10 [8];
  
  iVar1 = spi_nand_protocol_get_feature(0xd0,local_10);
  if (iVar1 == 0) {
    if (param_1 == 0) {
      local_10[0] = local_10[0] & 0xbf;
    }
    else {
      local_10[0] = local_10[0] | 0x40;
    }
    spi_nand_protocol_set_feature(0xd0,local_10[0]);
  }
  return;
}



void spi_nand_direct_select_die(uint param_1)

{
  if ((_current_flash_info_t._44_4_ & 2) == 0) {
    if (((_current_flash_info_t._44_4_ & 4) != 0) && (_die_id != param_1)) {
      _die_id = (byte)param_1;
      spi_nand_protocol_die_select_2(param_1 & 0xff);
    }
  }
  else if (_die_id != param_1) {
    _die_id = (byte)param_1;
    spi_nand_protocol_die_select_1(param_1 & 0xff);
  }
  return;
}



int spi_nand_protocol_show_status(void)

{
  int iVar1;
  uint uVar2;
  undefined1 auStack_18 [12];
  
  iVar1 = 0;
  if (_current_flash_info_t[0x24] != 0) {
    uVar2 = 0;
    while( true ) {
      spi_nand_direct_select_die(uVar2);
      iVar1 = spi_nand_protocol_get_feature(0xa0,auStack_18);
      if ((iVar1 != 0) || (iVar1 = spi_nand_protocol_get_feature(0xb0,auStack_18), iVar1 != 0))
      break;
      iVar1 = spi_nand_protocol_get_feature(0xc0,auStack_18);
      if (iVar1 != 0) {
        return iVar1;
      }
      uVar2 = uVar2 + 1 & 0xff;
      if (_current_flash_info_t[0x24] <= uVar2) {
        return 0;
      }
    }
  }
  return iVar1;
}



void spi_nand_select_die(uint param_1)

{
  uint uVar1;
  
  if ((_current_flash_info_t._44_4_ & 2) == 0) {
    uVar1 = (param_1 & 0x1ffffff) >> 0x11;
    if (((_current_flash_info_t._44_4_ & 4) != 0) && (_die_id != uVar1)) {
      _die_id = (byte)((param_1 << 7) >> 0x18);
      spi_nand_protocol_die_select_2(uVar1);
    }
  }
  else {
    uVar1 = (param_1 & 0xffffff) >> 0x10;
    if (otp_status != 0) {
      uVar1 = 0;
    }
    if (_die_id != uVar1) {
      _die_id = (byte)uVar1;
      spi_nand_protocol_die_select_1(uVar1);
    }
  }
  return;
}



// WARNING: Globals starting with '_' overlap smaller symbols at the same address

void spi_nand_set_clock_speed(int param_1)

{
  uint uVar1;
  
  if ((_parallel_nand_mode == '\0') && (-1 < _DAT_bfb00284 << 9)) {
    uVar1 = 400;
    if (((_DAT_bfb00064 & 0xffff0000) != 0x80000) &&
       ((((_DAT_bfb00064 & 0xffff0000) != 0x90000 && ((_DAT_bfb00064 & 0xffff0000) != 0xb0000)) &&
        (uVar1 = 400, (_DAT_bfb00064 & 0xffff0000) != 0xa0000)))) {
      uVar1 = 500;
    }
    if (param_1 << 1 == 0) {
      trap(7);
    }
    _DAT_bfa2011c = _DAT_bfa2011c & 0xffff0000 | 1 | uVar1 / (uint)(param_1 << 1) << 8;
  }
  return;
}



void SPI_NAND_Flash_Clear_Read_Cache_Data(void)

{
  _current_page_num = 0xffffffff;
  return;
}



void SPI_NAND_Flash_Set_DmaMode(undefined4 param_1)

{
  _spi_dma_mode = param_1;
  return;
}



void SPI_NAND_Flash_Get_DmaMode(undefined4 *param_1)

{
  *param_1 = _spi_dma_mode;
  return;
}



undefined4 SPI_NAND_Flash_Enable_OnDie_ECC(void)

{
  uint uVar1;
  uint uVar2;
  byte local_18 [8];
  
  if (_parallel_nand_mode == '\0') {
    if ((_current_flash_info_t._44_4_ & 6) == 0) {
      spi_nand_protocol_get_feature(_current_flash_info_t[0x3c],local_18);
      if ((local_18[0] & _current_flash_info_t[0x3d]) != _current_flash_info_t[0x3e]) {
        local_18[0] = ~_current_flash_info_t[0x3d] & local_18[0] |
                      _current_flash_info_t[0x3e] & _current_flash_info_t[0x3d];
        spi_nand_protocol_set_feature(_current_flash_info_t[0x3c],local_18[0]);
        spi_nand_protocol_get_feature(_current_flash_info_t[0x3c],local_18);
        if ((local_18[0] & _current_flash_info_t[0x3d]) == 0) {
          return 8;
        }
        return 0;
      }
    }
    else {
      uVar1 = (uint)_current_flash_info_t[0x24];
      uVar2 = 0;
      if (uVar1 != 0) {
        do {
          if (uVar1 == 0) {
            trap(7);
          }
          if (_current_flash_info_t._12_4_ == 0) {
            trap(7);
          }
          spi_nand_select_die((((uint)_current_flash_info_t._8_4_ / uVar1) * uVar2) /
                              (uint)_current_flash_info_t._12_4_);
          spi_nand_protocol_get_feature(_current_flash_info_t[0x3c],local_18);
          if ((local_18[0] & _current_flash_info_t[0x3d]) != _current_flash_info_t[0x3e]) {
            local_18[0] = ~_current_flash_info_t[0x3d] & local_18[0] |
                          _current_flash_info_t[0x3d] & _current_flash_info_t[0x3e];
            spi_nand_protocol_set_feature(_current_flash_info_t[0x3c],local_18[0]);
            spi_nand_protocol_get_feature(_current_flash_info_t[0x3c],local_18);
            if ((local_18[0] & _current_flash_info_t[0x3d]) == 0) {
              return 8;
            }
            return 0;
          }
          uVar2 = uVar2 + 1 & 0xff;
          uVar1 = (uint)_current_flash_info_t[0x24];
        } while (uVar2 < uVar1);
      }
    }
    _ondie_ecc_flag = 1;
  }
  else {
    _ondie_ecc_flag = 0;
  }
  return 0;
}



undefined4 SPI_NAND_Flash_Disable_OnDie_ECC(void)

{
  uint uVar1;
  uint uVar2;
  byte local_18 [8];
  
  if (_parallel_nand_mode == '\0') {
    if ((_current_flash_info_t._44_4_ & 6) == 0) {
      spi_nand_protocol_get_feature(_current_flash_info_t[0x3c],local_18);
      if (_current_flash_info_t[0x3e] == (local_18[0] & _current_flash_info_t[0x3d])) {
        local_18[0] = _current_flash_info_t[0x3d] & ~(local_18[0] & _current_flash_info_t[0x3d]) |
                      ~_current_flash_info_t[0x3d] & local_18[0];
        spi_nand_protocol_set_feature(_current_flash_info_t[0x3c],local_18[0]);
        spi_nand_protocol_get_feature(_current_flash_info_t[0x3c],local_18);
        if ((local_18[0] & _current_flash_info_t[0x3d]) == 0) {
          return 0;
        }
        return 9;
      }
    }
    else {
      uVar1 = (uint)_current_flash_info_t[0x24];
      uVar2 = 0;
      if (uVar1 != 0) {
        do {
          if (uVar1 == 0) {
            trap(7);
          }
          if (_current_flash_info_t._12_4_ == 0) {
            trap(7);
          }
          spi_nand_select_die((((uint)_current_flash_info_t._8_4_ / uVar1) * uVar2) /
                              (uint)_current_flash_info_t._12_4_);
          spi_nand_protocol_get_feature(_current_flash_info_t[0x3c],local_18);
          if (_current_flash_info_t[0x3e] == (local_18[0] & _current_flash_info_t[0x3d])) {
            local_18[0] = _current_flash_info_t[0x3d] & ~(local_18[0] & _current_flash_info_t[0x3d])
                          | ~_current_flash_info_t[0x3d] & local_18[0];
            spi_nand_protocol_set_feature(_current_flash_info_t[0x3c],local_18[0]);
            spi_nand_protocol_get_feature(_current_flash_info_t[0x3c],local_18);
            if ((local_18[0] & _current_flash_info_t[0x3d]) == 0) {
              return 0;
            }
            return 9;
          }
          uVar2 = uVar2 + 1 & 0xff;
          uVar1 = (uint)_current_flash_info_t[0x24];
        } while (uVar2 < uVar1);
      }
    }
  }
  _ondie_ecc_flag = 0;
  return 0;
}



// WARNING: Globals starting with '_' overlap smaller symbols at the same address

void set_spi_quad_mode_shared_pin(void)

{
  if (-1 < _DAT_bfb00284 << 9) {
    if ((_DAT_bfb00064 & 0xffff0000) != 0x80000) {
      if (((_DAT_bfb00064 & 0xffff0000) != 0x90000) && ((_DAT_bfb00064 & 0xffff0000) != 0xb0000)) {
        if ((_DAT_bfb00064 & 0xffff0000) == 0xa0000) {
          _DAT_bfa2015c = _DAT_bfa2015c | 0x10;
          return;
        }
        do {
                    // WARNING: Do nothing block with infinite loop
        } while( true );
      }
      _DAT_bfa2015c = _DAT_bfa2015c & 0xfefdbfff | 0x8000000;
      return;
    }
    _DAT_bfa2015c = _DAT_bfa2015c & 0xfffbf677 | 0x80000;
  }
  return;
}



void enable_quad(void)

{
  uint uVar1;
  byte local_18 [12];
  
  uVar1 = 0;
  if (_current_flash_info_t[0x24] != 0) {
    do {
      spi_nand_direct_select_die(uVar1);
      if (_current_flash_info_t[0x3a] != 0) {
        spi_nand_protocol_get_feature(0xb0,local_18);
        local_18[0] = ~_current_flash_info_t[0x3a] & local_18[0] | _current_flash_info_t[0x3b];
        spi_nand_protocol_set_feature(0xb0,local_18[0]);
        spi_nand_protocol_get_feature(0xb0,local_18);
      }
      uVar1 = uVar1 + 1 & 0xff;
    } while (uVar1 < _current_flash_info_t[0x24]);
  }
  set_spi_quad_mode_shared_pin();
  return;
}



int spi_nand_probe(undefined1 *param_1)

{
  char cVar1;
  int iVar2;
  uint uVar3;
  byte local_18 [8];
  
  iVar2 = spi_nand_protocol_chk_status(1,0);
  if (iVar2 == 0) {
    SPI_CONTROLLER_Chip_Select_Low();
    SPI_CONTROLLER_Write_One_Byte(0x9f);
    SPI_CONTROLLER_Write_One_Byte(0);
    SPI_CONTROLLER_Read_NByte(param_1,1,0);
    SPI_CONTROLLER_Read_NByte(param_1 + 1,1,0);
    SPI_CONTROLLER_Chip_Select_High();
  }
  iVar2 = scan_spi_nand_table(param_1);
  if (iVar2 == 0) {
    prom_puts("===mfr_id:0x");
    prom_print_hex(*param_1,8);
    prom_puts(", dev_id:0x");
    prom_print_hex(param_1[1],8);
    prom_puts(&_LC2);
    cVar1 = param_1[0x24];
  }
  else {
    iVar2 = spi_nand_protocol_chk_status(1,0);
    if (iVar2 == 0) {
      SPI_CONTROLLER_Chip_Select_Low();
      SPI_CONTROLLER_Write_One_Byte(0x9f);
      SPI_CONTROLLER_Read_NByte(param_1,1,0);
      SPI_CONTROLLER_Read_NByte(param_1 + 1,1,0);
      SPI_CONTROLLER_Chip_Select_High();
    }
    iVar2 = scan_spi_nand_table(param_1);
    prom_puts("===mfr_id:0x");
    prom_print_hex(*param_1,8);
    prom_puts(", dev_id:0x");
    prom_print_hex(param_1[1],8);
    prom_puts(&_LC2);
    if (iVar2 != 0) {
      return iVar2;
    }
    cVar1 = param_1[0x24];
  }
  if (cVar1 != '\0') {
    uVar3 = 0;
    do {
      spi_nand_direct_select_die(uVar3);
      if (param_1[0x38] != '\0') {
        spi_nand_protocol_get_feature(0xa0,local_18);
        local_18[0] = ~param_1[0x38] & local_18[0] | param_1[0x38] & param_1[0x39];
        spi_nand_protocol_set_feature(0xa0,local_18[0]);
        spi_nand_protocol_get_feature(0xa0,local_18);
      }
      uVar3 = uVar3 + 1 & 0xff;
    } while (uVar3 < (byte)param_1[0x24]);
  }
  if ((*(int *)(param_1 + 0x28) == 1) || (*(int *)(param_1 + 0x1c) == 2)) {
    enable_quad();
  }
  return 0;
}



int spi_nand_protocol_write_enable(void)

{
  int iVar1;
  int iVar2;
  undefined1 auStack_10 [8];
  
  iVar1 = spi_nand_protocol_chk_status(1,0);
  if ((iVar1 == 0) && (_parallel_nand_mode == '\0')) {
    SPI_CONTROLLER_Chip_Select_Low();
    SPI_CONTROLLER_Write_One_Byte(6);
    SPI_CONTROLLER_Chip_Select_High();
    iVar1 = spi_nand_protocol_chk_status(2,2);
    if ((iVar1 != 0) && (iVar2 = spi_nand_protocol_get_feature(0xc0,auStack_10), iVar2 != 0)) {
      iVar1 = iVar2;
    }
  }
  return iVar1;
}



int spi_nand_protocol_write_disable(void)

{
  int iVar1;
  int iVar2;
  undefined1 auStack_10 [8];
  
  iVar1 = spi_nand_protocol_chk_status(1,0);
  if ((iVar1 == 0) && (_parallel_nand_mode == '\0')) {
    SPI_CONTROLLER_Chip_Select_Low();
    SPI_CONTROLLER_Write_One_Byte(4);
    SPI_CONTROLLER_Chip_Select_High();
    iVar1 = spi_nand_protocol_chk_status(2,0);
    if ((iVar1 != 0) && (iVar2 = spi_nand_protocol_get_feature(0xc0,auStack_10), iVar2 != 0)) {
      iVar1 = iVar2;
    }
  }
  return iVar1;
}



int spi_nand_protocol_program_execute(uint param_1)

{
  int iVar1;
  byte local_10 [8];
  
  iVar1 = spi_nand_protocol_chk_status(1,0);
  if (iVar1 == 0) {
    SPI_CONTROLLER_Chip_Select_Low();
    SPI_CONTROLLER_Write_One_Byte(0x10);
    SPI_CONTROLLER_Write_One_Byte((param_1 & 0xffffff) >> 0x10);
    SPI_CONTROLLER_Write_One_Byte((param_1 & 0xffff) >> 8);
    SPI_CONTROLLER_Write_One_Byte(param_1 & 0xff);
    SPI_CONTROLLER_Chip_Select_High();
    iVar1 = spi_nand_protocol_chk_status(1,0);
    if (((iVar1 == 0) && (iVar1 = spi_nand_protocol_get_feature(0xc0,local_10), iVar1 == 0)) &&
       ((local_10[0] & 8) != 0)) {
      iVar1 = 5;
    }
  }
  return iVar1;
}



// WARNING: Globals starting with '_' overlap smaller symbols at the same address

int SPI_NAND_Flash_Write_Nbyte(uint param_1,int param_2,int *param_3,int param_4,int param_5)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  uint uVar5;
  int iVar6;
  uint uVar7;
  undefined1 uStack_b0;
  byte local_af;
  undefined1 auStack_ae [2];
  int local_ac;
  undefined4 local_a8;
  undefined4 local_a4;
  int local_a0;
  int local_9c;
  int local_98;
  int local_94;
  int local_90;
  byte local_88;
  byte local_87;
  int local_84;
  uint local_80;
  byte local_7c;
  int local_6c;
  int local_68;
  int local_5c;
  uint local_58;
  byte local_54;
  int local_40;
  int local_3c;
  uint local_38;
  int local_34;
  uint local_30;
  int local_2c;
  
  *param_3 = 0;
  SPI_NAND_Flash_Clear_Read_Cache_Data();
  iVar1 = 0;
  local_3c = param_2;
  do {
    if (local_3c == 0) {
      *param_3 = param_2;
      return iVar1;
    }
    uVar7 = param_1 / (uint)_current_flash_info_t._12_4_;
    local_30 = param_1 % (uint)_current_flash_info_t._12_4_;
    if (_current_flash_info_t._12_4_ == 0) {
      trap(7);
    }
    local_34 = local_3c;
    if ((uint)_current_flash_info_t._12_4_ < local_30 + local_3c) {
      local_34 = _current_flash_info_t._12_4_ - local_30;
    }
    local_2c = param_4 + (param_2 - local_3c);
    local_38 = uVar7;
    SPI_NFI_Get_Configure(&local_6c);
    SPI_CONTROLLER_Enable_Manual_Mode();
    uVar5 = _current_page_num;
    if (uVar7 != _current_page_num) {
      SPI_CONTROLLER_Enable_Manual_Mode();
      if ((_parallel_nand_mode == '\0') && (uVar7 != _current_page_num)) {
        spi_nand_select_die(local_38);
        iVar1 = spi_nand_protocol_chk_status(1,0);
        if (iVar1 == 0) {
          SPI_CONTROLLER_Chip_Select_Low();
          SPI_CONTROLLER_Write_One_Byte(0x13);
          SPI_CONTROLLER_Write_One_Byte((local_38 & 0xffffff) >> 0x10);
          SPI_CONTROLLER_Write_One_Byte((local_38 & 0xffff) >> 8);
          SPI_CONTROLLER_Write_One_Byte(local_38 & 0xff);
          SPI_CONTROLLER_Chip_Select_High();
        }
        do {
          spi_nand_protocol_get_feature(0xc0,&local_af);
        } while ((local_af & 1) != 0);
        if ((((((((_DAT_bfb00064 & 0xffff0000) != 0x90000) &&
                ((_DAT_bfb00064 & 0xffff0000) != 0xb0000)) &&
               ((_DAT_bfb00064 & 0xffff0000) != 0xa0000)) &&
              (((_DAT_bfb00064 & 0xffff0000) != 0x70000 && ((_DAT_bfb00064 & 0xffff0000) != 0x80000)
               ))) && ((_DAT_bfb00064 & 0xffff0000) != 0x40000)) &&
            ((_DAT_bfb00064 & 0xffff0000) != 0x50000)) ||
           (((_DAT_bfa10114 & 2) == 0 || (-1 < _DAT_bfb00284 << 8)))) {
          spi_nand_protocol_get_feature(0xc0,&uStack_b0);
        }
      }
      uVar2 = _current_flash_info_t._24_4_;
      uVar4 = _current_flash_info_t._20_4_;
      uVar3 = _current_flash_info_t._12_4_;
      iVar1 = _current_cache_page;
      if ((_current_flash_info_t._44_4_ & 1) != 0) {
        _plane_select_bit = (byte)((local_38 << 0x19) >> 0x1f);
      }
      if (_spi_dma_mode == 1) {
        SPI_CONTROLLER_Get_Configure(&local_a8);
        local_a4 = 0;
        if (param_5 == 1) {
          if (_current_flash_info_t._24_4_ == 0) {
            local_a4 = 1;
            uVar4 = 0x3b;
            uVar3 = 1;
          }
          else {
            uVar4 = 0x3b;
            uVar3 = 1;
          }
        }
        else {
          uVar4 = 3;
          if (param_5 == 2) {
            if (_current_flash_info_t._24_4_ == 0) {
              local_a4 = 1;
              uVar4 = 0x6b;
              uVar3 = 2;
            }
            else {
              uVar4 = 0x6b;
              uVar3 = 2;
            }
          }
          else {
            uVar3 = 0;
          }
        }
        local_a8 = 2;
        SPI_CONTROLLER_Set_Configure(&local_a8);
        SPI_NFI_Reset();
        SPI_NFI_Get_Configure(&local_94);
        SPI_NFI_Set_Configure(&local_94);
        SPI_ECC_Decode_Get_Configure(&local_a0);
        SPI_ECC_Decode_Set_Configure(&local_a0);
        if ((local_90 == 1) && (SPI_ECC_Decode_Get_Configure(&local_a0), local_a0 == 1)) {
          local_9c = (local_87 + 0x200) * 8 + local_98 * 0xd;
          SPI_ECC_Decode_Set_Configure(&local_a0);
          SPI_ECC_Decode_Disable();
          SPI_ECC_Encode_Disable();
          SPI_ECC_Decode_Enable();
        }
        uVar2 = 0;
        if ((((_current_flash_info_t._44_4_ & 1) != 0) && (_plane_select_bit != 0)) &&
           (_plane_select_bit == 1)) {
          uVar2 = 0x1000;
        }
        flush_dcache_range(dma_read_page,dma_read_page + 0x1100);
        SPI_NFI_Read_SPI_NAND_Page(uVar3,uVar4,uVar2,dma_read_page & 0x1fffffff);
        if ((local_90 == 1) && (iVar1 = 0, local_a0 == 1)) {
          do {
            SPI_ECC_Decode_Check_Done(&local_ac);
            if (local_ac == 2) {
              if (iVar1 != 1000000) goto LAB_0001149c;
              break;
            }
            iVar1 = iVar1 + 1;
          } while (iVar1 != 1000000);
          SPI_NAND_Flash_Clear_Read_Cache_Data();
LAB_0001149c:
          iVar1 = SPI_ECC_DECODE_Check_Correction_Status();
          if (iVar1 == 1) {
            SPI_NAND_Flash_Clear_Read_Cache_Data();
            SPI_CONTROLLER_Enable_Manual_Mode();
          }
        }
        SPI_CONTROLLER_Enable_Manual_Mode();
      }
      else {
        iVar6 = spi_nand_protocol_chk_status(1,0);
        if (iVar6 == 0) {
          SPI_CONTROLLER_Chip_Select_Low();
          if (param_5 == 1) {
            SPI_CONTROLLER_Write_One_Byte(0x3b);
          }
          else if (param_5 == 0) {
            SPI_CONTROLLER_Write_One_Byte(3);
          }
          else if (param_5 == 2) {
            SPI_CONTROLLER_Write_One_Byte(0x6b);
          }
          if (uVar2 == 0) {
            SPI_CONTROLLER_Write_One_Byte(0xff);
          }
          if ((_current_flash_info_t._44_4_ & 1) == 0) {
            SPI_CONTROLLER_Write_One_Byte(0);
          }
          else {
            if (_plane_select_bit == 0) {
              SPI_CONTROLLER_Write_One_Byte(0);
            }
            if (_plane_select_bit == 1) {
              SPI_CONTROLLER_Write_One_Byte(0x10);
            }
          }
          iVar6 = uVar4 + uVar3;
          if ((uVar2 == 0) && (param_5 == 0)) {
            SPI_CONTROLLER_Write_One_Byte_With_Cmd(0x10,0);
LAB_00011648:
            if (param_5 == 1) {
              SPI_CONTROLLER_Write_One_Byte_With_Cmd(0x11,0xff);
LAB_000116c8:
              SPI_CONTROLLER_Read_NByte(iVar1,iVar6,1);
            }
            else {
              if (param_5 != 2) goto LAB_00011688;
              SPI_CONTROLLER_Write_One_Byte_With_Cmd(0x12,0xff);
LAB_000116e0:
              SPI_CONTROLLER_Read_NByte(iVar1,iVar6,2);
            }
          }
          else {
            SPI_CONTROLLER_Write_One_Byte(0);
            if (uVar2 == 1) {
              if (param_5 == 1) {
                SPI_CONTROLLER_Write_One_Byte_With_Cmd(0x11,0xff);
                goto LAB_00011640;
              }
              if (param_5 != 0) {
                if (param_5 == 2) {
                  SPI_CONTROLLER_Write_One_Byte_With_Cmd(0x12,0xff);
                }
                goto LAB_00011640;
              }
              SPI_CONTROLLER_Write_One_Byte_With_Cmd(0x10,0xff);
            }
            else {
LAB_00011640:
              if (uVar2 == 0) goto LAB_00011648;
LAB_00011688:
              if (param_5 == 1) goto LAB_000116c8;
              if (param_5 != 0) {
                if (param_5 == 2) goto LAB_000116e0;
                goto LAB_000116e8;
              }
            }
            SPI_CONTROLLER_Read_NByte(iVar1,iVar6,0);
          }
LAB_000116e8:
          SPI_CONTROLLER_Chip_Select_High();
        }
      }
      if (_spi_dma_mode == 0) {
        memcpy4(_current_cache_page_data,_current_cache_page,_current_flash_info_t._12_4_);
        memcpy4(_current_cache_page_oob,_current_cache_page + _current_flash_info_t._12_4_,
                _current_flash_info_t._20_4_);
LAB_00011860:
        uVar3 = _current_flash_info_t._32_4_;
        if (_ondie_ecc_flag == '\x01') {
          uVar5 = local_38;
          if ((*(int *)(_current_flash_info_t._32_4_ + 8) != 0) &&
             (uVar5 = local_38, *(int *)_current_flash_info_t._32_4_ != 0)) {
            iVar6 = 0;
            uVar7 = 0;
            iVar1 = _current_flash_info_t._32_4_;
            do {
              if ((*(int *)(iVar1 + 8) != 0) &&
                 (uVar5 = 0, uVar7 < *(uint *)_current_flash_info_t._32_4_)) {
                do {
                  *(undefined1 *)(_current_cache_page_oob_mapping + uVar7) =
                       *(undefined1 *)(_current_cache_page_oob + uVar5 + *(int *)(iVar1 + 4));
                  uVar5 = uVar5 + 1;
                  uVar7 = uVar7 + 1;
                  if (*(uint *)(iVar1 + 8) <= uVar5) break;
                } while (uVar7 < *(uint *)_current_flash_info_t._32_4_);
              }
              iVar6 = iVar6 + 1;
              uVar5 = local_38;
            } while (((iVar6 != 0x20) &&
                     (iVar1 = uVar3 + iVar6 * 8, uVar5 = local_38, *(int *)(iVar1 + 8) != 0)) &&
                    (uVar5 = local_38, uVar7 < *(uint *)_current_flash_info_t._32_4_));
          }
        }
        else {
          memcpy4(_current_cache_page_oob_mapping,_current_cache_page_oob,
                  _current_flash_info_t._20_4_);
          uVar5 = local_38;
        }
      }
      else {
        if ((_spi_dma_mode == 1) &&
           ((((((_DAT_bfb00064 & 0xffff0000) != 0x90000 && ((_DAT_bfb00064 & 0xffff0000) != 0xb0000)
               ) && (((_DAT_bfb00064 & 0xffff0000) != 0xa0000 &&
                     ((((_DAT_bfb00064 & 0xffff0000) != 0x70000 &&
                       ((_DAT_bfb00064 & 0xffff0000) != 0x80000)) &&
                      ((_DAT_bfb00064 & 0xffff0000) != 0x40000)))))) &&
             ((_DAT_bfb00064 & 0xffff0000) != 0x50000)) ||
            (((_DAT_bfa10114 & 2) == 0 || (-1 < _DAT_bfb00284 << 8)))))) {
          if (local_94 != 0) {
LAB_00011a50:
            memcpy4(_current_cache_page_data,dma_read_page,_current_flash_info_t._12_4_);
            SPI_NFI_Read_SPI_NAND_FDM(_current_cache_page_oob,_current_flash_info_t._20_4_);
            memcpy4(_current_cache_page_oob_mapping,_current_cache_page_oob,
                    _current_flash_info_t._20_4_);
            uVar5 = local_38;
            goto LAB_00011a90;
          }
          if (local_90 == 0) {
            memcpy4(_current_cache_page_data,dma_read_page,_current_flash_info_t._12_4_);
            memcpy4(_current_cache_page_oob,dma_read_page + _current_flash_info_t._12_4_,
                    _current_flash_info_t._20_4_);
            goto LAB_00011860;
          }
        }
        else if (local_94 != 0) goto LAB_00011a50;
        uVar7 = local_80 / local_7c;
        if (local_7c == 0) {
          trap(7);
          uVar5 = local_38;
        }
        else {
          uVar5 = 0;
          iVar6 = 0;
          iVar1 = 0;
          local_40 = 0;
          do {
            memcpy4(_current_cache_page_data + local_40,dma_read_page + iVar1,uVar7);
            memcpy4(_current_cache_page_oob + iVar6,dma_read_page + iVar1 + uVar7,local_84);
            local_40 = local_40 + uVar7;
            iVar1 = iVar1 + uVar7 + local_84;
            iVar6 = iVar6 + local_84;
            uVar5 = uVar5 + 1;
          } while (uVar5 < local_7c);
          iVar1 = 0;
          uVar5 = local_38;
          if (local_7c != 0) {
            uVar7 = 0;
            iVar6 = 0;
            do {
              memcpy4(_current_cache_page_oob_mapping + iVar6,_current_cache_page_oob + iVar1,
                      local_88);
              iVar6 = iVar6 + (uint)local_88;
              iVar1 = iVar1 + local_84;
              uVar7 = uVar7 + 1;
              uVar5 = local_38;
            } while (uVar7 < local_7c);
          }
        }
      }
    }
LAB_00011a90:
    _current_page_num = uVar5;
    flush_dcache_range(dma_write_page,dma_write_page + 0x1100);
    if ((_spi_dma_mode == 0) ||
       (((_spi_dma_mode == 1 &&
         (((((((_DAT_bfb00064 & 0xffff0000) != 0x90000 && ((_DAT_bfb00064 & 0xffff0000) != 0xb0000))
             && (((_DAT_bfb00064 & 0xffff0000) != 0xa0000 &&
                 (((_DAT_bfb00064 & 0xffff0000) != 0x70000 &&
                  ((_DAT_bfb00064 & 0xffff0000) != 0x80000)))))) &&
            ((_DAT_bfb00064 & 0xffff0000) != 0x40000)) && ((_DAT_bfb00064 & 0xffff0000) != 0x50000))
          || (((_DAT_bfa10114 & 2) == 0 || (-1 < _DAT_bfb00284 << 8)))))) &&
        ((local_6c == 0 && (local_68 == 0)))))) {
      if (local_34 != 0) {
        memcpy4(_current_cache_page_data + local_30,local_2c,local_34);
      }
      if (_spi_dma_mode == 0) {
        memcpy4(_current_cache_page + local_30,_current_cache_page_data,_current_flash_info_t._12_4_
               );
      }
      else {
        memcpy4(dma_write_page + local_30 | 0xa0000000,_current_cache_page_data,
                _current_flash_info_t._12_4_);
      }
      if (_spi_dma_mode == 0) {
        memcpy4(_current_cache_page + _current_flash_info_t._12_4_,_current_cache_page_oob,
                _current_flash_info_t._20_4_);
      }
      else {
        memcpy4(dma_write_page + _current_flash_info_t._12_4_ | 0xa0000000,_current_cache_page_oob,
                _current_flash_info_t._20_4_);
      }
    }
    else {
      if (local_34 != 0) {
        memcpy4(_current_cache_page_data + local_30,local_2c,local_34);
      }
      if (local_6c == 0) {
        uVar7 = local_58 / local_54;
        if (local_54 == 0) {
          trap(7);
        }
        else {
          uVar5 = 0;
          iVar6 = 0;
          local_40 = 0;
          iVar1 = 0;
          do {
            memcpy4(dma_write_page + iVar1 | 0xa0000000,_current_cache_page_data + local_40,uVar7);
            memcpy4(dma_write_page + iVar1 + uVar7 | 0xa0000000,_current_cache_page_oob + iVar6,
                    local_5c);
            local_40 = local_40 + uVar7;
            iVar6 = iVar6 + local_5c;
            uVar5 = uVar5 + 1;
            iVar1 = iVar1 + uVar7 + local_5c;
          } while (uVar5 < local_54);
        }
      }
      else {
        memcpy4(dma_write_page | 0xa0000000,_current_cache_page_data,_current_flash_info_t._12_4_);
      }
    }
    if ((_current_flash_info_t._44_4_ & 1) != 0) {
      _plane_select_bit = (byte)((local_38 << 0x19) >> 0x1f);
    }
    spi_nand_select_die(local_38);
    if (_current_flash_info_t._52_4_ == 0) {
      if (_spi_dma_mode == 1) {
        iVar1 = spi_nand_dma_program_load(0,0,param_5);
        if (iVar1 != 0) {
          SPI_NAND_Flash_Clear_Read_Cache_Data();
          SPI_CONTROLLER_Enable_Manual_Mode();
          goto LAB_00011f60;
        }
      }
      else {
        spi_nand_protocol_program_load
                  (0,_current_cache_page,_current_flash_info_t._20_4_ + _current_flash_info_t._12_4_
                   ,param_5);
      }
      iVar1 = spi_nand_protocol_write_enable();
      if (iVar1 == 0) goto LAB_00011ef0;
      SPI_NAND_Flash_Clear_Read_Cache_Data();
      SPI_CONTROLLER_Enable_Manual_Mode();
    }
    else {
      iVar1 = spi_nand_protocol_write_enable();
      if (iVar1 == 0) {
        if (_spi_dma_mode == 1) {
          iVar1 = spi_nand_dma_program_load(0,0,param_5);
          if (iVar1 != 0) {
            SPI_NAND_Flash_Clear_Read_Cache_Data();
            SPI_CONTROLLER_Enable_Manual_Mode();
            goto LAB_00011f60;
          }
        }
        else {
          spi_nand_protocol_program_load
                    (0,_current_cache_page,
                     _current_flash_info_t._20_4_ + _current_flash_info_t._12_4_,param_5);
        }
LAB_00011ef0:
        iVar6 = spi_nand_protocol_program_execute(local_38);
        iVar1 = spi_nand_protocol_write_disable();
        if (iVar1 == 0) {
          if (iVar6 == 0) {
            spi_nand_protocol_get_feature(0xa0,auStack_ae);
            print_dot = print_dot + 1;
            SPI_NAND_Flash_Clear_Read_Cache_Data();
          }
          else {
            SPI_NAND_Flash_Clear_Read_Cache_Data();
            SPI_CONTROLLER_Enable_Manual_Mode();
            iVar1 = iVar6;
          }
        }
        else {
          SPI_NAND_Flash_Clear_Read_Cache_Data();
          SPI_CONTROLLER_Enable_Manual_Mode();
        }
      }
      else {
        SPI_NAND_Flash_Clear_Read_Cache_Data();
        SPI_CONTROLLER_Enable_Manual_Mode();
      }
    }
LAB_00011f60:
    param_1 = param_1 + local_34;
    local_3c = local_3c - local_34;
    *param_3 = *param_3 + local_34;
  } while( true );
}



undefined4 nandflash_write(uint param_1,int param_2,int *param_3,int param_4)

{
  int iVar1;
  undefined4 uVar2;
  
  iVar1 = SPI_NAND_Flash_Write_Nbyte(param_1,param_2,param_3,param_4,_current_flash_info_t._40_4_);
  uVar2 = 0xffffffff;
  if (iVar1 == 0) {
    uVar2 = 0;
  }
  return uVar2;
}



undefined4 spi_nand_erase_block(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  uint uVar3;
  byte local_10 [8];
  
  if (_current_flash_info_t._12_4_ == 0) {
    trap(7);
  }
  spi_nand_select_die(param_1 * ((uint)_current_flash_info_t._16_4_ /
                                (uint)_current_flash_info_t._12_4_));
  iVar1 = spi_nand_protocol_write_enable();
  uVar2 = 4;
  if (iVar1 == 0) {
    iVar1 = spi_nand_protocol_chk_status(1,0);
    if (iVar1 == 0) {
      uVar3 = param_1 << 6;
      SPI_CONTROLLER_Chip_Select_Low();
      SPI_CONTROLLER_Write_One_Byte(0xd8);
      SPI_CONTROLLER_Write_One_Byte((uVar3 & 0xffffff) >> 0x10);
      SPI_CONTROLLER_Write_One_Byte((uVar3 & 0xffff) >> 8);
      SPI_CONTROLLER_Write_One_Byte(uVar3 & 0xc0);
      SPI_CONTROLLER_Chip_Select_High();
    }
    do {
      spi_nand_protocol_get_feature(0xc0,local_10);
    } while ((local_10[0] & 1) != 0);
    iVar1 = spi_nand_protocol_write_disable();
    if (iVar1 == 0) {
      uVar2 = 0;
      if ((local_10[0] & 4) != 0) {
        uVar2 = 4;
      }
    }
    else {
      uVar2 = 4;
    }
  }
  return uVar2;
}



undefined4 SPI_NAND_Flash_Erase(uint param_1,uint param_2)

{
  undefined4 uVar1;
  int iVar2;
  uint uVar3;
  
  SPI_CONTROLLER_Enable_Manual_Mode();
  SPI_NAND_Flash_Clear_Read_Cache_Data();
  uVar3 = param_2;
  if ((param_2 & _current_flash_info_t._16_4_ - 1) != 0) {
    if (_current_flash_info_t._16_4_ == 0) {
      trap(7);
    }
    uVar3 = (param_2 / (uint)_current_flash_info_t._16_4_ + 1) * _current_flash_info_t._16_4_;
  }
  uVar1 = 2;
  if ((((uVar3 | param_1) & _current_flash_info_t._16_4_ - 1) == 0) && (uVar1 = 0, param_2 != 0)) {
    uVar3 = 0;
    do {
      if (_current_flash_info_t._16_4_ == 0) {
        trap(7);
      }
      iVar2 = spi_nand_erase_block(param_1 / (uint)_current_flash_info_t._16_4_);
      if (iVar2 != 0) {
        return 4;
      }
      param_1 = param_1 + _current_flash_info_t._16_4_;
      uVar3 = uVar3 + _current_flash_info_t._16_4_;
    } while (uVar3 < param_2);
    uVar1 = 0;
  }
  return uVar1;
}



undefined4 nandflash_erase(uint param_1,uint param_2)

{
  int iVar1;
  undefined4 uVar2;
  
  iVar1 = SPI_NAND_Flash_Erase(param_1,param_2);
  uVar2 = 0xffffffff;
  if (iVar1 == 0) {
    uVar2 = 0;
  }
  return uVar2;
}



// WARNING: Globals starting with '_' overlap smaller symbols at the same address

undefined4 enable_dma(int param_1,undefined4 param_2,undefined4 param_3)

{
  int iVar1;
  
  if ((((((_DAT_bfb00064 & 0xffff0000) == 0x90000) || ((_DAT_bfb00064 & 0xffff0000) == 0xb0000)) ||
       ((_DAT_bfb00064 & 0xffff0000) == 0xa0000)) ||
      ((((_DAT_bfb00064 & 0xffff0000) == 0x70000 || ((_DAT_bfb00064 & 0xffff0000) == 0x80000)) ||
       (((_DAT_bfb00064 & 0xffff0000) == 0x40000 || ((_DAT_bfb00064 & 0xffff0000) == 0x50000))))))
     && (((_DAT_bfa10114 & 2) != 0 && (_DAT_bfb00284 << 8 < 0)))) {
    _current_flash_info_t._32_4_ = &ooblayout_spi_controller_ecc;
    _ooblayout_spi_controller_ecc = (uint)*(byte *)(param_1 + 0x18) << 3;
    if (*(byte *)(param_1 + 0x18) != 0) {
      iVar1 = 0;
      do {
        *(int *)(_current_flash_info_t._32_4_ + iVar1 * 8 + 4) = iVar1 * *(int *)(param_1 + 0x10);
        *(undefined4 *)(_current_flash_info_t._32_4_ + iVar1 * 8 + 8) = 8;
        iVar1 = iVar1 + 1;
      } while (iVar1 < (int)(uint)*(byte *)(param_1 + 0x18));
    }
  }
  SPI_NAND_Flash_Set_DmaMode(1);
  SPI_NFI_Init();
  SPI_NFI_Set_Configure(param_1);
  if (*(int *)(param_1 + 4) == 1) {
    if ((((_DAT_bfb00064 & 0xffff0000) == 0x90000) || ((_DAT_bfb00064 & 0xffff0000) == 0xb0000)) ||
       ((_DAT_bfb00064 & 0xffff0000) == 0xa0000)) {
      SPI_NFI_ECC_DATA_SOURCE_INV(1);
    }
    SPI_ECC_Encode_Init();
    SPI_ECC_Decode_Init();
    SPI_ECC_Encode_Set_Configure(param_2);
    SPI_ECC_Decode_Set_Configure(param_3);
  }
  return 0;
}



// WARNING: Globals starting with '_' overlap smaller symbols at the same address

undefined4 SPI_NAND_Flash_Init(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined1 uVar3;
  undefined4 local_48;
  undefined4 local_44;
  undefined4 local_40;
  undefined1 local_3c;
  undefined1 local_3b;
  uint local_38;
  undefined4 local_34;
  undefined1 local_30;
  undefined4 local_2c;
  undefined4 local_28;
  undefined4 local_20;
  undefined4 local_1c;
  uint local_18;
  undefined4 local_14;
  int local_10;
  uint local_c;
  
  nand_probe = spi_nand_probe;
  spi_nand_set_clock_speed(0x32);
  SPI_CONTROLLER_Enable_Manual_Mode();
  iVar1 = (*nand_probe)(_current_flash_info_t);
  uVar2 = 1;
  if (iVar1 == 0) {
    if (dma_read_page == 0) {
      iVar1 = tmp_dma_read_page - (tmp_dma_read_page & 0x1f);
      dma_read_page = iVar1 + 0x20;
      flush_dcache_range(dma_read_page,iVar1 + 0x1120);
    }
    if (dma_write_page == 0) {
      iVar1 = tmp_dma_write_page - (tmp_dma_write_page & 0x1f);
      dma_write_page = iVar1 + 0x20;
      flush_dcache_range(dma_write_page,iVar1 + 0x1120);
    }
    uVar3 = 4;
    if ((_current_flash_info_t._12_4_ != 0x800) &&
       (uVar3 = 8, _current_flash_info_t._12_4_ != 0x1000)) {
      uVar3 = 1;
    }
    if (((((((_DAT_bfb00064 & 0xffff0000) == 0x90000) || ((_DAT_bfb00064 & 0xffff0000) == 0xb0000))
          || ((_DAT_bfb00064 & 0xffff0000) == 0xa0000)) ||
         (((_DAT_bfb00064 & 0xffff0000) == 0x70000 || ((_DAT_bfb00064 & 0xffff0000) == 0x80000))))
        || (((_DAT_bfb00064 & 0xffff0000) == 0x40000 || ((_DAT_bfb00064 & 0xffff0000) == 0x50000))))
       && (((_DAT_bfa10114 & 2) != 0 && (_DAT_bfb00284 << 8 < 0)))) {
      SPI_NAND_Flash_Disable_OnDie_ECC();
      if ((_DAT_bfb00064 & 0xffff0000) == 0x80000) {
        local_38 = 0x10;
        local_18 = 4;
      }
      else {
        local_38 = _DAT_9fa40000 & 0xffff;
        local_18 = _DAT_9fa40000 >> 0x10;
      }
      local_48 = 1;
      local_44 = 1;
      local_40 = 1;
      local_3c = 8;
      local_3b = 8;
      local_34 = _current_flash_info_t._12_4_;
      local_2c = 0;
      local_28 = 0;
      local_20 = 1;
      local_1c = 0x208;
      local_14 = 1;
      local_10 = local_18 * 0xd + 0x1040;
      local_30 = uVar3;
      local_c = local_18;
      enable_dma((int)&local_48,&local_20,&local_14);
      uVar2 = 0;
    }
    else {
      SPI_NAND_Flash_Enable_OnDie_ECC();
      uVar2 = 0;
    }
  }
  return uVar2;
}



undefined4 nandflash_init(void)

{
  int iVar1;
  undefined4 uVar2;
  
  iVar1 = SPI_NAND_Flash_Init();
  uVar2 = 0xffffffff;
  if (iVar1 == 0) {
    uVar2 = 0;
  }
  return uVar2;
}


