/* SPDX-License-Identifier: GPL-2.0+ */
#include "chainloader.h"
#include "../flash/sfc.h"

#define TCBOOT_FLASH_OFFSET 0x00000000U

int chainloader_flash_tcboot(const void *image, u32 len)
{
	int ret;

	/*
	 * image already points at the complete tcboot.bin staged in DRAM by
	 * xmodem_receive().  Never erase/program while XMODEM is still active:
	 * a retransmit, timeout or UART error must leave the boot flash intact.
	 */

	if (len != TCBOOT_FLASH_SIZE)
		return -EINVAL;

	ret = econet_sfc_init();
	if (ret)
		return ret;

	ret = econet_sfc_erase(TCBOOT_FLASH_OFFSET, len);
	if (ret == -EOPNOTSUPP)
		return CHAINLOADER_FLASH_UNSUPPORTED;

	if (ret)
		return ret;

	ret = econet_sfc_write(TCBOOT_FLASH_OFFSET, image, len);
	if (ret)
		return ret;

	return econet_sfc_verify(TCBOOT_FLASH_OFFSET, image, len);
}
