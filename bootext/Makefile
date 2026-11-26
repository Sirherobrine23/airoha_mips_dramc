# SPDX-License-Identifier: GPL-2.0+
# BootROM recovery chainloader and SRAM boot-extension builders.

# Compile the standalone BootROM/XMODEM recovery receiver into RECOVERY_OUTPUT.
define chainload/compile
	@rm -rf "$(RECOVERY_BUILD)"
	@mkdir -p "$(RECOVERY_BUILD)" "$(dir $(RECOVERY_OUTPUT))"
	@printf '  CC         %s\n' start.o
	$(TOOL_CC) $(TOOL_CC_TARGET) $(TARGET_ENDIAN) $(MIPS_COMMON_CFLAGS) $(SIZE_OPT) -Wall \
		-ffunction-sections -fdata-sections \
		$(CHAINLOADER_SOC_CPPFLAGS) $(CHAINLOADER_CPPFLAGS) $(PROJECT_INCLUDES) \
		-I"$(SRCTREE)/bootext" -c "$(SRCTREE)/bootext/start.S" -o "$(RECOVERY_BUILD)/start.o"
	@printf '  CC         %s\n' uart.o
	$(TOOL_CC) $(TOOL_CC_TARGET) $(TARGET_ENDIAN) $(MIPS_COMMON_CFLAGS) $(SIZE_OPT) -Wall \
		-ffunction-sections -fdata-sections \
		$(CHAINLOADER_SOC_CPPFLAGS) $(CHAINLOADER_CPPFLAGS) $(PROJECT_INCLUDES) \
		-c "$(SRCTREE)/common/uart.c" -o "$(RECOVERY_BUILD)/uart.o"
	@printf '  CC         %s\n' crc32.o
	$(TOOL_CC) $(TOOL_CC_TARGET) $(TARGET_ENDIAN) $(MIPS_COMMON_CFLAGS) $(SIZE_OPT) -Wall \
		-ffunction-sections -fdata-sections \
		$(CHAINLOADER_SOC_CPPFLAGS) $(CHAINLOADER_CPPFLAGS) $(PROJECT_INCLUDES) \
		-c "$(SRCTREE)/common/crc32.c" -o "$(RECOVERY_BUILD)/crc32.o"
	@printf '  CC         %s\n' chainloader.o
	$(TOOL_CC) $(TOOL_CC_TARGET) $(TARGET_ENDIAN) $(MIPS_COMMON_CFLAGS) $(SIZE_OPT) -Wall \
		-ffunction-sections -fdata-sections \
		$(CHAINLOADER_SOC_CPPFLAGS) $(CHAINLOADER_CPPFLAGS) -DUBOOT_LOAD_ADDR=$(UBOOT_LOAD_ADDR) \
		$(PROJECT_INCLUDES) -I"$(SRCTREE)/bootext" \
		-c "$(SRCTREE)/bootext/chainloader.c" -o "$(RECOVERY_BUILD)/chainloader.o"
	@printf '  CC         %s\n' flash-write.o
	$(TOOL_CC) $(TOOL_CC_TARGET) $(TARGET_ENDIAN) $(MIPS_COMMON_CFLAGS) $(SIZE_OPT) -Wall \
		-ffunction-sections -fdata-sections \
		$(CHAINLOADER_SOC_CPPFLAGS) $(CHAINLOADER_CPPFLAGS) -DUBOOT_LOAD_ADDR=$(UBOOT_LOAD_ADDR) \
		$(PROJECT_INCLUDES) -I"$(SRCTREE)/bootext" \
		-c "$(SRCTREE)/bootext/flash-write.c" -o "$(RECOVERY_BUILD)/flash-write.o"
	@printf '  CC         %s\n' sfc.o
	$(TOOL_CC) $(TOOL_CC_TARGET) $(TARGET_ENDIAN) $(MIPS_COMMON_CFLAGS) $(SIZE_OPT) -Wall \
		-ffunction-sections -fdata-sections \
		$(CHAINLOADER_SOC_CPPFLAGS) $(CHAINLOADER_CPPFLAGS) $(CHAINLOADER_SFC_CPPFLAGS) \
		$(PROJECT_INCLUDES) -c "$(SRCTREE)/common/sfc.c" -o "$(RECOVERY_BUILD)/sfc.o"
	@set -e; for obj in $(CHAINLOADER_SOC_OBJS); do \
		printf '  CC         %s\n' "$$obj"; \
		$(TOOL_CC) $(TOOL_CC_TARGET) $(TARGET_ENDIAN) $(MIPS_COMMON_CFLAGS) $(SIZE_OPT) -Wall \
			-ffunction-sections -fdata-sections \
			$(CHAINLOADER_SOC_CPPFLAGS) $(CHAINLOADER_CPPFLAGS) $(PROJECT_INCLUDES) \
			-I"$(SRCTREE)/bootext" -c "$(SRCTREE)/bootext/$${obj%.o}.c" -o "$(RECOVERY_BUILD)/$$obj"; \
	done
	@printf '  LD         %s\n' "$(RECOVERY_BUILD)/chainloader.elf"
	$(TOOL_LD) $(TARGET_ENDIAN) -m $(SOC_LD_EMULATION) --gc-sections \
		--defsym CHAINLOADER_BASE=$(CHAINLOADER_BASE) \
		-T "$(SRCTREE)/bootext/chainloader.lds" -Map "$(RECOVERY_BUILD)/chainloader.map" \
		-o "$(RECOVERY_BUILD)/chainloader.elf" \
		"$(RECOVERY_BUILD)/start.o" "$(RECOVERY_BUILD)/uart.o" "$(RECOVERY_BUILD)/crc32.o" \
		"$(RECOVERY_BUILD)/chainloader.o" "$(RECOVERY_BUILD)/flash-write.o" "$(RECOVERY_BUILD)/sfc.o" \
		$(addprefix $(RECOVERY_BUILD)/,$(CHAINLOADER_SOC_OBJS))
	$(TOOL_OBJCOPY) -O binary "$(RECOVERY_BUILD)/chainloader.elf" "$(RECOVERY_BUILD)/chainloader.raw.bin"
	@set -eu; \
		start_hex=`$(TOOL_NM) -n "$(RECOVERY_BUILD)/chainloader.elf" | awk '$$3 == "__image_start" { print $$1; exit }'`; \
		check_hex=`$(TOOL_NM) -n "$(RECOVERY_BUILD)/chainloader.elf" | awk '$$3 == "__chk_start" { print $$1; exit }'`; \
		test -n "$$start_hex" -a -n "$$check_hex"; \
		check_off=$$((0x$$check_hex - 0x$$start_hex)); \
		"$(HOST_TOOL)" chainloader --image "$(RECOVERY_BUILD)/chainloader.raw.bin" \
			--check-offset "$$check_off" --output "$(RECOVERY_OUTPUT).tmp"; \
		mv -f "$(RECOVERY_OUTPUT).tmp" "$(RECOVERY_OUTPUT)"
