/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef X_HAL_IO_H
#define X_HAL_IO_H

#include "dramc_pi_api.h"
//===========================================================================

// field access macro-----------------------------------------------------------

/* field macros */
#define Fld(wid,shft,ac)    (((U32)wid<<16)|(shft<<8)|ac)
#define Fld_wid(fld)    (UINT8)((fld)>>16)
#define Fld_shft(fld)   (UINT8)((fld)>>8)
#define Fld_ac(fld)     (UINT8)(fld)

/* access method*/
#define AC_FULLB0       1
#define AC_FULLB1       2
#define AC_FULLB2       3
#define AC_FULLB3       4
#define AC_FULLW10      5
#define AC_FULLW21      6
#define AC_FULLW32      7
#define AC_FULLDW       8
#define AC_MSKB0        11
#define AC_MSKB1        12
#define AC_MSKB2        13
#define AC_MSKB3        14
#define AC_MSKW10       15
#define AC_MSKW21       16
#define AC_MSKW32       17
#define AC_MSKDW        18

#define Fld2Msk32(fld)  /*lint -save -e504 */ (((U32)0xffffffff>>(32-Fld_wid(fld)))<<Fld_shft(fld)) /*lint -restore */
#define P_Fld(val,fld) ((sizeof(upk)>1)?Fld2Msk32(fld):(((UINT32)(val)&((UINT32)(1<<Fld_wid(fld))-1))<<Fld_shft(fld)))

extern U32 u4Dram_Register_Read(U32 u4reg_addr); 
extern U8 ucDram_Register_Write(U32 u4reg_addr, U32 u4reg_value);

extern void vIO32Write4BMsk(U32 reg32, U32 val32, U32 msk32);
extern void vIO32Write4BMsk_All(U32 reg32, U32 val32, U32 msk32);
#if 0
extern void vIO32Write4BMsk_C(U32 reg32, U32 val32, U32 msk32);
#endif
extern void vIO32Write4B_All(U32 reg32, U32 val32);
//extern void vIO32Write4B_All2(U32 reg32, U32 val32);
#define vIO32Write4B_All2(reg32,val32) vIO32Write4B_All(reg32,val32)

// =========================
// public Macro for general use. 
//==========================
#define u4IO32Read4B(reg32) u4Dram_Register_Read(reg32)
#define vIO32Write4B(reg32,val32) ucDram_Register_Write(reg32,val32)

#define u4IO32ReadFldAlign(reg32,fld) /*lint -save -e506 -e504 -e514 -e62 -e737 -e572 -e961 -e648 -e701 -e732 -e571 */ \
	((Fld_ac(fld)==AC_FULLDW)? u4IO32Read4B(reg32):((u4IO32Read4B(reg32)&Fld2Msk32(fld))>>Fld_shft(fld)) )

#define vIO32WriteFldAlign(reg32,val,fld) /*lint -save -e506 -e504 -e514 -e62 -e737 -e572 -e961 -e648 -e701 -e732 -e571 */ \
	((Fld_ac(fld)==AC_FULLDW)?(vIO32Write4B((reg32),(val))) : (vIO32Write4BMsk((reg32),((U32)(val)<<Fld_shft(fld)),Fld2Msk32(fld))) )

#define vIO32WriteFldMulti(reg32,list) /*lint -save -e506 -e504 -e514 -e62 -e737 -e572 -e961 -e648 -e701 -e732 -e571 */ \
{ \
	UINT16 upk; \
	INT32 msk=(INT32)(list); \
	{UINT8 upk; \
	((U32)msk==0xffffffff)?(vIO32Write4B(reg32,(list))):(((U32)msk)?vIO32Write4BMsk(reg32,(list),((U32)msk)):0); \
	} \
}/*lint -restore */

//=========================
// Public Macro for write all-dramC or all-PHY registers
//=========================
#define vIO32WriteFldAlign_All(reg32,val,fld) /*lint -save -e506 -e504 -e514 -e62 -e737 -e572 -e961 -e648 -e701 -e732 -e571 */ \
	((Fld_ac(fld)==AC_FULLDW)?(vIO32Write4B_All((reg32),(val))) : (vIO32Write4BMsk_All((reg32),((U32)(val)<<Fld_shft(fld)),Fld2Msk32(fld))) )

#define vIO32WriteFldMulti_All(reg32,list) /*lint -save -e506 -e504 -e514 -e62 -e737 -e572 -e961 -e648 -e701 -e732 -e571 */ \
{ \
	UINT16 upk; \
	INT32 msk=(INT32)(list); \
	{UINT8 upk; \
	((U32)msk==0xffffffff)?(vIO32Write4B_All(reg32,(list))):(((U32)msk)?vIO32Write4BMsk_All(reg32,(list),((U32)msk)):0); \
	} \
}/*lint -restore */

#define REG_ACCESS_PORTING_DGB 0


#endif  // X_HAL_IO_H
