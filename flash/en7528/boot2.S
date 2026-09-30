/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * boot2 reconstructed from the supplied EcoNet EN7528 LE vendor SDK.
 * Ordinary MIPS32 instructions are emitted as source. MIPS MT ASE
 * opcodes unsupported by Clang IAS are kept as annotated .word values.
 * The final vendor ELF .reginfo bytes are retained so the resulting
 * raw stage remains byte-identical to the supplied SDK image.
 */

#include <asm/regdef.h>

.section .text, "ax", @progbits
.set noreorder
.set noat

.globl __reset_vector
__reset_vector:
	b	boot_start
	mtc0	zero,$9
boot_start:
	mfc0	a0,$12
	srl	a0,a0,0x13
	andi	a0,a0,0x1
	beqz	a0,verify_isa
	nop
	lui	k0,0xbfa0
	ori	k0,k0,0xd0
	lw	k1,0(k0)
	andi	k1,k1,0x1
	bnez	k1,rbus_timeout_handler
	nop
	lui	k0,0xbfb0
	ori	k0,k0,0x244
	lw	k1,0(k0)
	jr	k1
	nop
rbus_timeout_handler:
	lui	k0,0xbfa0
	ori	k0,k0,0xd0
	li	k1,1023
	sw	k1,8(k0)
	sw	k1,12(k0)
	sw	k1,16(k0)
	li	t7,115
	jal	disp_character
	nop
	li	t7,58
	jal	disp_character
	nop
	lui	k0,0xbfa0
	ori	k0,k0,0xd0
	li	t4,0
print_reg_val:
	lw	k1,0(k0)
	li	t2,10
	li	t3,28
print_more:
	srlv	t7,k1,t3
	andi	t7,t7,0xf
	slt	t1,t7,t2
	bnez	t1,go_print
	addiu	t7,t7,48
	addiu	t7,t7,7
go_print:
	jal	disp_character
	nop
	addiu	t3,t3,-4
	bgez	t3,print_more
	nop
	bnez	t4,rbus_timeout_loop
	nop
	li	t7,44
	jal	disp_character
	nop
	li	t7,97
	jal	disp_character
	nop
	li	t7,58
	jal	disp_character
	nop
	lui	k0,0xbfa0
	ori	k0,k0,0xd4
	li	t4,1
	b	print_reg_val
	nop
rbus_timeout_loop:
	b	rbus_timeout_loop
	nop
verify_isa:
	mfc0	a0,$15,1
	ext	s7,a0,0x0,0x4
	bnez	s7,continue_init
	nop
	li	t2,1
	li	t3,3
init_launch_flag:
	lui	t0,0xbfa4
	ori	t0,t0,0x140
	sll	t1,t2,0x5
	addu	t0,t0,t1
	li	t1,0
	sw	t1,28(t0)
	slt	t4,t2,t3
	bnez	t4,init_launch_flag
	addiu	t2,t2,1
continue_init:
	mfc0	a0,$16
	srl	a0,a0,0xa
	andi	a3,a0,0x18
	beqz	a3,is_mips32
	andi	a3,a0,0x7
	sdbbp
is_mips32:
	bnez	a3,init_vpe_resources
	nop
	sdbbp
init_vpe_resources:
	bal	init_gpr
	nop
	bal	init_cp0
	nop
	bal	check_for_tlb
	nop
	bal	init_gic
	nop
	bnez	s1,all_done
	nop
	bnez	s0,init_core_resources
	nop
init_sys_resources:
	bal	init_cpc
	nop
	bal	init_cm
	nop
	bal	init_l23u
	nop
	bal	gdmp_sram_access
	nop
	bal	release_mp
	nop
init_core_resources:
	bal	init_icache
	nop
	bal	init_dcache
	nop
	bal	init_l23c
	nop
	bal	join_domain
	nop
	bal	init_vpe1
	nop
all_done:
	mfc0	t2,$16
	beqz	v1,set_kseg0_cca
	li	t3,3
	li	t3,5
set_kseg0_cca:
	ins	t2,t3,0x0,0x3
	ins	t2,t3,0x19,0x3
	ins	t2,t3,0x1c,0x3
	mtc0	t2,$16
	bnez	s7,busy_waiting
	nop
	li	t1,1
	addiu	t1,t1,1
	addiu	t1,t1,2
	lui	t0,0xbfa4
	ori	t0,t0,0x140
	sw	t1,28(t0)
	j	continue_ecnt_booting
	nop
