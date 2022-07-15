  lw 0 1 data1
  lw 0 2 data2
  lw 0 3 data3
  lw 0 4 data4
  noop
loop beq 1 2 done
  noop
  noop
  noop
  add 3 1 1
  add 4 2 2
  beq 0 0 loop
  noop
  noop
  noop
done halt
data1 .fill 1
data2 .fill 2
data3 .fill 8
data4 .fill 7
