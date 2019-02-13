	.text
B1:	lw	    $t2,0($t1)
	addi	$s0,$s0,7
	ori	    $s1,$s1,-7
	bne	    $t2,$0,B1
	bne     $s0,$0,B2
	bne     $0,$0,B3
B2: lui	    $s0,98765
	bne     $0,$0,B3
	bne     $s0,$0,B4
	ori	    $t0,$s0,0
	ori	    $s0,$s0,-4
	.data
B3:	.word	9
B4:	.space	10