busy_waiting:
	lui	t9,0xbfa4
	ori	t9,t9,0x40
	jr	t9
	nop
waitcode_start:
	lui	t0,0xbfa4
	ori	t0,t0,0x140
	sll	t1,s7,0x5
	addu	t0,t0,t1
	lw	t1,28(t0)
	andi	t1,t1,0x0
	ori	t1,t1,0x1
	sw	t1,28(t0)
waitcode_start_plus_0x20:
	mfc0	t1,$9
	addiu	t1,t1,10000
swwait:
	mfc0	t4,$9
	subu	t4,t4,t1
	bltz	t4,swwait
	nop
checklaunch:
	lw	t1,28(t0)
	andi	t1,t1,0x2
	beqz	t1,waitcode_start_plus_0x20
	nop
	lw	t2,0(t0)
	lw	gp,4(t0)
	lw	sp,8(t0)
	lw	a0,12(t0)
	addu	a1,zero,zero
	addu	a2,zero,zero
	addu	a3,zero,zero
	ori	t1,t1,0x4
	jr	t2
	sw	t1,28(t0)
init_gpr:
	li	AT,0
	mfc0	sp,$12,2
	ext	s8,sp,0x1a,0x4
next_shadow_set:
	ins	sp,s8,0x6,0x4
	mtc0	sp,$12,2
	.word	0x41c10800	/* wrpgpr AT,AT */
	.word	0x41c11000	/* wrpgpr v0,AT */
	.word	0x41c11800	/* wrpgpr v1,AT */
	.word	0x41c12000	/* wrpgpr a0,AT */
	.word	0x41c12800	/* wrpgpr a1,AT */
	.word	0x41c13000	/* wrpgpr a2,AT */
	.word	0x41c13800	/* wrpgpr a3,AT */
	.word	0x41c14000	/* wrpgpr t0,AT */
	.word	0x41c14800	/* wrpgpr t1,AT */
	.word	0x41c15000	/* wrpgpr t2,AT */
	.word	0x41c15800	/* wrpgpr t3,AT */
	.word	0x41c16000	/* wrpgpr t4,AT */
	.word	0x41c16800	/* wrpgpr t5,AT */
	.word	0x41c17000	/* wrpgpr t6,AT */
	.word	0x41c17800	/* wrpgpr t7,AT */
	.word	0x41c18000	/* wrpgpr s0,AT */
	.word	0x41c18800	/* wrpgpr s1,AT */
	.word	0x41c19000	/* wrpgpr s2,AT */
	.word	0x41c19800	/* wrpgpr s3,AT */
	.word	0x41c1a000	/* wrpgpr s4,AT */
	.word	0x41c1a800	/* wrpgpr s5,AT */
	.word	0x41c1b000	/* wrpgpr s6,AT */
	.word	0x41c1b800	/* wrpgpr s7,AT */
	.word	0x41c1c000	/* wrpgpr t8,AT */
	.word	0x41c1c800	/* wrpgpr t9,AT */
	.word	0x41c1d000	/* wrpgpr k0,AT */
	.word	0x41c1d800	/* wrpgpr k1,AT */
	.word	0x41c1e000	/* wrpgpr gp,AT */
	.word	0x41c1e800	/* wrpgpr sp,AT */
	beqz	s8,set_gpr_boot_values
	.word	0x41c1f000	/* wrpgpr s8,AT */
	.word	0x41c1f800	/* wrpgpr ra,AT */
	b	next_shadow_set
	addi	s8,s8,-1
set_gpr_boot_values:
	li	AT,-1
	mfc0	a0,$15,1
	ext	s7,a0,0x0,0x4
	lui	sp,0x80a0
	ins	sp,s7,0x16,0x3
check_mt_ase:
	mfc0	a0,$16,1
	bgez	a0,no_mt_ase
	mfc0	a0,$16,2
	bgez	a0,no_mt_ase
	mfc0	a0,$16,3
	andi	a0,a0,0x4
	beqz	a0,no_mt_ase
	li	v0,0
has_mt_ase:
	li	v0,1
	mfc0	a0,$2,2
	ext	s1,a0,0x0,0x4
	ext	s2,a0,0x15,0x8
	mfc0	a0,$0,2
	ext	s5,a0,0x0,0x8
	b	check_cps
	ext	s4,a0,0xa,0x4
