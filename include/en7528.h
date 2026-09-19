/* SPDX-License-Identifier: GPL-2.0+ */
#ifndef __MACH_EN7528_H__
#define __MACH_EN7528_H__

/*
 * EN7580 uses the same 48 KiB FE SRAM window as EN7528. The vendor RAM
 * training stage is linked at the beginning of the cached alias.
 */
#define EN7528_DDR_BLOB_ADDR		0x9fa30000
#define EN7528_DDR_BLOB_ENTRY	EN7528_DDR_BLOB_ADDR
#define EN7528_DDR_BLOB_OFFSET		0x00004000
#define EN7528_DDR_BLOB_SIZE		0x0000b000

/* Keep TPL and the RAM-training payload below the 0xff00 constant-data area. */
#define EN7528_SPL_IMAGE_OFFSET		0x00010000
#define EN7528_UBOOT_IMAGE_OFFSET	0x00060000

#define EN7528_TPL_STACK_ADDR	0xbfa3bff0
#define EN7528_TPL_SAVED_SP		0xbfa3bffc

#define EN7528_SYS_GLOBAL_PARM		0xbfb00284
#define EN7528_DRAM_SIZE_MASK		0xfff00000
#define EN7528_DRAM_SIZE_SHIFT		20
#define EN7528_SYS_CLK_MASK		0x000ffc00
#define EN7528_SYS_CLK_SHIFT		10

#endif
