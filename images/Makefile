# SPDX-License-Identifier: GPL-2.0+
# OpenWrt-style image/device context registry.

DEFAULT_DEVICES :=
TARGET_DEVICES :=
TARGET_BOOTEXT :=
TARGET_TCBOOT :=
TARGET_RECOVERY :=

include $(SRCTREE)/images/en751221.mk
include $(SRCTREE)/images/en751627.mk
include $(SRCTREE)/images/en7528.mk
include $(SRCTREE)/images/en7580.mk

# Reset every context-owned variable before Device/<soc> is applied.  This is
# intentionally verbose: adding a builder variable means adding its neutral
# value here as well, which prevents state from a previously evaluated device
# from leaking into the next one.
define Device/Init
	DEVICE_SOC := $(1)
	DEVICE_DEFAULT := y
	EXPERIMENTAL := n

	SOC_CROSS_COMPILE :=
	SOC_GNU_CROSS_COMPILE :=
	SOC_TRIPLE :=
	SOC_ENDIAN := little
	SOC_LD_EMULATION :=
	SOC_PAYLOAD_OFFSET := 0x00020000
	SOC_TCBOOT := n
	SOC_RECOVERY_CHAINLOADER := n
	SOC_BOOTEXT := n

	OUT := $$(abspath $$(O))/$(1)
	UBOOT_LOAD_ADDR := 0x81000000

	DRAMC_DIR := $$(SRCTREE)/dramc/$(1)
	DRAMC_BUILD := $$(OUT)/.dramc
	DRAMC_OUTPUT := $$(OUT)/$(1)-dramc.bin
	DRAMC_LDS :=
	DRAMC_INCLUDES :=
	DRAMC_OBJS :=
	DRAMC_GNU_ASM_OBJS :=
	DRAMC_RELOC_SRCS :=
	DRAMC_CHECK_RELOCS := n
	DRAMC_PREBUILT :=
	DRAMC_PREBUILT_SIZE :=
	DDR_CPPFLAGS :=

	FLASH_STANDALONE_CPPFLAGS :=
	CHAIN_BUILD := $$(OUT)/.chainload
	CHAIN_OUTPUT := $$(OUT)/$(1)-chainload.bin
	PAYLOAD_OFFSET := $$(SOC_PAYLOAD_OFFSET)

	TCBOOT_BUILD := $$(OUT)/.tcboot
	TCBOOT_OUTPUT := $$(OUT)/tcboot.bin
	TCBOOT_SIZE_OPT := -Os
	DDR_IMAGE := $$(DRAMC_OUTPUT)
	TCBOOT_BOOT2 := n
	TCBOOT_MOVE_DATA_SRC :=
	TCBOOT_MOVE_DATA_LDS :=
	TCBOOT_BOOT2_SRC :=
	TCBOOT_BOOT2_LDS :=

	RECOVERY_BUILD := $$(OUT)/.recovery-chainloader
	RECOVERY_OUTPUT := $$(OUT)/$(1)-recovery-chainloader.bin
	CHAINLOADER_BASE :=
	CHAINLOADER_SOC_CPPFLAGS :=
	CHAINLOADER_SFC_CPPFLAGS :=
	CHAINLOADER_CPPFLAGS :=

	BOOTEXT_MODE :=
	BOOTEXT_BUILD := $$(OUT)/.bootext
	BOOTEXT_OUTPUT := $$(OUT)/bootext.bin
	BOOTEXT_DDR_WORK := $$(BOOTEXT_BUILD)/dramc-work
	BOOTEXT_CHAIN_WORK := $$(BOOTEXT_BUILD)/chainloader-work
	BOOTEXT_DDR_IMAGE := $$(BOOTEXT_BUILD)/dramc.bin
	BOOTEXT_CHAIN_IMAGE := $$(BOOTEXT_BUILD)/chainloader.bin
	BOOTEXT_DDR_ELF := $$(BOOTEXT_DDR_WORK)/.dramc/dramc.elf
	BOOTEXT_CHAIN_ELF := $$(BOOTEXT_CHAIN_WORK)/.recovery-chainloader/chainloader.elf
	BOOTEXT_BASE :=
	BOOTEXT_CHAIN_OFFSET :=
	BOOTEXT_CHAINLOADER_BASE :=
	BOOTEXT_SRAM_SIZE :=
	BOOTEXT_SRAM_LIMIT :=
	BOOTEXT_DDR_ENTRY :=
	BOOTEXT_DDR_MAX_SIZE :=
	BOOTEXT_CHAIN_MAX_SIZE :=
	BOOTEXT_SIZE_OPT := -Os
	BOOTEXT_DRAMC_CPPFLAGS :=
	BOOTEXT_CHAINLOADER_CPPFLAGS :=