endef

# Single-ELF unified bootext: links DRAMC + chainloader + shared drivers into one SRAM image.
define bootext/compile/unified
	@rm -rf "$(BOOTEXT_BUILD)"
	@mkdir -p "$(BOOTEXT_BUILD)/dramc" "$(BOOTEXT_BUILD)/chain" "$(dir $(BOOTEXT_OUTPUT))"
	$(call dramc/compile/objs,$(BOOTEXT_BUILD)/dramc,-DBOOTEXT_UNIFIED $(BOOTEXT_DRAMC_CPPFLAGS))
	@printf '  CC         %s\n' start.o
	$(TOOL_CC) $(TOOL_CC_TARGET) $(TARGET_ENDIAN) $(MIPS_COMMON_CFLAGS) $(BOOTEXT_SIZE_OPT) -Wall \
		-ffunction-sections -fdata-sections -DBOOTEXT_UNIFIED \
		$(CHAINLOADER_SOC_CPPFLAGS) $(BOOTEXT_CHAINLOADER_CPPFLAGS) $(PROJECT_INCLUDES) \
		-I"$(SRCTREE)/bootext" -c "$(SRCTREE)/bootext/start.S" -o "$(BOOTEXT_BUILD)/chain/start.o"
	@printf '  CC         %s\n' crc32.o
	$(TOOL_CC) $(TOOL_CC_TARGET) $(TARGET_ENDIAN) $(MIPS_COMMON_CFLAGS) $(BOOTEXT_SIZE_OPT) -Wall \
		-ffunction-sections -fdata-sections \
		$(CHAINLOADER_SOC_CPPFLAGS) $(BOOTEXT_CHAINLOADER_CPPFLAGS) $(PROJECT_INCLUDES) \
		-c "$(SRCTREE)/common/crc32.c" -o "$(BOOTEXT_BUILD)/chain/crc32.o"
	@printf '  CC         %s\n' chainloader.o
	$(TOOL_CC) $(TOOL_CC_TARGET) $(TARGET_ENDIAN) $(MIPS_COMMON_CFLAGS) $(BOOTEXT_SIZE_OPT) -Wall \
		-ffunction-sections -fdata-sections -DBOOTEXT_UNIFIED \
		$(CHAINLOADER_SOC_CPPFLAGS) $(BOOTEXT_CHAINLOADER_CPPFLAGS) -DUBOOT_LOAD_ADDR=$(UBOOT_LOAD_ADDR) \
		$(PROJECT_INCLUDES) -I"$(SRCTREE)/bootext" \
		-c "$(SRCTREE)/bootext/chainloader.c" -o "$(BOOTEXT_BUILD)/chain/chainloader.o"
	@printf '  CC         %s\n' flash-write.o
	$(TOOL_CC) $(TOOL_CC_TARGET) $(TARGET_ENDIAN) $(MIPS_COMMON_CFLAGS) $(BOOTEXT_SIZE_OPT) -Wall \
		-ffunction-sections -fdata-sections \
		$(CHAINLOADER_SOC_CPPFLAGS) $(BOOTEXT_CHAINLOADER_CPPFLAGS) -DUBOOT_LOAD_ADDR=$(UBOOT_LOAD_ADDR) \
		$(PROJECT_INCLUDES) -I"$(SRCTREE)/bootext" \
		-c "$(SRCTREE)/bootext/flash-write.c" -o "$(BOOTEXT_BUILD)/chain/flash-write.o"
	@printf '  CC         %s\n' sfc.o
	$(TOOL_CC) $(TOOL_CC_TARGET) $(TARGET_ENDIAN) $(MIPS_COMMON_CFLAGS) $(BOOTEXT_SIZE_OPT) -Wall \
		-ffunction-sections -fdata-sections \
		$(CHAINLOADER_SOC_CPPFLAGS) $(BOOTEXT_CHAINLOADER_CPPFLAGS) $(CHAINLOADER_SFC_CPPFLAGS) \
		$(PROJECT_INCLUDES) -c "$(SRCTREE)/common/sfc.c" -o "$(BOOTEXT_BUILD)/chain/sfc.o"
	@set -e; for obj in $(CHAINLOADER_SOC_OBJS); do \
		printf '  CC         %s\n' "$$obj"; \
		$(TOOL_CC) $(TOOL_CC_TARGET) $(TARGET_ENDIAN) $(MIPS_COMMON_CFLAGS) $(BOOTEXT_SIZE_OPT) -Wall \
			-ffunction-sections -fdata-sections \
			$(CHAINLOADER_SOC_CPPFLAGS) $(BOOTEXT_CHAINLOADER_CPPFLAGS) $(PROJECT_INCLUDES) \
			-I"$(SRCTREE)/bootext" -c "$(SRCTREE)/bootext/$${obj%.o}.c" -o "$(BOOTEXT_BUILD)/chain/$$obj"; \
	done
	@printf '  LD         %s\n' "$(BOOTEXT_BUILD)/bootext.elf"
	$(TOOL_LD) $(TARGET_ENDIAN) -m $(SOC_LD_EMULATION) --gc-sections \
		--defsym BOOTEXT_BASE=$(BOOTEXT_BASE) \
		--defsym BOOTEXT_SRAM_SIZE=$(BOOTEXT_SRAM_SIZE) \
		-T "$(SRCTREE)/bootext/bootext-unified.lds" -Map "$(BOOTEXT_BUILD)/bootext.map" \
		-o "$(BOOTEXT_BUILD)/bootext.elf" \
		$(addprefix $(BOOTEXT_BUILD)/dramc/,$(DRAMC_OBJS)) \
		"$(BOOTEXT_BUILD)/chain/start.o" "$(BOOTEXT_BUILD)/chain/crc32.o" \
		"$(BOOTEXT_BUILD)/chain/chainloader.o" "$(BOOTEXT_BUILD)/chain/flash-write.o" \
		"$(BOOTEXT_BUILD)/chain/sfc.o" \
		$(addprefix $(BOOTEXT_BUILD)/chain/,$(CHAINLOADER_SOC_OBJS))
	$(TOOL_OBJCOPY) -O binary "$(BOOTEXT_BUILD)/bootext.elf" "$(BOOTEXT_BUILD)/bootext.raw.bin"
	@set -eu; \
		start_hex=`$(TOOL_NM) -n "$(BOOTEXT_BUILD)/bootext.elf" | awk '$$3 == "__image_start" { print $$1; exit }'`; \
		check_hex=`$(TOOL_NM) -n "$(BOOTEXT_BUILD)/bootext.elf" | awk '$$3 == "__chk_start" { print $$1; exit }'`; \
		end_hex=`$(TOOL_NM) -n "$(BOOTEXT_BUILD)/bootext.elf" | awk '$$3 == "_end" { print $$1; exit }'`; \
		entry_hex=`$(TOOL_NM) -n "$(BOOTEXT_BUILD)/bootext.elf" | awk '$$3 == "ddr_entry" { print $$1; exit }'`; \
		test -n "$$start_hex" -a -n "$$check_hex" -a -n "$$end_hex" -a -n "$$entry_hex"; \
		test $$((0x$$entry_hex)) -eq $$(( $(BOOTEXT_BASE) )) || { echo 'unexpected DRAMC entry' >&2; exit 1; }; \
		test $$((0x$$end_hex)) -le $$(( $(BOOTEXT_BASE) + $(BOOTEXT_SRAM_SIZE) )) || { \
			echo 'unified bootext BSS/runtime exceeds SRAM window' >&2; exit 1; }; \
		check_off=$$((0x$$check_hex - 0x$$start_hex)); \
		"$(HOST_TOOL)" chainloader --image "$(BOOTEXT_BUILD)/bootext.raw.bin" \
			--check-offset "$$check_off" --output "$(BOOTEXT_OUTPUT).tmp"; \
		size=`wc -c < "$(BOOTEXT_OUTPUT).tmp"`; \
		test "$$size" -le $$(( $(BOOTEXT_SRAM_SIZE) )) || { \
			echo 'unified bootext binary exceeds SRAM window' >&2; exit 1; }; \
		mv -f "$(BOOTEXT_OUTPUT).tmp" "$(BOOTEXT_OUTPUT)"