no_mt_ase:
	li	s1,0
	li	s2,0
	li	s4,0
	li	s5,0
check_cps:
	mfc0	t9,$15
	ext	a0,t9,0x8,0x10
	li	a3,409
	beq	a3,a0,is_cps
	li	a3,417
	beq	a3,a0,is_cps
	nop
is_not_cps:
	li	v1,0
	li	s0,0
	b	done_init_gpr
	li	s3,0
is_cps:
	li	v1,1
	lui	a1,0xbf8e
	lw	a0,8(a1)
	ins	a1,zero,0x1d,0x3
	ins	a0,zero,0x0,0xf
	beq	a1,a0,gcr_found
	nop
	sdbbp
gcr_found:
	lui	s6,0xbf8e
	lw	s0,8232(s6)
	lw	a0,0(s6)
	ext	s3,a0,0x0,0x8
done_init_gpr:
	jr	ra
	nop
init_cp0:
	lui	t3,0x40
	ori	t3,t3,0x404
	mtc0	t3,$12
	mfc0	t2,$16,1
	ext	t3,t2,0x3,0x1
	beqz	t3,done_wr
	li	t3,7
	mtc0	t3,$19
	mfc0	t2,$19
	bgez	t2,done_wr
	mtc0	zero,$18
	mtc0	t3,$19,1
	mfc0	t2,$19,1
	bgez	t2,done_wr
	mtc0	zero,$18,1
	mtc0	t3,$19,2
	mfc0	t2,$19,2
	bgez	t2,done_wr
	mtc0	zero,$18,2
	mtc0	t3,$19,3
	mfc0	t2,$19,3
	bgez	t2,done_wr
	mtc0	zero,$18,3
	mtc0	t3,$19,4
	mfc0	t2,$19,4
	bgez	t2,done_wr
	mtc0	zero,$18,4
	mtc0	t3,$19,5
	mfc0	t2,$19,5
	bgez	t2,done_wr
	mtc0	zero,$18,5
	mtc0	t3,$19,6
	mfc0	t2,$19,6
	bgez	t2,done_wr
	mtc0	zero,$18,6
	mtc0	t3,$19,7
	mtc0	zero,$18,7
done_wr:
	mtc0	zero,$13
	mtc0	zero,$11
	jr	ra
	nop
check_for_tlb:
	mfc0	t3,$16
	ext	t3,t3,0x7,0x3
	li	t7,1
	bne	t3,t7,done_init_tlb
	mfc0	t2,$16,1
	beqz	v0,start_init_tlb
	nop
	beqz	s1,start_init_tlb
	mfc0	a0,$0,1
	ext	a0,a0,0x2,0x1
	bnez	a0,done_init_tlb
	nop
start_init_tlb:
	ext	t3,t2,0x19,0x6
	mtc0	zero,$2
	mtc0	zero,$3
	mtc0	zero,$5
	mtc0	zero,$6
	lui	t4,0x8000
next_tlb_entry_pair:
	ins	t4,s7,0x14,0x4
	mtc0	t3,$0
	mtc0	t4,$10
	ehb
	tlbwi
	addi	t4,t4,16384
	bnez	t3,next_tlb_entry_pair
	addi	t3,t3,-1
done_init_tlb:
	jr	ra
	nop
init_cpc:
	beqz	v1,done_init_cpc
	nop
	lw	a0,240(s6)
	andi	a0,a0,0x1
	beqz	a0,done_init_cpc
	addu	s8,zero,zero
	lui	a0,0x1f8e
	ori	a0,a0,0x8001
	sw	a0,136(s6)
	lui	s8,0xbf8e
	ori	s8,s8,0x8000
done_init_cpc:
	jr	ra
	nop
init_gic:
	beqz	v1,done_gic
	nop
	lui	a1,0xbf8e
	ori	a1,a1,0xd0
	lw	a0,0(a1)
	ext	a0,a0,0x0,0x1
	beqz	a0,done_gic
	nop
	bnez	s7,init_vpe_gic
	nop
	lui	a1,0xbf8e
	ori	a1,a1,0x80
	lui	a0,0x1f8c
	ori	a0,a0,0x1
	sw	a0,0(a1)
	nop
	lui	a1,0xbf8c
	lw	a0,0(a1)
	ext	a0,a0,0x10,0x8
	li	a3,7
	beq	a0,a3,configure_slices
	nop
	sdbbp
