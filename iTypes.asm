	.text
main:   la	$t0,B1
	la		$s6,B2
	lw		$s4,0($t0)
	addi	$t1,$t0,42
	addi	$t2,$t0,-23
	ori		$t3,$t0,12
	ori		$t4,$t0,-2
	ori		$t4,$0,89
	j		main
	lui		$s4,87663
	lui		$s2,-123
	sw		$s2,0($s6)
	sw		$s4,4($s6)
	.data
B1:	.word	5
B2:	.space	16

