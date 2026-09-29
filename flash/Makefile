# SPDX-License-Identifier: GPL-2.0+
# Post-DRAM chainload and TCBoot builders.

# Compile the standalone post-DRAM flash reader into CHAIN_OUTPUT.
define flash/compile
	@rm -rf "$(CHAIN_BUILD)"
	@mkdir -p "$(CHAIN_BUILD)" "$(dir $(CHAIN_OUTPUT))"
	@printf '  CC         %s\n' loader-start.o
	$(TOOL_CC) $(TOOL_CC_TARGET) $(TARGET_ENDIAN) -mabi=32 -mips32r2 -msoft-float -mno-abicalls \
		-fno-pic -fno-pie -ffreestanding -fno-builtin -fno-stack-protector -Os -G0 -Wall \
		-I"$(SRCTREE)/include" -I"$(SRCTREE)/flash" \
		-c "$(SRCTREE)/flash/loader-start.S" -o "$(CHAIN_BUILD)/loader-start.o"
	@printf '  CC         %s\n' uart.o
	$(TOOL_CC) $(TOOL_CC_TARGET) $(TARGET_ENDIAN) -mabi=32 -mips32r2 -msoft-float -mno-abicalls \
		-fno-pic -fno-pie -ffreestanding -fno-builtin -fno-stack-protector -Os -G0 -Wall \
		-I"$(SRCTREE)/include" -I"$(SRCTREE)/flash" \
		-c "$(SRCTREE)/flash/uart.c" -o "$(CHAIN_BUILD)/uart.o"
	@printf '  CC         %s\n' loader.o
	$(TOOL_CC) $(TOOL_CC_TARGET) $(TARGET_ENDIAN) -mabi=32 -mips32r2 -msoft-float -mno-abicalls \
		-fno-pic -fno-pie -ffreestanding -fno-builtin -fno-stack-protector -Os -G0 -Wall \
		-DPAYLOAD_OFFSET=$(PAYLOAD_OFFSET) \
		-I"$(SRCTREE)/include" -I"$(SRCTREE)/flash" \
		-c "$(SRCTREE)/flash/loader.c" -o "$(CHAIN_BUILD)/loader.o"
	@printf '  CC         %s\n' early_sfc.o
	$(TOOL_CC) $(TOOL_CC_TARGET) $(TARGET_ENDIAN) -mabi=32 -mips32r2 -msoft-float -mno-abicalls \
		-fno-pic -fno-pie -ffreestanding -fno-builtin -fno-stack-protector -Os -G0 -Wall \
		$(FLASH_STANDALONE_CPPFLAGS) -I"$(SRCTREE)/include" -I"$(SRCTREE)/flash" \
		-c "$(SRCTREE)/flash/early_sfc.c" -o "$(CHAIN_BUILD)/early_sfc.o"
	@printf '  LD         %s\n' "$(CHAIN_BUILD)/chainload.elf"
	$(TOOL_LD) $(TARGET_ENDIAN) -m $(SOC_LD_EMULATION) -T "$(SRCTREE)/flash/loader.lds" \
		-Map "$(CHAIN_BUILD)/chainload.map" -o "$(CHAIN_BUILD)/chainload.elf" \
		"$(CHAIN_BUILD)/loader-start.o" "$(CHAIN_BUILD)/uart.o" "$(CHAIN_BUILD)/loader.o" "$(CHAIN_BUILD)/early_sfc.o"
	$(TOOL_OBJCOPY) -O binary "$(CHAIN_BUILD)/chainload.elf" "$(CHAIN_OUTPUT).tmp"
	@mv -f "$(CHAIN_OUTPUT).tmp" "$(CHAIN_OUTPUT)"
endef

