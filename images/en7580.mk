# SPDX-License-Identifier: GPL-2.0+

define Device/en7580
	DEVICE_DEFAULT := n
	EXPERIMENTAL := y
	SOC_CROSS_COMPILE := mipsel-linux-gnu-
	SOC_GNU_CROSS_COMPILE := mips64-linux-gnu-
	SOC_TRIPLE := mipsel-linux-gnu
	SOC_ENDIAN := little
	SOC_LD_EMULATION := elf32ltsmip
	SOC_PAYLOAD_OFFSET := 0x00060000

	DRAMC_LDS := $$(DRAMC_DIR)/ddr.lds
	DRAMC_INCLUDES := -I$$(DRAMC_DIR)
	DRAMC_OBJS := \
		reconstructed/dramc.o \
		reconstructed/dramc_pi_basic_api.o \
		reconstructed/dramc_pi_calibration_api.o \
		reconstructed/dramc_pi_main.o \
		reconstructed/hal_io.o \
		reconstructed/main.o \
		reconstructed/pkgId.o \
		reconstructed/spram.o \
		reconstructed/system.o \
		entry.o \
		glue.o
	DRAMC_GNU_ASM_OBJS := \
		reconstructed/dramc.o \
		reconstructed/dramc_pi_basic_api.o \
		reconstructed/dramc_pi_calibration_api.o \
		reconstructed/dramc_pi_main.o \
		reconstructed/hal_io.o \
		reconstructed/main.o \
		reconstructed/pkgId.o \
		reconstructed/spram.o \
		reconstructed/system.o
	DRAMC_RELOC_SRCS := $$(patsubst %.o,$$(DRAMC_DIR)/%.S,$$(DRAMC_GNU_ASM_OBJS))
	DRAMC_CHECK_RELOCS := y
endef

TARGET_DEVICES += en7580
