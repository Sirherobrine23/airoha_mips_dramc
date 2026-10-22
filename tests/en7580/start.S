/* SPDX-License-Identifier: GPL-2.0-or-later */
.set noreorder
.set nomips16
.globl _start
.ent _start
_start:
 lw $a0,0($sp)
 addiu $a1,$sp,4
 jal guard_test
  nop
 move $a0,$v0
 li $v0,4001
 syscall
.end _start
