/* SPDX-License-Identifier: GPL-2.0+ */
.section .rodata.blobs, "a", @progbits
.balign 4
.globl sram_stage_start, sram_stage_end, ddr_stage_start, ddr_stage_end
sram_stage_start:
.incbin SRAM_STAGE_BIN
.balign 4
sram_stage_end:
ddr_stage_start:
.incbin DDR_STAGE_BIN
.balign 4
ddr_stage_end:
.section .note.GNU-stack,"",@progbits
