# Pipeline
MIPS Pipeline Instruction Solver

This program was created for my computer architecture class. It takes
input in the following format and converts it to the output shown below:

INPUT:
```
 Address    Code        Basic                     Source

0x00400000  0x00430822  sub $1,$2,$3          7    	sub	$1, $2, $3
0x00400004  0x12120006  beq $16,$18,0x000000068    	beq	$s0, $s2, exit
0x00400008  0x8e080064  lw $8,0x00000064($16) 9    	lw	$t0, 100($s0)
0x0040000c  0x02118020  add $16,$16,$17       10   	add	$s0, $s0, $s1
0x00400010  0xae2dfffc  sw $13,0xfffffffc($17)11   	sw	$t5, -4($s1)
0x00400014  0x02b6082a  slt $1,$21,$22        12   	slt	$at, $s5, $s6
0x00400018  0x02328024  and $16,$17,$18       13   	and	$s0, $s1, $s2
0x0040001c  0x08100000  j 0x00400000          14   	j	loop
```

OUTPUT:
```
#====================
instruction_address=0x00400000
instruction_encoding=0x00430822
OPCode=000000
Funct=100010
InstrFormat=R-type
RegDst=1
Jump=0
Branch=0
MemRead=0
MemtoReg=0
MemWrite=0
ALUSrc=0
RegWrite=1
Read_register_1=00010 2
Read_register_2=00011 3
Write_register=00001 1
Sign_extend_output=0x00000822
ALU_control_output=0110
BTarget=0x0040208C
Jump_address=0x010C2088
#Continue for next instruction. Do not print this line.
#==================== 
instruction_address=0x00400004
instruction_encoding=0x12120006
...
```