endef

define bootext/compile/dramc-only
	@set -eu; \
		entry=`$(TOOL_NM) -n "$(BOOTEXT_DDR_ELF)" | awk '$$3 == "ddr_entry" { print $$1; exit }'`; \
		end=`$(TOOL_NM) -n "$(BOOTEXT_DDR_ELF)" | awk '$$3 == "_end" { print $$1; exit }'`; \
		test -n "$$entry" -a -n "$$end" || { echo 'missing EN7580 bootext symbols' >&2; exit 1; }; \
		test $$((0x$$entry)) -eq $$(( $(BOOTEXT_BASE) )) || { echo 'unexpected EN7580 bootext entry' >&2; exit 1; }; \
		test $$((0x$$end)) -le $$(( $(BOOTEXT_SRAM_LIMIT) )) || { echo 'EN7580 bootext overlaps the SRAM stack' >&2; exit 1; }; \
		test $$(wc -c < "$(BOOTEXT_DDR_IMAGE)") -le $$(( $(BOOTEXT_SRAM_LIMIT) - $(BOOTEXT_BASE) )) || { \
			echo 'EN7580 bootext binary exceeds its SRAM window' >&2; exit 1; }; \
		mkdir -p "$(dir $(BOOTEXT_OUTPUT))"; \
		cp "$(BOOTEXT_DDR_IMAGE)" "$(BOOTEXT_OUTPUT).tmp"; \
		mv -f "$(BOOTEXT_OUTPUT).tmp" "$(BOOTEXT_OUTPUT)"
