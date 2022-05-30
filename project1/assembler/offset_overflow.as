    lw 0 1 input 
    lw 0 2 cnt 
    add 0 0 3 
    lw 0 4 sub 
    lw 0 5 fAdr 
    jalr 5 6 
    halt
func    add 3 1 3 
    add 2 4 2 
    beq 0 2 38000 
    beq 0 0 func
done jalr 6   7
fAdr .fill func
cnt .fill 16
input .fill 3
sub .fill -2