configure_slices:
	li	a0,-1
	sw	a0,768(a1)
	sw	a0,772(a1)
	li	a0,0
	sw	a0,384(a1)
	lui	a0,0xff80
	sw	a0,388(a1)
	li	a0,-1
	sw	a0,256(a1)
	sw	a0,260(a1)
	lui	a0,0x8000
	sw	a0,1280(a1)
	lui	a0,0x8000
	sw	a0,1284(a1)
	lui	a0,0x8000
	ori	a0,a0,0x17
	sw	a0,1288(a1)
	lui	a0,0x8000
	ori	a0,a0,0xd
	sw	a0,1292(a1)
	lui	a0,0x8000
	ori	a0,a0,0x2
	sw	a0,1296(a1)
	lui	a0,0x8000
	ori	a0,a0,0x1f
	sw	a0,1300(a1)
	lui	a0,0x8000
	ori	a0,a0,0xf
	sw	a0,1304(a1)
	lui	a0,0x8000
	ori	a0,a0,0x19
	sw	a0,1316(a1)
	lui	a0,0x8000
	ori	a0,a0,0x15
	sw	a0,1320(a1)
	lui	a0,0x8000
	ori	a0,a0,0x14
	sw	a0,1324(a1)
	lui	a0,0x8000
	ori	a0,a0,0xe
	sw	a0,1336(a1)
	lui	a0,0x8000
	ori	a0,a0,0x11
	sw	a0,1340(a1)
	lui	a0,0x8000
	ori	a0,a0,0x10
	sw	a0,1344(a1)
	lui	a0,0x8000
	ori	a0,a0,0x18
	sw	a0,1348(a1)
	lui	a0,0x8000
	ori	a0,a0,0xc
	sw	a0,1352(a1)
	lui	a0,0x8000
	ori	a0,a0,0x1c
	sw	a0,1356(a1)
	lui	a0,0x8000
	ori	a0,a0,0x1
	sw	a0,1360(a1)
	lui	a0,0x8000
	ori	a0,a0,0x1a
	sw	a0,1364(a1)
	lui	a0,0x8000
	ori	a0,a0,0x1b
	sw	a0,1368(a1)
	lui	a0,0x8000
	ori	a0,a0,0x9
	sw	a0,1372(a1)
	lui	a0,0x8000
	ori	a0,a0,0x8
	sw	a0,1376(a1)
	lui	a0,0x8000
	ori	a0,a0,0x7
	sw	a0,1380(a1)
	lui	a0,0x8000
	ori	a0,a0,0x4
	sw	a0,1392(a1)
	lui	a0,0x8000
	ori	a0,a0,0x20
	sw	a0,1404(a1)
	lui	a0,0x8000
	sw	a0,1408(a1)
	lui	a0,0x8000
	sw	a0,1416(a1)
	lui	a0,0x8000
	sw	a0,1420(a1)
	li	a0,1
	sw	a0,8192(a1)
	sw	a0,8224(a1)
	sw	a0,8256(a1)
	sw	a0,8288(a1)
	sw	a0,8320(a1)
	sw	a0,8352(a1)
	sw	a0,8384(a1)
	sw	a0,8480(a1)
	sw	a0,8512(a1)
	sw	a0,8544(a1)
	sw	a0,8640(a1)
	sw	a0,8672(a1)
	sw	a0,8704(a1)
	sw	a0,8736(a1)
	sw	a0,8768(a1)
	sw	a0,8800(a1)
	sw	a0,8832(a1)
	sw	a0,8864(a1)
	sw	a0,8896(a1)
	sw	a0,8928(a1)
	sw	a0,8960(a1)
	sw	a0,8992(a1)
	sw	a0,9088(a1)
	sw	a0,9184(a1)
	sw	a0,9216(a1)
	sw	a0,9280(a1)
	sw	a0,9312(a1)
init_vpe_gic:
	lui	a1,0xbf8c
	ori	a1,a1,0x8000
	lw	a0,0(a1)
	ori	a0,a0,0x1
	sw	a0,0(a1)
	lui	a1,0xbf8c
	ori	a1,a1,0x8000
	lw	a3,0(a1)