endef

# Everything a builder is allowed to consume is snapshotted on its target.
# This is the same reason OpenWrt exports device variables onto generated image
# targets: later Device/<soc> evaluation must not change an earlier target.
DEVICE_VARS := \
	DEVICE_SOC DEVICE_DEFAULT EXPERIMENTAL \
	SOC_CROSS_COMPILE SOC_GNU_CROSS_COMPILE SOC_TRIPLE SOC_ENDIAN SOC_LD_EMULATION \
	SOC_PAYLOAD_OFFSET SOC_TCBOOT SOC_RECOVERY_CHAINLOADER SOC_BOOTEXT \
	OUT LLVM UBOOT_LOAD_ADDR \
	CROSS_COMPILE GNU_CROSS_COMPILE GNU_CC SIZE_OPT TARGET_ENDIAN \
	TOOL_CC TOOL_LD TOOL_OBJCOPY TOOL_NM TOOL_CC_TARGET \
	DRAMC_DIR DRAMC_BUILD DRAMC_OUTPUT DRAMC_LDS DRAMC_INCLUDES DRAMC_OBJS \
	DRAMC_GNU_ASM_OBJS DRAMC_RELOC_SRCS DRAMC_CHECK_RELOCS DRAMC_PREBUILT \
	DRAMC_PREBUILT_SIZE DDR_CPPFLAGS \
	FLASH_STANDALONE_CPPFLAGS CHAIN_BUILD CHAIN_OUTPUT PAYLOAD_OFFSET \
	TCBOOT_BUILD TCBOOT_OUTPUT TCBOOT_SIZE_OPT DDR_IMAGE TCBOOT_BOOT2 TCBOOT_MOVE_DATA_SRC \
	TCBOOT_MOVE_DATA_LDS TCBOOT_BOOT2_SRC TCBOOT_BOOT2_LDS \
	RECOVERY_BUILD RECOVERY_OUTPUT CHAINLOADER_BASE CHAINLOADER_SOC_CPPFLAGS \
	CHAINLOADER_SFC_CPPFLAGS CHAINLOADER_CPPFLAGS \
	BOOTEXT_MODE BOOTEXT_BUILD BOOTEXT_OUTPUT BOOTEXT_DDR_WORK BOOTEXT_CHAIN_WORK \
	BOOTEXT_DDR_IMAGE BOOTEXT_CHAIN_IMAGE BOOTEXT_DDR_ELF BOOTEXT_CHAIN_ELF \
	BOOTEXT_BASE BOOTEXT_CHAIN_OFFSET BOOTEXT_CHAINLOADER_BASE BOOTEXT_SRAM_SIZE \
	BOOTEXT_SRAM_LIMIT BOOTEXT_DDR_ENTRY BOOTEXT_DDR_MAX_SIZE BOOTEXT_CHAIN_MAX_SIZE \
	BOOTEXT_SIZE_OPT BOOTEXT_DRAMC_CPPFLAGS BOOTEXT_CHAINLOADER_CPPFLAGS

define Device/ExportVar
	$(eval $(1): $(2):=$($(2)))
endef

define Device/Export
	$(foreach var,$(DEVICE_VARS),$(call Device/ExportVar,$(1),$(var)))
endef

define Device/Build/Common
$(call Device/Export,$(1)-dramc)
$(call Device/Export,$(1)-chainload)

.PHONY: $(1) $(1)-dramc $(1)-chainload
$(1): $(1)-dramc $(1)-chainload

$(1)-dramc:
	$$(call dramc/compile)
	@printf '  DRAMC      %s\n' "$$(DRAMC_OUTPUT)"

$(1)-chainload:
	$$(call flash/compile)
	@printf '  CHAINLOAD  %s\n' "$$(CHAIN_OUTPUT)"
endef

define Device/Build/Recovery
$(call Device/Export,$(1)-recovery)

.PHONY: $(1)-recovery
$(1): $(1)-recovery
$(1)-recovery: $$(HOST_TOOL)
	$$(call chainload/compile)
	@printf '  RECOVERY   %s\n' "$$(RECOVERY_OUTPUT)"
endef

define Device/Build/RecoveryUnsupported
.PHONY: $(1)-recovery
$(1)-recovery:
	@echo "error: $(1) has no BootROM recovery chainloader" >&2; exit 2
endef

