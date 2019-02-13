	.text
main:   la	$t0,B1
	la		$s6,B2
	lw		$s4,0($s6)
	add		$t1,$s4,$t1
	add		$t1,$0,$s4
	nor		$t2,$t1,$s4
	j		main
	nor		$t4,$t7,$s3
	sll		$t2,$t3,2
	sll		$t4,$t2,6
	sll		$t3,$t2,31
	.data
B1:	.space	8
B2:	.word	16