map_timer_int:
	ext	a0,a3,0x1,0x1
	beqz	a0,done_gic
	nop
	lui	a0,0x8000
	ori	a0,a0,0x28
	add	a0,a0,s7
	sw	a0,72(a1)
	li	a0,127
	sw	a0,12(a1)
done_gic:
	jr	ra
	nop
gdmp_sram_access:
	lui	t0,0x9fa4
	addiu	t0,t0,4668
	lui	t1,0x9fa4
	addiu	t1,t1,4780
	lui	t2,0xbfa4
	addiu	t2,t2,64
gdmp_sram_access_plus_0x18:
	lw	t3,0(t0)
	sw	t3,0(t2)
	addiu	t0,t0,4
	addiu	t2,t2,4
	bne	t0,t1,gdmp_sram_access_plus_0x18
	nop
done_gdmp_sram_access:
	jr	ra
	nop
release_mp:
	blez	s3,done_release_mp
	li	a3,1
	beqz	s8,release_next_core
	nop
powerup_next_core:
	addu	a0,a3,zero
	sll	a0,a0,0x10
	sw	a0,8216(s6)
	lui	a0,0x9fa4
	ori	a0,a0,0x1000
	sw	a0,16416(s6)
	addu	a0,a3,zero
	sll	a0,a0,0x10
	sw	a0,8208(s8)
	li	a0,4
	sw	a0,16384(s8)
	bne	s3,a3,powerup_next_core
	addi	a3,a3,1
	jr	ra
	nop
release_next_core:
	addu	a0,a3,zero
	sll	a0,a0,0x10
	sw	a0,8216(s6)
	sw	zero,16384(s6)
	bne	s3,a3,release_next_core
	addi	a3,a3,1
done_release_mp:
	jr	ra
	nop
init_icache:
	mfc0	t2,$16,1
	ext	t3,t2,0x13,0x3
	beqz	t3,done_icache
	nop
	li	t6,2
	sllv	t3,t6,t3
	ext	t4,t2,0x16,0x3
	li	t6,64
	sllv	t4,t6,t4
	ext	t5,t2,0x10,0x3
	addi	t5,t5,1
	mul	t4,t4,t5
	lui	t6,0x8000
	mtc0	zero,$28
	mtc0	zero,$29
	addu	t7,t4,zero
next_icache_tag:
	cache	0x8,0(t6)
	addi	t7,t7,-1
	bnez	t7,next_icache_tag
	add	t6,t6,t3
done_icache:
	ins	ra,zero,0x1d,0x1
	jr	ra
	nop
init_dcache:
	ext	t3,t2,0xa,0x3
	beqz	t3,done_dcache
	nop
	li	t6,2
	sllv	t3,t6,t3
	ext	t4,t2,0xd,0x3
	li	t6,64
	sllv	t4,t6,t4
	ext	t5,t2,0x7,0x3
	addi	t5,t5,1
	mul	t4,t4,t5
	lui	t6,0x8000
	mtc0	zero,$28
	mtc0	zero,$29
	mtc0	zero,$28,2
	mtc0	zero,$29,2
	addu	t7,t4,zero
next_dcache_tag:
	cache	0x9,0(t6)
	addi	t7,t7,-1
	bnez	t7,next_dcache_tag
	add	t6,t6,t3
done_dcache:
	jr	ra
	nop
init_cm:
	beqz	v1,done_cm_init
	nop
	lui	a1,0xbfbf
	ori	a1,a1,0x8000
	li	a0,2
	sllv	a0,a0,s3
	addiu	a0,a0,-1
	sw	a0,32(a1)
	lw	a0,16(s6)
	lui	t0,0x1
	or	a0,a0,t0
	sw	a0,16(s6)
	lw	a0,0(s6)
	ext	a0,a0,0x8,0x4
	beqz	a0,done_cm_init
	lui	a0,0xffff
	sw	a0,144(a1)
	sw	a0,152(a1)
	sw	a0,160(a1)
	sw	a0,168(a1)
	sw	a0,176(a1)
	sw	a0,184(a1)
	sw	a0,192(a1)
	sw	a0,200(a1)
done_cm_init:
	jr	ra
	nop
init_itc:
	nop
done_init_itc:
	jr	ra
	nop
