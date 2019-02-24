# Mini MIPS Assembler

An Assembler for a subset of MIPS.

**Usage:**
gcc MiniMips.c
./a.out < mipsCode.asm

**Supported Instructions:**
add, addi, nor, ori, sll, lui, sw, lw, beq, bne, j, la

**Supported Directives:**
.data, .text, .space *n* . word *w*

**Supported Registers:**
$t0-$t7, $s0-$s7, $0
