# SPDX-License-Identifier: GPL-2.0+
# DRAMC builder.  All SoC-specific state is supplied by images/<soc>.mk.

# Compile DRAMC object files into $(1) using extra CPPFLAGS $(2).
define dramc/compile/objs
	@set -eu; \
		mkdir -p "$(1)/includes"; \
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
		} > "$(1)/includes/tmp_version.h"; \
		for obj in $(DRAMC_OBJS); do \
			stem=$${obj%.o}; \
			srcdir="$(DRAMC_DIR)"; \
			case " $(DRAMC_SHARED_OBJS) " in \
				*" $$obj "*) srcdir="$(DRAMC_SHARED_DIR)" ;; \
			esac; \
			case " $(DRAMC_COMMON_OBJS) " in \
				*" $$obj "*) srcdir="$(SRCTREE)/common" ;; \
			esac; \
			dst="$(1)/$$obj"; \
			mkdir -p "$$(dirname "$$dst")"; \
			if [ -f "$$srcdir/$$stem.c" ]; then \
				printf '  CC         %s\n' "$$obj"; \
				$(TOOL_CC) $(TOOL_CC_TARGET) $(TARGET_ENDIAN) $(MIPS_COMMON_CFLAGS) \
					$(SIZE_OPT) -fomit-frame-pointer -ffunction-sections -fdata-sections \
					-std=gnu11 $(DDR_CPPFLAGS) $(2) \
					-I"$(1)/includes" $(PROJECT_INCLUDES) $(DRAMC_INCLUDES) \
					-c "$$srcdir/$$stem.c" -o "$$dst"; \
			elif [ -f "$$srcdir/$$stem.S" ]; then \
				printf '  AS         %s\n' "$$obj"; \
				case " $(DRAMC_GNU_ASM_OBJS) " in \
					*" $$obj "*) use_gnu=y ;; \
					*) use_gnu=n ;; \
				esac; \
				if [ "$(LLVM)" = 1 ] && [ "$$use_gnu" = y ]; then \
					$(GNU_CC) $(TARGET_ENDIAN) $(MIPS_COMMON_CFLAGS) \
						$(SIZE_OPT) -D__ASSEMBLY__ $(DDR_CPPFLAGS) $(2) \
						-I"$(1)/includes" $(PROJECT_INCLUDES) $(DRAMC_INCLUDES) \
						-x assembler-with-cpp -c "$$srcdir/$$stem.S" -o "$$dst"; \
				else \
					$(TOOL_CC) $(TOOL_CC_TARGET) $(TARGET_ENDIAN) $(MIPS_COMMON_CFLAGS) \
						$(SIZE_OPT) -D__ASSEMBLY__ $(DDR_CPPFLAGS) $(2) \
						-I"$(1)/includes" $(PROJECT_INCLUDES) $(DRAMC_INCLUDES) \
						-x assembler-with-cpp -c "$$srcdir/$$stem.S" -o "$$dst"; \
				fi; \
			else \
				echo "error: missing DRAMC source for $$obj" >&2; \
				exit 1; \
			fi; \
		done
endef

# Compile a configured DRAMC payload into DRAMC_OUTPUT.
define dramc/compile
	@rm -rf "$(DRAMC_BUILD)"
	@mkdir -p "$(DRAMC_BUILD)" "$(dir $(DRAMC_OUTPUT))"
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
	$(call dramc/compile/objs,$(DRAMC_BUILD),)
	@printf '  LD         %s\n' "$(DRAMC_BUILD)/dramc.elf"
	$(TOOL_LD) $(TARGET_ENDIAN) -m $(SOC_LD_EMULATION) $(DRAMC_LDFLAGS) -T "$(DRAMC_LDS)" \
		-Map "$(DRAMC_BUILD)/dramc.map" -o "$(DRAMC_BUILD)/dramc.elf" \
		$(addprefix $(DRAMC_BUILD)/,$(DRAMC_OBJS))
	@printf '  OBJCOPY    %s\n' "$(DRAMC_OUTPUT)"
	$(TOOL_OBJCOPY) -O binary "$(DRAMC_BUILD)/dramc.elf" "$(DRAMC_OUTPUT).tmp"
	@mv -f "$(DRAMC_OUTPUT).tmp" "$(DRAMC_OUTPUT)"
endef
