/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * move_data reconstructed from the supplied EcoNet EN7516 vendor SDK used by EN751627.
 * Ordinary MIPS32 instructions are emitted as source. MIPS MT ASE
 * opcodes unsupported by Clang IAS are kept as annotated .word values.
 * The final vendor ELF .reginfo bytes are retained so the resulting
 * raw stage remains byte-identical to the supplied SDK image.
 */

#include <asm/regdef.h>

.section .text, "ax", @progbits
.set noreorder
.set noat

.globl _ftext
_ftext:
	lui	t1,0xbfa1
	ori	t1,t1,0x114
	lw	t4,0(t1)
	andi	t4,t4,0x2
	lw	t8,0(t1)
	andi	t8,t8,0x1
	lw	t0,0(t1)
	andi	t0,t0,0x4
	beqz	t4,move_data_begin
	li	s5,11
	nop
	lui	t1,0xbfa4
	ori	t1,t1,0x20
	lw	s5,0(t1)
	bnez	s5,move_data_begin
	nop
	nop
	lui	t1,0xbfa4
	ori	t1,t1,0x200
	sw	s1,0(t1)
	sw	s2,4(t1)
	sw	s3,8(t1)
	sw	s4,12(t1)
	li	s5,11
page_size_autosizing:
	lui	s1,0xbfc0
	ori	s1,s1,0xffb0
	lui	s2,0xbfc0
	ori	s2,s2,0xffb3
	lui	s3,0xbfa4
	ori	s3,s3,0x24
	lui	s4,0x9fa4
	b	move_data_begin
	addiu	s4,s4,904
	nop
chk_page_size_magic:
	lui	t3,0xbfa4
	ori	t3,t3,0x24
	lw	t1,0(t3)
	lui	t2,0x5041
	ori	t2,t2,0x4745
	beq	t1,t2,page_size_done
	nop
	nop
	li	t1,13
	addiu	s5,s5,1
	slt	AT,t1,s5
	bnez	AT,page_size_default
	nop
	b	page_size_autosizing
	nop
	nop
page_size_default:
	li	s5,11
page_size_done:
	lui	t1,0xbfa4
	ori	t1,t1,0x20
	sw	s5,0(t1)
	lui	t1,0xbfa4
	ori	t1,t1,0x200
	lw	s1,0(t1)
	lw	s2,4(t1)
	lw	s3,8(t1)
	lw	s4,12(t1)
move_data_begin:
	li	s6,1
	sllv	s6,s6,s5
	addi	s6,s6,-1
Init_SFC:
	lui	t1,0xbfa1
	ori	t1,t1,0x9c
	li	t2,9
	sw	t2,0(t1)
	lui	t1,0xbfa1
	ori	t1,t1,0x4
	sw	zero,0(t1)
fsm_idle:
	lui	t1,0xbfa1
	ori	t1,t1,0x18
	lw	t7,0(t1)
	bnez	t7,fsm_idle
	nop
sf_mtx:
	lui	t1,0xbfa1
	ori	t1,t1,0x14
	li	t2,9
	sw	t2,0(t1)
macmux_sel:
	lui	t1,0xbfa1
	ori	t1,t1,0x1c
	li	t2,1
	sw	t2,0(t1)
	lui	t1,0xbfa1
	ori	t1,t1,0x20
	li	t2,1
	sw	t2,0(t1)
START_LOAD:
	beqz	t4,READ_DATA
	nop
	nop
LOAD_PAGE:
	slt	AT,s2,s1
	bnez	AT,LOAD_DONE
	nop
	nop
	li	t5,1
	jal	send_opfifo_write_cmd
	li	t7,1
	nop
	li	t5,8
	jal	send_opfifo_write_cmd
	li	t7,1
	nop
	jal	send_datafifo_write_cmd
	li	t5,19
	nop
PAGE_NUMBER:
	lui	t5,0xbfc0
	subu	t6,s1,t5
	srlv	t6,t6,s5
	li	t5,8
	jal	send_opfifo_write_cmd
	li	t7,3
	nop
	jal	send_datafifo_write_cmd
	srl	t5,t6,0x10
	nop
	jal	send_datafifo_write_cmd
	srl	t5,t6,0x8
	nop
	jal	send_datafifo_write_cmd
	andi	t5,t6,0xff
	nop
	li	t5,0
	jal	send_opfifo_write_cmd
	li	t7,1
	nop
	li	t5,2
	jal	send_opfifo_write_cmd
	li	t7,5
	nop
