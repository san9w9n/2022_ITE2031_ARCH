    lw 0 1 num
    lw 0 5 addr
    jalr 5 6
    halt
    lw 0 7 one
    add 1 7 1
    jalr 6 5
num	.fill 99
one	.fill -1
addr .fill 4