# EN751221 has two extra first-stage blobs; other devices leave TCBOOT_BOOT2=n.
define tcboot/compile/boot2
	@printf '  CC         %s\n' move_data.o
	$(TOOL_CC) $(TOOL_CC_TARGET) $(TARGET_ENDIAN) -mabi=32 -mips32r2 -msoft-float -mno-abicalls \
		-fno-pic -fno-pie -ffreestanding -fno-builtin -fno-stack-protector $(TCBOOT_SIZE_OPT) -G0 \
		-D__ASSEMBLY__ -I"$(SRCTREE)/include" -I"$(SRCTREE)/flash" \
		-c "$(TCBOOT_MOVE_DATA_SRC)" -o "$(TCBOOT_BUILD)/move_data.o"
	@printf '  CC         %s\n' boot2.o
	$(TOOL_CC) $(TOOL_CC_TARGET) $(TARGET_ENDIAN) -mabi=32 -mips32r2 -msoft-float -mno-abicalls \
		-fno-pic -fno-pie -ffreestanding -fno-builtin -fno-stack-protector $(TCBOOT_SIZE_OPT) -G0 \
		-D__ASSEMBLY__ -I"$(SRCTREE)/include" -I"$(SRCTREE)/flash" \
		-c "$(TCBOOT_BOOT2_SRC)" -o "$(TCBOOT_BUILD)/boot2.o"
	$(TOOL_LD) $(TARGET_ENDIAN) -m $(SOC_LD_EMULATION) -G 0 -static -n -nostdlib --build-id=none \
		-T "$(TCBOOT_MOVE_DATA_LDS)" -Map "$(TCBOOT_BUILD)/move_data.map" \
		-o "$(TCBOOT_BUILD)/move_data.elf" "$(TCBOOT_BUILD)/move_data.o"
	$(TOOL_LD) $(TARGET_ENDIAN) -m $(SOC_LD_EMULATION) -G 0 -static -n -nostdlib --build-id=none \
		-T "$(TCBOOT_BOOT2_LDS)" -Map "$(TCBOOT_BUILD)/boot2.map" \
		-o "$(TCBOOT_BUILD)/boot2.elf" "$(TCBOOT_BUILD)/boot2.o"
	$(TOOL_OBJCOPY) -O binary -j .text "$(TCBOOT_BUILD)/move_data.elf" "$(TCBOOT_BUILD)/move_data.bin"
	$(TOOL_OBJCOPY) -O binary -j .text "$(TCBOOT_BUILD)/boot2.elf" "$(TCBOOT_BUILD)/boot2.bin"
	$(TOOL_OBJCOPY) \
		--add-section .move_data="$(TCBOOT_BUILD)/move_data.bin" \
		--set-section-flags .move_data=alloc,load,readonly,code \
		--add-section .boot2="$(TCBOOT_BUILD)/boot2.bin" \
		--set-section-flags .boot2=alloc,load,readonly,code \
		--add-section .loader="$(TCBOOT_BUILD)/loader.bin" \
		--set-section-flags .loader=alloc,load,readonly,data \
		--add-section .spram="$(DDR_IMAGE)" \
		--set-section-flags .spram=alloc,load,readonly,data \
		"$(TCBOOT_BUILD)/stages.o" "$(TCBOOT_BUILD)/payloads.o"
endef

define tcboot/compile/basic
	$(TOOL_OBJCOPY) \
		--add-section .loader="$(TCBOOT_BUILD)/loader.bin" \
		--set-section-flags .loader=alloc,load,readonly,data \
		--add-section .spram="$(DDR_IMAGE)" \
		--set-section-flags .spram=alloc,load,readonly,data \
		"$(TCBOOT_BUILD)/stages.o" "$(TCBOOT_BUILD)/payloads.o"
endef

