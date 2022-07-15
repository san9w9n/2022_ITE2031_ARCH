  lw 0 1 data1  # load data finish at mem stage
	lw 0 2 data2  # lod data finish at mem stage
	add 1 2 4     # Because of the Data Hazard, reg1 and reg2 are still zero. 
  noop
	noop
	halt
data1 .fill 10
data2 .fill 20