endef

define bootext/check/checked-wrapper
	@set -eu; \
		ddr_end=`$(TOOL_NM) -n "$(BOOTEXT_DDR_ELF)" | awk '$$3 == "_end" { print $$1; exit }'`; \
		ddr_start=`$(TOOL_NM) -n "$(BOOTEXT_DDR_ELF)" | awk '$$3 == "start" { print $$1; exit }'`; \
		chain_end=`$(TOOL_NM) -n "$(BOOTEXT_CHAIN_ELF)" | awk '$$3 == "__image_end" { print $$1; exit }'`; \
		test -n "$$ddr_end" -a -n "$$ddr_start" -a -n "$$chain_end"; \
		test $$((0x$$ddr_start)) -eq $$(( $(BOOTEXT_DDR_ENTRY) )) || { echo 'unexpected DRAMC entry' >&2; exit 1; }; \
		test $$((0x$$ddr_end)) -le $$(( $(BOOTEXT_CHAINLOADER_BASE) )) || { echo 'DRAMC runtime overlaps SRAM receiver' >&2; exit 1; }; \
		test $$((0x$$chain_end)) -le $$(( $(BOOTEXT_SRAM_LIMIT) )) || { echo 'receiver exceeds FE SRAM window' >&2; exit 1; }; \
		test $$(wc -c < "$(BOOTEXT_DDR_IMAGE)") -le $$(( $(BOOTEXT_DDR_MAX_SIZE) )); \
		test $$(wc -c < "$(BOOTEXT_CHAIN_IMAGE)") -le $$(( $(BOOTEXT_CHAIN_MAX_SIZE) ))
