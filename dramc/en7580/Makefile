# SPDX-License-Identifier: GPL-2.0+

DRAMC_LDS := $(DRAMC_DIR)/ddr.lds
DRAMC_INCLUDES := -I$(DRAMC_DIR)

DRAMC_RECON_OBJS := \
	reconstructed/dramc.o \
	reconstructed/dramc_pi_basic_api.o \
	reconstructed/dramc_pi_calibration_api.o \
	reconstructed/dramc_pi_main.o \
	reconstructed/hal_io.o \
	reconstructed/main.o \
	reconstructed/pkgId.o \
	reconstructed/spram.o \
	reconstructed/system.o

DRAMC_OBJS := $(DRAMC_RECON_OBJS) entry.o glue.o
DRAMC_RELOC_SRCS := $(patsubst %.o,$(DRAMC_DIR)/%.S,$(DRAMC_RECON_OBJS))
DRAMC_CHECK_RELOCS := y

# LLVM's integrated MIPS assembler does not understand R_MIPS16_26 in
# .reloc. Keep the rest of an LLVM=1 build on clang/lld/llvm-objcopy and
# assemble only these reconstructed MIPS16 objects with GNU as via gcc.
DRAMC_GNU_ASM_OBJS := $(DRAMC_RECON_OBJS)