join_domain:
	beqz	v1,done_join_domain
	nop
	li	t1,1
	sllv	t1,t1,s3
	sll	t1,t1,0x1
	addiu	t1,t1,-1
	sw	t1,8200(s6)
	ehb
	addu	a3,zero,zero
next_coherent_core:
	sll	a0,a3,0x10
	sw	a0,8216(s6)
busy_wait_coherent_core:
	lw	a0,16392(s6)
	beqz	a0,busy_wait_coherent_core
	nop
	bne	a3,s3,next_coherent_core
	addiu	a3,a3,1
done_join_domain:
	jr	ra
	nop
init_l23u:
	beqz	v1,init_l23
	nop
	lw	a0,8(s6)
	li	a3,80
	ins	a0,a3,0x0,0x8
	sw	a0,8(s6)
	lw	a0,8(s6)
	ext	a0,a0,0x4,0x1
	bnez	a0,done_l23
	nop
	b	init_l23
	nop
init_l23c:
	beqz	v1,done_l3cache
	nop
	lw	a0,8(s6)
	bnez	s0,done_l3cache
	ext	a0,a0,0x4,0x1
	beqz	a0,done_l3cache
	nop
init_l23:
	mfc0	t2,$16,2
	ext	t3,t2,0x4,0x4
	beqz	t3,done_l2cache
	nop
	li	t6,2
	sllv	t3,t6,t3
	ext	t4,t2,0x8,0x4
	li	t6,64
	sllv	t4,t6,t4
	ext	t5,t2,0x0,0x4
	addi	t5,t5,1
	mul	t4,t4,t5
	lui	t6,0x8000
	mtc0	zero,$28,4
	mtc0	zero,$29,4
	addu	t7,t4,zero
init_l23_plus_0x40:
	cache	0xb,0(t6)
	addi	t7,t7,-1
	bnez	t7,init_l23_plus_0x40
	add	t6,t6,t3
done_l2cache:
	ext	t3,t2,0x14,0x4
	beqz	t3,done_l3cache
	nop
	li	t6,2
	sllv	t3,t6,t3
	ext	t4,t2,0x18,0x4
	li	t6,64
	sllv	t4,t6,t4
	ext	t5,t2,0x10,0x4
	addi	t5,t5,1
	mul	t4,t4,t5
	lui	t6,0x8000
	mtc0	zero,$28,4
	mtc0	zero,$29,4
	addu	t7,t4,zero
done_l2cache_plus_0x3c:
	cache	0xa,0(t6)
	addi	t7,t7,-1
	bnez	t7,done_l2cache_plus_0x3c
	add	t6,t6,t3
done_l3cache:
	beqz	v1,done_l23
	nop
	lw	a0,8(s6)
	ins	a0,zero,0x0,0x8
	sw	a0,8(s6)
done_l23:
	jr	ra
	nop
init_vpe1:
	beqz	s5,done_init_vpe1
	nop
	beqz	s4,done_init_vpe1
	nop
	mfc0	t0,$0,1
	ori	t0,t0,0x2
	mtc0	t0,$0,1
	ehb
	mfc0	t0,$0,2
	ext	t2,t0,0x0,0x8
	ext	t3,t0,0xa,0x4
	addu	t4,zero,zero
nexttc:
	mfc0	t0,$1,1
	ins	t0,t4,0x0,0x8
	mtc0	t0,$1,1
	ehb
	beqz	t4,nextvpe
	nop
	li	t0,1
	.word	0x41881004	/* mttc0 t0,c0_tchalt */
	ehb
	slt	t1,t3,t4
	bnez	t1,nexttc_plus_0x40
	addu	t1,t3,zero
	.word	0x41014002	/* mftc0 t0,c0_vpeconf0 */
	ins	t0,t4,0x15,0x8
	.word	0x41880802	/* mttc0 t0,c0_vpeconf0 */
	addu	t1,t4,zero