endef

define bootext/compile/checked-wrapper
	$(call bootext/check/checked-wrapper)
	@mkdir -p "$(BOOTEXT_BUILD)" "$(dir $(BOOTEXT_OUTPUT))"
	@rm -f "$(BOOTEXT_BUILD)/start.o" "$(BOOTEXT_BUILD)/bootstrap.o" \
		"$(BOOTEXT_BUILD)/uart.o" "$(BOOTEXT_BUILD)/crc32.o" "$(BOOTEXT_BUILD)/blobs.o" \
		"$(BOOTEXT_BUILD)/bootstrap.elf" "$(BOOTEXT_BUILD)/bootstrap.raw.bin"
	@printf '  CC         %s\n' start.o
	$(TOOL_CC) $(TOOL_CC_TARGET) $(TARGET_ENDIAN) $(MIPS_COMMON_CFLAGS) $(BOOTEXT_SIZE_OPT) -Wall \
		-ffunction-sections -fdata-sections -I"$(SRCTREE)/bootext" \
		-c "$(SRCTREE)/bootext/start.S" -o "$(BOOTEXT_BUILD)/start.o"
	@printf '  CC         %s\n' bootstrap.o
	$(TOOL_CC) $(TOOL_CC_TARGET) $(TARGET_ENDIAN) $(MIPS_COMMON_CFLAGS) $(BOOTEXT_SIZE_OPT) -Wall \
		-ffunction-sections -fdata-sections $(PROJECT_INCLUDES) -I"$(SRCTREE)/bootext" \
		-c "$(SRCTREE)/bootext/en751221-bootstrap.c" -o "$(BOOTEXT_BUILD)/bootstrap.o"
	@printf '  CC         %s\n' uart.o
	$(TOOL_CC) $(TOOL_CC_TARGET) $(TARGET_ENDIAN) $(MIPS_COMMON_CFLAGS) $(BOOTEXT_SIZE_OPT) -Wall \
		-ffunction-sections -fdata-sections $(PROJECT_INCLUDES) \
		-c "$(SRCTREE)/common/uart.c" -o "$(BOOTEXT_BUILD)/uart.o"
	@printf '  CC         %s\n' crc32.o
	$(TOOL_CC) $(TOOL_CC_TARGET) $(TARGET_ENDIAN) $(MIPS_COMMON_CFLAGS) $(BOOTEXT_SIZE_OPT) -Wall \
		-ffunction-sections -fdata-sections $(PROJECT_INCLUDES) \
		-c "$(SRCTREE)/common/crc32.c" -o "$(BOOTEXT_BUILD)/crc32.o"
	@printf '  CC         %s\n' blobs.o
	$(TOOL_CC) $(TOOL_CC_TARGET) $(TARGET_ENDIAN) $(MIPS_COMMON_CFLAGS) $(BOOTEXT_SIZE_OPT) -Wall \
		-DSRAM_STAGE_BIN='"$(BOOTEXT_CHAIN_IMAGE)"' -DDDR_STAGE_BIN='"$(BOOTEXT_DDR_IMAGE)"' \
		-c "$(SRCTREE)/bootext/en751221-blobs.S" -o "$(BOOTEXT_BUILD)/blobs.o"
	@printf '  LD         %s\n' "$(BOOTEXT_BUILD)/bootstrap.elf"
	$(TOOL_LD) $(TARGET_ENDIAN) -m $(SOC_LD_EMULATION) --gc-sections \
		--defsym CHAINLOADER_BASE=$(BOOTEXT_BASE) \
		-T "$(SRCTREE)/bootext/chainloader.lds" -Map "$(BOOTEXT_BUILD)/bootstrap.map" \
		-o "$(BOOTEXT_BUILD)/bootstrap.elf" \
		"$(BOOTEXT_BUILD)/start.o" "$(BOOTEXT_BUILD)/bootstrap.o" \
		"$(BOOTEXT_BUILD)/uart.o" "$(BOOTEXT_BUILD)/crc32.o" "$(BOOTEXT_BUILD)/blobs.o"
	$(TOOL_OBJCOPY) -O binary "$(BOOTEXT_BUILD)/bootstrap.elf" "$(BOOTEXT_BUILD)/bootstrap.raw.bin"
	@set -eu; \
		check=`$(TOOL_NM) -n "$(BOOTEXT_BUILD)/bootstrap.elf" | awk '$$3 == "__chk_start" { print $$1; exit }'`; \
		test -n "$$check"; \
		"$(HOST_TOOL)" chainloader --image "$(BOOTEXT_BUILD)/bootstrap.raw.bin" \
			--check-offset $$((0x$$check - $(BOOTEXT_BASE))) --output "$(BOOTEXT_OUTPUT).tmp"; \
		mv -f "$(BOOTEXT_OUTPUT).tmp" "$(BOOTEXT_OUTPUT)"
endef

# BOOTEXT_MODE is part of the target-specific image context.
define bootext/compile
	$(call bootext/compile/$(BOOTEXT_MODE))
endef
