	lw	1	0	neg1
	lw	3	0	ten
start	beq 	0	3	done
	add	3	3	1
	beq	1	1	start
done	halt
neg1	.fill -1
ten	.fill 5

