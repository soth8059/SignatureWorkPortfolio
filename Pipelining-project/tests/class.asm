        lw	1	0	five
	lw	2	0	neg1
start	add	1	2	1	#comment
	beq	0	1	2
	beq	0	0	start
	noop
done	halt
five	.fill	5
neg1	.fill	-1
stAdd	.fill	start
