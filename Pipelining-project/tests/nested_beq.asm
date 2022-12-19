	lw	1	0	ten
	lw	2	0	one
start	beq	3	1	end
	add	3	3	2
second	beq	4	1	break
	add	4	4	2
	beq	0	0	second
break	beq	0	0	start
end	halt
ten	.fill 10
one	.fill 1