# Build the standalone TCBoot component. The final payload is attached later
# by the image builder (for example OpenWrt Build/econet-tcboot).
define tcboot/compile
	@test -f "$(DDR_IMAGE)" || { echo "error: DRAMC image not found: $(DDR_IMAGE)" >&2; exit 2; }
	@rm -rf "$(TCBOOT_BUILD)"
	@mkdir -p "$(TCBOOT_BUILD)" "$(dir $(TCBOOT_OUTPUT))"
	@printf '  CC         %s\n' loader-start.o
	$(TOOL_CC) $(TOOL_CC_TARGET) $(TARGET_ENDIAN) -mabi=32 -mips32r2 -msoft-float -mno-abicalls \
		-fno-pic -fno-pie -ffreestanding -fno-builtin -fno-stack-protector $(TCBOOT_SIZE_OPT) -G0 \
		-I"$(SRCTREE)/include" -I"$(SRCTREE)/flash" \
		-c "$(SRCTREE)/flash/loader-start.S" -o "$(TCBOOT_BUILD)/loader-start.o"
	@printf '  CC         %s\n' uart.o
	$(TOOL_CC) $(TOOL_CC_TARGET) $(TARGET_ENDIAN) -mabi=32 -mips32r2 -msoft-float -mno-abicalls \
		-fno-pic -fno-pie -ffreestanding -fno-builtin -fno-stack-protector $(TCBOOT_SIZE_OPT) -G0 \
		-I"$(SRCTREE)/include" -I"$(SRCTREE)/flash" \
		-c "$(SRCTREE)/flash/uart.c" -o "$(TCBOOT_BUILD)/uart.o"
	@printf '  CC         %s\n' loader.o
	$(TOOL_CC) $(TOOL_CC_TARGET) $(TARGET_ENDIAN) -mabi=32 -mips32r2 -msoft-float -mno-abicalls \
		-fno-pic -fno-pie -ffreestanding -fno-builtin -fno-stack-protector $(TCBOOT_SIZE_OPT) -G0 \
		-DPAYLOAD_OFFSET=$(PAYLOAD_OFFSET) -I"$(SRCTREE)/include" -I"$(SRCTREE)/flash" \
		-c "$(SRCTREE)/flash/loader.c" -o "$(TCBOOT_BUILD)/loader.o"
	@printf '  CC         %s\n' flash.o
	$(TOOL_CC) $(TOOL_CC_TARGET) $(TARGET_ENDIAN) -mabi=32 -mips32r2 -msoft-float -mno-abicalls \
		-fno-pic -fno-pie -ffreestanding -fno-builtin -fno-stack-protector $(TCBOOT_SIZE_OPT) -G0 \
		$(FLASH_STANDALONE_CPPFLAGS) -I"$(SRCTREE)/include" -I"$(SRCTREE)/flash" \
		-c "$(SRCTREE)/flash/early_sfc.c" -o "$(TCBOOT_BUILD)/flash.o"
	$(TOOL_LD) $(TARGET_ENDIAN) -m $(SOC_LD_EMULATION) -T "$(SRCTREE)/flash/loader.lds" \
		-Map "$(TCBOOT_BUILD)/loader.map" -o "$(TCBOOT_BUILD)/loader.elf" \
		"$(TCBOOT_BUILD)/loader-start.o" "$(TCBOOT_BUILD)/uart.o" "$(TCBOOT_BUILD)/loader.o" "$(TCBOOT_BUILD)/flash.o"
	$(TOOL_OBJCOPY) -O binary "$(TCBOOT_BUILD)/loader.elf" "$(TCBOOT_BUILD)/loader.bin"
	@printf '  CC         %s\n' start.o
	$(TOOL_CC) $(TOOL_CC_TARGET) $(TARGET_ENDIAN) -mabi=32 -mips32r2 -msoft-float -mno-abicalls \
		-fno-pic -fno-pie -ffreestanding -fno-builtin -fno-stack-protector $(TCBOOT_SIZE_OPT) -G0 \
		-D__ASSEMBLY__ -I"$(SRCTREE)/include" -I"$(SRCTREE)/flash" \
		-c "$(SRCTREE)/flash/$(DEVICE_SOC)/start.S" -o "$(TCBOOT_BUILD)/start.o"
	@printf '  CC         %s\n' stages.o
	$(TOOL_CC) $(TOOL_CC_TARGET) $(TARGET_ENDIAN) -mabi=32 -mips32r2 -msoft-float -mno-abicalls \
		-fno-pic -fno-pie -ffreestanding -fno-builtin -fno-stack-protector $(TCBOOT_SIZE_OPT) -G0 \
		-I"$(SRCTREE)/include" -I"$(SRCTREE)/flash" \
		-c "$(SRCTREE)/flash/$(DEVICE_SOC)/stages.S" -o "$(TCBOOT_BUILD)/stages.o"
	$(if $(filter y,$(TCBOOT_BOOT2)),$(call tcboot/compile/boot2),$(call tcboot/compile/basic))
	$(TOOL_LD) $(TARGET_ENDIAN) -m $(SOC_LD_EMULATION) -T "$(SRCTREE)/flash/flash.lds" \
		-Map "$(TCBOOT_BUILD)/flash.map" -o "$(TCBOOT_BUILD)/flash.elf" \
		"$(TCBOOT_BUILD)/start.o" "$(TCBOOT_BUILD)/payloads.o"
	$(TOOL_OBJCOPY) -O binary "$(TCBOOT_BUILD)/flash.elf" "$(TCBOOT_BUILD)/flash.bin"
	$(TOOL_NM) -n "$(TCBOOT_BUILD)/flash.elf" > "$(TCBOOT_BUILD)/flash.nm"
	"$(HOST_TOOL)" tcboot-base --soc "$(DEVICE_SOC)" \
		--stages "$(TCBOOT_BUILD)/flash.bin" --symbols "$(TCBOOT_BUILD)/flash.nm" \
		--output "$(TCBOOT_OUTPUT)"
endef