nexttc_plus_0x40:
	.word	0x41024002	/* mftc0 t0,c0_tcbind */
	ins	t0,t1,0x0,0x4
	.word	0x41881002	/* mttc0 t0,c0_tcbind */
	li	t0,1024
	.word	0x41881001	/* mttc0 t0,c0_tcstatus */
	.word	0x41800820	/* mttgpr zero,AT */
	.word	0x41801020	/* mttgpr zero,v0 */
	.word	0x41801820	/* mttgpr zero,v1 */
	.word	0x41802020	/* mttgpr zero,a0 */
	.word	0x41802820	/* mttgpr zero,a1 */
	.word	0x41803020	/* mttgpr zero,a2 */
	.word	0x41803820	/* mttgpr zero,a3 */
	.word	0x41804020	/* mttgpr zero,t0 */
	.word	0x41804820	/* mttgpr zero,t1 */
	.word	0x41805020	/* mttgpr zero,t2 */
	.word	0x41805820	/* mttgpr zero,t3 */
	.word	0x41806020	/* mttgpr zero,t4 */
	.word	0x41806820	/* mttgpr zero,t5 */
	.word	0x41807020	/* mttgpr zero,t6 */
	.word	0x41807820	/* mttgpr zero,t7 */
	.word	0x41808020	/* mttgpr zero,s0 */
	.word	0x41808820	/* mttgpr zero,s1 */
	.word	0x41809020	/* mttgpr zero,s2 */
	.word	0x41809820	/* mttgpr zero,s3 */
	.word	0x4180a020	/* mttgpr zero,s4 */
	.word	0x4180a820	/* mttgpr zero,s5 */
	.word	0x4180b020	/* mttgpr zero,s6 */
	.word	0x4180b820	/* mttgpr zero,s7 */
	.word	0x4180c020	/* mttgpr zero,t8 */
	.word	0x4180c820	/* mttgpr zero,t9 */
	.word	0x4180d020	/* mttgpr zero,k0 */
	.word	0x4180d820	/* mttgpr zero,k1 */
	.word	0x4180e020	/* mttgpr zero,gp */
	.word	0x4180e820	/* mttgpr zero,sp */
	.word	0x4180f020	/* mttgpr zero,s8 */
	.word	0x4180f820	/* mttgpr zero,ra */
nextvpe:
	slt	t1,t3,t4
	bnez	t1,donevpe
	nop
	.word	0x41014001	/* mftc0 t0,c0_vpecontrol */
	ins	t0,zero,0xf,0x1
	.word	0x41880801	/* mttc0 t0,c0_vpecontrol */
	beqz	t4,donevpe
	nop
	.word	0x41014002	/* mftc0 t0,c0_vpeconf0 */
	ins	t0,zero,0x0,0x1
	ori	t0,t0,0x2
	.word	0x41880802	/* mttc0 t0,c0_vpeconf0 */
	mfc0	t0,$12
	.word	0x41886000	/* mttc0 t0,c0_status */
	lui	t0,0x1234
	ori	t0,t0,0x5678
	.word	0x41887000	/* mttc0 t0,c0_epc */
	.word	0x41806800	/* mttc0 zero,c0_cause */
	mfc0	t0,$16
	.word	0x41888000	/* mttc0 t0,c0_config */
	.word	0x410f4001	/* mftc0 t0,c0_ebase */
	ext	t0,t0,0x0,0xa
	.word	0x4188b820	/* mttgpr t0,s7 */
	lui	a1,0x9fa4
	addiu	a1,a1,4096
	ins	a1,zero,0x1d,0x1
	.word	0x41851003	/* mttc0 a1,c0_tcrestart */
	.word	0x41024001	/* mftc0 t0,c0_tcstatus */
	ins	t0,zero,0xa,0x1
	ori	t0,t0,0x2000
	.word	0x41881001	/* mttc0 t0,c0_tcstatus */
	.word	0x41801004	/* mttc0 zero,c0_tchalt */
	.word	0x41014002	/* mftc0 t0,c0_vpeconf0 */
	ori	t0,t0,0x1
	.word	0x41880802	/* mttc0 t0,c0_vpeconf0 */
donevpe:
	addiu	t4,t4,1
	sltu	t1,t2,t4
	beqz	t1,nexttc
	nop
	mfc0	t0,$0,1
	ori	t0,t0,0x1
	ins	t0,zero,0x1,0x1
	mtc0	t0,$0,1
	ehb
done_init_vpe1:
	jr	ra
	nop
continue_ecnt_booting:
	lui	v0,0xbfb0
	ori	v0,v0,0x958
	lw	t6,0(v0)
	nop
	ori	t6,t6,0x1
	sw	t6,0(v0)
	nop
	lui	t1,0xbfa4
	lw	s1,8(t1)
	lw	s2,12(t1)
	lui	s3,0xbfa3
	lui	s4,0x9fa4
	addiu	s4,s4,7392
	lui	t1,0xbfa4
	ori	t1,t1,0x300
	jr	t1
	nop
