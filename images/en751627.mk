# SPDX-License-Identifier: GPL-2.0+

define Device/en751627
	EXPERIMENTAL := y
	SOC_CROSS_COMPILE := mips-linux-gnu-
	SOC_TRIPLE := mips-linux-gnu
	SOC_ENDIAN := big
	SOC_LD_EMULATION := elf32btsmip
	SOC_PAYLOAD_OFFSET := 0x00020000
	SOC_TCBOOT := y
	# The prebuilt DRAMC leaves little room before manufacturing data.
	TCBOOT_SIZE_OPT := -Oz

	DRAMC_PREBUILT := $$(DRAMC_DIR)/en751627_ddr.bin
	DRAMC_PREBUILT_SIZE := 53328
endef

TARGET_DEVICES += en751627