GET_FEATURE:
	li	t5,1
	jal	send_opfifo_write_cmd
	li	t7,1
	nop
	li	t5,8
	jal	send_opfifo_write_cmd
	li	t7,1
	nop
	jal	send_datafifo_write_cmd
	li	t5,15
	nop
	li	t5,8
	jal	send_opfifo_write_cmd
	li	t7,1
	nop
	jal	send_datafifo_write_cmd
	li	t5,192
	nop
	li	t5,12
	jal	send_opfifo_write_cmd
	li	t7,1
	jal	send_datafifo_read_cmd
	nop
	nop
	andi	t9,t5,0x1
	li	t5,0
	jal	send_opfifo_write_cmd
	li	t7,1
	nop
	li	t5,2
	jal	send_opfifo_write_cmd
	li	t7,5
	bnez	t9,GET_FEATURE
	nop
	nop
READ_DATA:
	slt	AT,s2,s1
	bnez	AT,LOAD_DONE
	nop
	nop
	li	t5,1
	jal	send_opfifo_write_cmd
	li	t7,1
	nop
	lui	t5,0xbfc0
	beqz	t4,ADDR_OPCODE
	subu	t9,s1,t5
	nop
	and	t9,t9,s6
ADDR_OPCODE:
	li	t5,8
	jal	send_opfifo_write_cmd
	li	t7,1
	nop
	jal	send_datafifo_write_cmd
	li	t5,3
	nop
SEND_ADDR:
	beqz	t8,THREE_BYTE_ADDR
	nop
	nop
	li	t5,8
	jal	send_opfifo_write_cmd
	li	t7,1
	nop
	jal	send_datafifo_write_cmd
	srl	t5,t9,0x18
	nop
THREE_BYTE_ADDR:
	li	t5,8
	jal	send_opfifo_write_cmd
	li	t7,3
	bnez	t0,DUMMY_APPEND
	nop
	nop
	jal	send_datafifo_write_cmd
	srl	t5,t9,0x10
	nop
DUMMY_APPEND:
	jal	send_datafifo_write_cmd
	srl	t5,t9,0x8
	nop
	jal	send_datafifo_write_cmd
	andi	t5,t9,0xff
	beqz	t0,read_one_byte
	nop
	nop
	jal	send_datafifo_write_cmd
	li	t5,0
	nop
read_one_byte:
	slt	AT,s2,s1
	bnez	AT,LOAD_DONE
	nop
	nop
	li	t5,12
	jal	send_opfifo_write_cmd
	li	t7,1
	jal	send_datafifo_read_cmd
	nop
	nop
	sb	t5,0(s3)
	addiu	s1,s1,1
	addiu	s3,s3,1
	addiu	t9,t9,1
	and	t5,t9,s6
	bnez	t5,read_one_byte
	nop
	nop
READ_PAGE_DONE:
	li	t5,0
	jal	send_opfifo_write_cmd
	li	t7,1
	nop
	li	t5,2
	jal	send_opfifo_write_cmd
	li	t7,5
	j	START_LOAD
	nop
	nop
LOAD_DONE:
	li	t5,0
	jal	send_opfifo_write_cmd
	li	t7,1
	nop
	li	t5,2
	jal	send_opfifo_write_cmd
	li	t7,5
	jr	s4
	nop
	nop
send_datafifo_read_cmd:
	lui	t1,0xbfa1
	ori	t1,t1,0x3c
	lw	t2,0(t1)
	li	t3,1
	beq	t2,t3,send_datafifo_read_cmd
	nop
	nop
	lui	t1,0xbfa1
	ori	t1,t1,0x44
	lw	t5,0(t1)
	andi	t5,t5,0xff
	lui	t1,0xbfa1
	ori	t1,t1,0x40
	li	t2,1
	jr	ra
	sw	t2,0(t1)
	nop
send_opfifo_write_cmd:
	lui	t1,0xbfa1
	ori	t1,t1,0x28
	andi	t2,t5,0x1f
	sll	t2,t2,0x9
	andi	t3,t7,0x1ff
	or	t2,t2,t3
	sw	t2,0(t1)
opfifo_full_check:
	lui	t1,0xbfa1
	ori	t1,t1,0x2c
	lw	t2,0(t1)
	bnez	t2,opfifo_full_check
	nop
	nop
	lui	t1,0xbfa1
	ori	t1,t1,0x30
	li	t2,1
	sw	t2,0(t1)
opfifo_empty_check:
	lui	t1,0xbfa1
	ori	t1,t1,0x24
	lw	t2,0(t1)
	beqz	t2,opfifo_empty_check
	nop
	jr	ra
	nop
	nop
send_datafifo_write_cmd:
	lui	t1,0xbfa1
	ori	t1,t1,0x34
	lw	t2,0(t1)
	li	t3,1
	beq	t2,t3,send_datafifo_write_cmd
	nop
	nop
	lui	t1,0xbfa1
	ori	t1,t1,0x38
	andi	t2,t5,0xff
	jr	ra
	sw	t2,0(t1)

vendor_stage_trailer:
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x83, 0x7e, 0xff, 0x02, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

.set at
