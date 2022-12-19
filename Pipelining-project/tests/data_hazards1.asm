	lw	1	0	ten
	noop
	noop
	noop
	add	2	1	1
	sw	2	0	1
	add	2	1	1
	add	3	2	2
	add	4	3	3
	lw	4	0	ten
	add	2	1	1
	beq	2	2	done
done	halt
ten	.fill	10

