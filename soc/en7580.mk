# SPDX-License-Identifier: GPL-2.0+
SOC_CROSS_COMPILE := mipsel-linux-gnu-
SOC_TRIPLE := mipsel-linux-gnu
SOC_GNU_CROSS_COMPILE := mips64-linux-gnu-
SOC_ENDIAN := little
SOC_LD_EMULATION := elf32ltsmip
SOC_UBOOT_OFFSET := 0x00060000
SOC_TCBOOT := n
SOC_RECOVERY_CHAINLOADER := n

SOC_BOOTEXT := n
