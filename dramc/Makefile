# SPDX-License-Identifier: GPL-2.0+
# DRAMC builder.  All SoC-specific state is supplied by images/<soc>.mk.

# Compile a configured DRAMC payload into DRAMC_OUTPUT.
define dramc/compile
	@rm -rf "$(DRAMC_BUILD)"
	@mkdir -p "$(DRAMC_BUILD)/include" "$(dir $(DRAMC_OUTPUT))"
	$(if $(strip $(DRAMC_PREBUILT)),$(call dramc/compile/prebuilt),$(call dramc/compile/source))
endef

define dramc/compile/prebuilt
	@size=$$(wc -c < "$(DRAMC_PREBUILT)"); \
		test "$$size" -eq "$(DRAMC_PREBUILT_SIZE)" || { \
			echo "error: $(DEVICE_SOC) DRAMC payload has size $$size, expected $(DRAMC_PREBUILT_SIZE)" >&2; \
			exit 1; \
		}; \
		cp "$(DRAMC_PREBUILT)" "$(DRAMC_OUTPUT).tmp"; \
		mv -f "$(DRAMC_OUTPUT).tmp" "$(DRAMC_OUTPUT)"
endef

define dramc/compile/source
	@set -eu; \
		if [ "$(DRAMC_CHECK_RELOCS)" = y ]; then \
			if grep -nE '\.reloc.*,[[:space:]]*$$' $(DRAMC_RELOC_SRCS); then \
				echo "error: DRAMC source contains unresolved section relocations" >&2; \
				exit 1; \
			fi; \
		fi; \
		{ \
			printf '#define PLAIN_VERSION "%s"\n' '$(PLAIN_VERSION)'; \
			printf '#define CC_VERSION_STRING "%s"\n' "$$(LC_ALL=C $(TOOL_CC) --version | head -n 1)"; \
			printf '#define LD_VERSION_STRING "%s"\n' "$$(LC_ALL=C $(TOOL_LD) --version | head -n 1)"; \
		} > "$(DRAMC_BUILD)/include/tmp_version.h"; \
		for obj in $(DRAMC_OBJS); do \
			stem=$${obj%.o}; \
			dst="$(DRAMC_BUILD)/$$obj"; \
			mkdir -p "$$(dirname "$$dst")"; \
			if [ -f "$(DRAMC_DIR)/$$stem.c" ]; then \
				printf '  CC         %s\n' "$$obj"; \
				$(TOOL_CC) $(TOOL_CC_TARGET) $(TARGET_ENDIAN) -mabi=32 -mips32r2 -msoft-float -mno-abicalls \
					-fno-pic -fno-pie -ffreestanding -fno-builtin $(SIZE_OPT) -G0 \
					-fomit-frame-pointer -fno-stack-protector $(DDR_CPPFLAGS) \
					-I"$(DRAMC_BUILD)/include" -I"$(SRCTREE)/include" $(DRAMC_INCLUDES) \
					-c "$(DRAMC_DIR)/$$stem.c" -o "$$dst"; \
			elif [ -f "$(DRAMC_DIR)/$$stem.S" ]; then \
				printf '  AS         %s\n' "$$obj"; \
				case " $(DRAMC_GNU_ASM_OBJS) " in \
					*" $$obj "*) use_gnu=y ;; \
					*) use_gnu=n ;; \
				esac; \
				if [ "$(LLVM)" = 1 ] && [ "$$use_gnu" = y ]; then \
					$(GNU_CC) $(TARGET_ENDIAN) -mabi=32 -mips32r2 -msoft-float -mno-abicalls \
						-fno-pic -fno-pie -ffreestanding -fno-builtin $(SIZE_OPT) -G0 \
						-D__ASSEMBLY__ $(DDR_CPPFLAGS) -I"$(DRAMC_BUILD)/include" \
						-I"$(SRCTREE)/include" $(DRAMC_INCLUDES) -x assembler-with-cpp \
						-c "$(DRAMC_DIR)/$$stem.S" -o "$$dst"; \
				else \
					$(TOOL_CC) $(TOOL_CC_TARGET) $(TARGET_ENDIAN) -mabi=32 -mips32r2 -msoft-float -mno-abicalls \
						-fno-pic -fno-pie -ffreestanding -fno-builtin $(SIZE_OPT) -G0 \
						-D__ASSEMBLY__ $(DDR_CPPFLAGS) -I"$(DRAMC_BUILD)/include" \
						-I"$(SRCTREE)/include" $(DRAMC_INCLUDES) -x assembler-with-cpp \
						-c "$(DRAMC_DIR)/$$stem.S" -o "$$dst"; \
				fi; \
			else \
				echo "error: missing DRAMC source for $$obj" >&2; \
				exit 1; \
			fi; \
		done
	@printf '  LD         %s\n' "$(DRAMC_BUILD)/dramc.elf"
	$(TOOL_LD) $(TARGET_ENDIAN) -m $(SOC_LD_EMULATION) -T "$(DRAMC_LDS)" \
		-Map "$(DRAMC_BUILD)/dramc.map" -o "$(DRAMC_BUILD)/dramc.elf" \
		$(addprefix $(DRAMC_BUILD)/,$(DRAMC_OBJS))
	@printf '  OBJCOPY    %s\n' "$(DRAMC_OUTPUT)"
	$(TOOL_OBJCOPY) -O binary "$(DRAMC_BUILD)/dramc.elf" "$(DRAMC_OUTPUT).tmp"
	@mv -f "$(DRAMC_OUTPUT).tmp" "$(DRAMC_OUTPUT)"
endef
