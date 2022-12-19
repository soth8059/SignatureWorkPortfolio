	lw	1	0	ten
	noop
	lw	2	1	1
	sw	2	0	1
	lw	2	1	1
	add	3	2	2
	lw	4	3	3
	lw	4	0	ten
	lw	2	1	1
	beq	2	2	done
done	halt
ten	.fill	10