move_ddr_calibration:
	nop
	lui	t1,0xbfa4
	lw	s1,8(t1)
	lw	s2,12(t1)
	subu	s2,s2,s1
	lui	k0,0x9fa3
	addu	k1,k0,s2
cache_invalidate:
	cache	0x11,0(k0)
	cache	0x10,0(k0)
	cache	0x13,0(k0)
	sync
	addiu	k0,k0,32
	slt	t1,k0,k1
	bnez	t1,cache_invalidate
	nop
	lui	k1,0x9fa4
	addiu	k1,k1,7480
	lui	k0,0xbfb0
	sw	k1,640(k0)
	lui	k0,0x9fa3
	jr	k0
	nop
ddr_calibration_done:
	li	k1,0
	lui	k0,0xbfb0
	sw	k1,640(k0)
ddr_calibration_exit:
	lui	v1,0xbfb0
	lui	a0,0x8007
	ori	a0,a0,0x1f1e
	lui	a1,0x7
	ori	a1,a1,0x1f1f
	sw	a0,32(v1)
	sw	a1,36(v1)
	nop
	lui	a2,0x8005
	sw	a2,52(v1)
	nop
	lui	v1,0xbfb1
	lui	a0,0x102d
	ori	a0,a0,0x1040
	lui	a3,0x2000
	ori	a3,a3,0x28d0
	sw	a0,0(v1)
	sw	a3,20(v1)
	nop
	lui	t1,0xbfa4
	lw	t0,24(t1)
	lui	t1,0x8001
	sw	t0,0(t1)
	lui	t1,0xbfa4
	lw	t0,28(t1)
	lui	t1,0x8001
	ori	t1,t1,0x20
	sw	t0,0(t1)
	lui	t1,0xbfa4
	lw	k0,16(t1)
	lw	k1,20(t1)
	lui	t1,0x8000
	lui	t1,0xbfa4
	lw	t2,24(t1)
	lui	t3,0x8001
	sw	t2,0(t3)
	lw	t2,28(t1)
	lui	t3,0x8001
	ori	t3,t3,0x20
	sw	t2,0(t3)
	lw	s1,16(t1)
	lw	s2,20(t1)
	lui	s3,0x8000
	lui	s4,0x9fa4
	addiu	s4,s4,7688
	lui	k0,0xbfa4
	ori	k0,k0,0x300
	jr	k0
	nop
loading_bootcode_done:
	nop
load_exit:
	lui	t1,0x8000
	lui	t2,0x8020
load_exit_plus_0x8:
	cache	0x19,0(t1)
	sync
	cache	0x10,0(t1)
	cache	0x17,0(t1)
	sync
	addiu	t1,t1,32
	bne	t2,t1,load_exit_plus_0x8
	nop
	lui	t0,0xf200
	ori	t0,t0,0xff15
	mtc0	t0,$12
	nop
wait_for_cpu1:
	lui	t0,0xbfa4
	ori	t0,t0,0x140
	li	t2,1
	sll	t1,t2,0x5
	addu	t0,t0,t1
	lw	t1,28(t0)
	ext	t1,t1,0x0,0x1
	beqz	t1,wait_for_cpu1
	nop
wait_for_cpu2:
	lui	t0,0xbfa4
	ori	t0,t0,0x140
	li	t2,2
	sll	t1,t2,0x5
	addu	t0,t0,t1
	lw	t1,28(t0)
	ext	t1,t1,0x0,0x1
	beqz	t1,wait_for_cpu2
	nop
wait_for_cpu3:
	lui	t0,0xbfa4
	ori	t0,t0,0x140
	li	t2,3
	sll	t1,t2,0x5
	addu	t0,t0,t1
	lw	t1,28(t0)
	ext	t1,t1,0x0,0x1
	beqz	t1,wait_for_cpu3
	nop
	lui	t2,0x8000
	jr	t2
	nop
disp_character:
	lui	k0,0xbfbf
wait:
	lw	t5,20(k0)
	andi	t5,t5,0x20
	beqz	t5,wait
	nop
	sw	t7,0(k0)
	jr	ra
	nop
	nop

vendor_stage_trailer:
	.byte	0xfe, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

.set at