define Device/Build/Bootext
$(call Device/Export,$(1)-bootext)
$(call Device/Export,$(1)-bootext-dramc)
$(call Device/Export,$(1)-bootext-chainloader)

$(1)-bootext-dramc: DRAMC_BUILD := $$(BOOTEXT_DDR_WORK)/.dramc
$(1)-bootext-dramc: DRAMC_OUTPUT := $$(BOOTEXT_DDR_IMAGE)
$(1)-bootext-dramc: DDR_CPPFLAGS := $$(BOOTEXT_DRAMC_CPPFLAGS)
$(1)-bootext-dramc: SIZE_OPT := $$(BOOTEXT_SIZE_OPT)

$(1)-bootext-chainloader: RECOVERY_BUILD := $$(BOOTEXT_CHAIN_WORK)/.recovery-chainloader
$(1)-bootext-chainloader: RECOVERY_OUTPUT := $$(BOOTEXT_CHAIN_IMAGE)
$(1)-bootext-chainloader: CHAINLOADER_BASE := $$(BOOTEXT_CHAINLOADER_BASE)
$(1)-bootext-chainloader: CHAINLOADER_CPPFLAGS := $$(BOOTEXT_CHAINLOADER_CPPFLAGS)
$(1)-bootext-chainloader: SIZE_OPT := $$(BOOTEXT_SIZE_OPT)

.PHONY: $(1)-bootext $(1)-bootext-dramc $(1)-bootext-chainloader
$(1): $(1)-bootext

$(1)-bootext-dramc:
	$$(call dramc/compile)

$(1)-bootext-chainloader: $$(HOST_TOOL)
	$$(call chainload/compile)

$(1)-bootext: $(1)-bootext-dramc $(1)-bootext-chainloader $$(HOST_TOOL)
	$$(call bootext/compile)
	@printf '  BOOTEXT    %s\n' "$$(BOOTEXT_OUTPUT)"
endef

define Device/Build/BootextUnsupported
.PHONY: $(1)-bootext
$(1)-bootext:
	@echo "error: $(1) has no standalone bootext target" >&2; exit 2
endef

define Device/Build/TCBoot
$(call Device/Export,$(1)-tcboot)

.PHONY: $(1)-tcboot
$(1)-tcboot: $(1)-dramc $$(HOST_TOOL)
	$$(call tcboot/compile)
	@printf '  TCBOOT     %s\n' "$$(TCBOOT_OUTPUT)"
endef

define Device/Build/TCBootUnsupported
.PHONY: $(1)-tcboot
$(1)-tcboot:
	@echo "error: standalone tcboot packaging is not implemented for $(1)" >&2; exit 2
endef

# Registration is deliberately sequential.  The mutable parser context is
# reset, specialized, converted into a toolchain context, then immediately
# frozen onto the generated targets before moving to the next device.
define Device/Register
	$(eval $(call Device/Init,$(1)))
	$(eval $(call Device/$(1)))
	$(eval $(call compiler/prepare))
	$(eval $(call Device/Build/Common,$(1)))

	$(if $(filter y,$(SOC_RECOVERY_CHAINLOADER)),$(eval $(call Device/Build/Recovery,$(1))),$(eval $(call Device/Build/RecoveryUnsupported,$(1))))
	$(if $(filter y,$(SOC_BOOTEXT)),$(eval $(call Device/Build/Bootext,$(1))),$(eval $(call Device/Build/BootextUnsupported,$(1))))
	$(if $(filter y,$(SOC_TCBOOT)),$(eval $(call Device/Build/TCBoot,$(1))),$(eval $(call Device/Build/TCBootUnsupported,$(1))))

	$(if $(value Device/$(1)/extra),$(eval $(call Device/$(1)/extra,$(1))))

	$(if $(filter y,$(DEVICE_DEFAULT)),$(eval DEFAULT_DEVICES += $(1)))
	$(if $(filter y,$(SOC_RECOVERY_CHAINLOADER)),$(eval TARGET_RECOVERY += $(1)))
	$(if $(filter y,$(SOC_BOOTEXT)),$(eval TARGET_BOOTEXT += $(1)))
	$(if $(filter y,$(SOC_TCBOOT)),$(eval TARGET_TCBOOT += $(1)))
endef

$(foreach device,$(TARGET_DEVICES),$(call Device/Register,$(device)))

BUILD_DEVICES := $(if $(filter 1,$(EXPERIMENTAL_SOCS)),$(TARGET_DEVICES),$(DEFAULT_DEVICES))
SOCS := $(BUILD_DEVICES)
