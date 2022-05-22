		lw 0 1 neg1 
		add	1	1	1 
		nor	1	0	1	
		lw	0	6	len
L1  beq	3	6	finish
		add	0	0	4
		add	0	0	5
L2	beq	4	3 pass
		add	3	5	5
		add	1	4	4	
		beq	0	0	L2
pass	sw 3 5 arr0
		add	1	3	3
		beq	0	0	L1
finish	halt
len	.fill	5
arr0 .fill 0
arr1 .fill 0
arr2 .fill 0
arr3 .fill 0
arr4 .fill 0
neg1 .fill -1
