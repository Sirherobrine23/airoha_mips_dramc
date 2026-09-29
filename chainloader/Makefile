# SPDX-License-Identifier: GPL-2.0+
# BootROM/XMODEM recovery chainloader builder.

# Compile a configured recovery receiver into RECOVERY_OUTPUT.
define chainload/compile
	@rm -rf "$(RECOVERY_BUILD)"
	@mkdir -p "$(RECOVERY_BUILD)" "$(dir $(RECOVERY_OUTPUT))"
	@printf '  CC         %s\n' start.o
	$(TOOL_CC) $(TOOL_CC_TARGET) $(TARGET_ENDIAN) -mabi=32 -mips32r2 -msoft-float -mno-abicalls \
		-fno-pic -fno-pie -ffreestanding -fno-builtin -fno-stack-protector $(SIZE_OPT) -G0 -Wall \
		$(CHAINLOADER_SOC_CPPFLAGS) $(CHAINLOADER_CPPFLAGS) -I"$(SRCTREE)/include" \
		-c "$(SRCTREE)/chainloader/start.S" -o "$(RECOVERY_BUILD)/start.o"
	@printf '  CC         %s\n' uart.o
	$(TOOL_CC) $(TOOL_CC_TARGET) $(TARGET_ENDIAN) -mabi=32 -mips32r2 -msoft-float -mno-abicalls \
		-fno-pic -fno-pie -ffreestanding -fno-builtin -fno-stack-protector $(SIZE_OPT) -G0 -Wall \
		$(CHAINLOADER_SOC_CPPFLAGS) $(CHAINLOADER_CPPFLAGS) -DUBOOT_LOAD_ADDR=$(UBOOT_LOAD_ADDR) \
		-I"$(SRCTREE)/include" -c "$(SRCTREE)/chainloader/uart.c" -o "$(RECOVERY_BUILD)/uart.o"
	@printf '  CC         %s\n' chainloader.o
	$(TOOL_CC) $(TOOL_CC_TARGET) $(TARGET_ENDIAN) -mabi=32 -mips32r2 -msoft-float -mno-abicalls \
		-fno-pic -fno-pie -ffreestanding -fno-builtin -fno-stack-protector $(SIZE_OPT) -G0 -Wall \
		$(CHAINLOADER_SOC_CPPFLAGS) $(CHAINLOADER_CPPFLAGS) -DUBOOT_LOAD_ADDR=$(UBOOT_LOAD_ADDR) \
		-I"$(SRCTREE)/include" -c "$(SRCTREE)/chainloader/chainloader.c" -o "$(RECOVERY_BUILD)/chainloader.o"
	@printf '  CC         %s\n' flash-write.o
	$(TOOL_CC) $(TOOL_CC_TARGET) $(TARGET_ENDIAN) -mabi=32 -mips32r2 -msoft-float -mno-abicalls \
		-fno-pic -fno-pie -ffreestanding -fno-builtin -fno-stack-protector $(SIZE_OPT) -G0 -Wall \
		$(CHAINLOADER_SOC_CPPFLAGS) $(CHAINLOADER_CPPFLAGS) -DUBOOT_LOAD_ADDR=$(UBOOT_LOAD_ADDR) \
		-I"$(SRCTREE)/include" -I"$(SRCTREE)/flash" \
		-c "$(SRCTREE)/chainloader/flash-write.c" -o "$(RECOVERY_BUILD)/flash-write.o"
	@printf '  CC         %s\n' early_sfc.o
	$(TOOL_CC) $(TOOL_CC_TARGET) $(TARGET_ENDIAN) -mabi=32 -mips32r2 -msoft-float -mno-abicalls \
		-fno-pic -fno-pie -ffreestanding -fno-builtin -fno-stack-protector $(SIZE_OPT) -G0 -Wall \
		$(CHAINLOADER_SOC_CPPFLAGS) $(CHAINLOADER_CPPFLAGS) $(CHAINLOADER_SFC_CPPFLAGS) \
		-I"$(SRCTREE)/include" -I"$(SRCTREE)/flash" \
		-c "$(SRCTREE)/flash/early_sfc.c" -o "$(RECOVERY_BUILD)/early_sfc.o"
	@printf '  LD         %s\n' "$(RECOVERY_BUILD)/chainloader.elf"
	$(TOOL_LD) $(TARGET_ENDIAN) -m $(SOC_LD_EMULATION) --defsym CHAINLOADER_BASE=$(CHAINLOADER_BASE) \
		-T "$(SRCTREE)/chainloader/chainloader.lds" -Map "$(RECOVERY_BUILD)/chainloader.map" \
		-o "$(RECOVERY_BUILD)/chainloader.elf" \
		"$(RECOVERY_BUILD)/start.o" "$(RECOVERY_BUILD)/uart.o" \
		"$(RECOVERY_BUILD)/chainloader.o" "$(RECOVERY_BUILD)/flash-write.o" "$(RECOVERY_BUILD)/early_sfc.o"
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
