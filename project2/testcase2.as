	lw 0 1 data1
	lw 0 2 data2
	noop
	noop
	noop
	beq 0 0 label1     # compare will finish after 3 cycles later.
	add 1 1 2          # so, we should insert 3 no-op at here. -> branch hazards occur!
  nor 3 3 3
label1 lw 0 3 data3
	halt
data1	.fill 1
data2	.fill 2
data3	.fill 3
