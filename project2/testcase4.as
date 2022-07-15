	lw 0 1 data1
	lw 0 2 data2
	noop
	noop
	noop
	beq 0 0 label1
	noop
	noop
	noop
	add 1 1 2
  nor 3 3 3
label1 lw 0 3 data3
	halt
data1	.fill 1
data2	.fill 2
data3	.fill 3
