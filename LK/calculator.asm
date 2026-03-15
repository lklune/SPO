[section code_ram]

main:
MOV 1048576, sp

loop:
CALL prompt_a
CALL readint
ST r0, 0

CALL prompt_op
CALL readop
ST r0, 4
CMP r0, 113
JEQ quit

CALL prompt_b
CALL readint
ST r0, 8

LD 4, r0
CMP r0, 43
JEQ do_add
CMP r0, 45
JEQ do_sub
CMP r0, 42
JEQ do_mul
CMP r0, 47
JEQ do_div
CALL print_bad_op
JMP loop

do_add:
CALL print_result_prefix
LD 0, r0
LD 8, r1
ADD r0, r1
CALL writeint
CALL newline
JMP loop

do_sub:
CALL print_result_prefix
LD 0, r0
LD 8, r1
CMP r0, r1
JLT do_sub_negative
SUB r0, r1
CALL writeint
CALL newline
JMP loop

do_sub_negative:
MOV 45, r0
CALL writebyte
LD 8, r0
LD 0, r1
SUB r0, r1
CALL writeint
CALL newline
JMP loop

do_mul:
CALL print_result_prefix
LD 0, r0
LD 8, r1
MUL r0, r1
CALL writeint
CALL newline
JMP loop

do_div:
LD 8, r1
CMP r1, 0
JEQ do_div_zero
CALL print_result_prefix
LD 0, r0
DIV r0, r1
CALL writeint
CALL newline
JMP loop

do_div_zero:
CALL print_div_zero
JMP loop

quit:
CALL print_bye
END

readbyte:
IN r0
RET

writebyte:
OUT r0
RET

readop:
CALL readbyte
MOV r0, r1
readop_skip:
CMP r1, 32
JEQ readop_next
CMP r1, 10
JEQ readop_next
CMP r1, 13
JEQ readop_next
MOV r1, r0
RET

readop_next:
CALL readbyte
MOV r0, r1
JMP readop_skip

readint:
readint_skip:
CALL readbyte
MOV r0, r1
CMP r1, 32
JEQ readint_skip
CMP r1, 10
JEQ readint_skip
CMP r1, 13
JEQ readint_skip
CMP r1, 48
JLT readint_done
CMP r1, 57
JGT readint_done
MOV r1, r0
MOV 48, r3
SUB r0, r3

readint_done:
RET

writeint:
MOV r0, r2
CMP r2, 10
JLT writeint_one_digit

MOV r2, r1
MOV 10, r3
DIV r1, r3
MOV r1, r0
CALL printdigit

MOV r2, r0
MOV 10, r3
MOD r0, r3
CALL printdigit
RET

writeint_one_digit:
MOV r2, r0
CALL printdigit
RET

printdigit:
CMP r0, 0
JEQ digit_0
CMP r0, 1
JEQ digit_1
CMP r0, 2
JEQ digit_2
CMP r0, 3
JEQ digit_3
CMP r0, 4
JEQ digit_4
CMP r0, 5
JEQ digit_5
CMP r0, 6
JEQ digit_6
CMP r0, 7
JEQ digit_7
CMP r0, 8
JEQ digit_8
MOV 57, r0
CALL writebyte
RET

digit_0:
MOV 48, r0
CALL writebyte
RET

digit_1:
MOV 49, r0
CALL writebyte
RET

digit_2:
MOV 50, r0
CALL writebyte
RET

digit_3:
MOV 51, r0
CALL writebyte
RET

digit_4:
MOV 52, r0
CALL writebyte
RET

digit_5:
MOV 53, r0
CALL writebyte
RET

digit_6:
MOV 54, r0
CALL writebyte
RET

digit_7:
MOV 55, r0
CALL writebyte
RET

digit_8:
MOV 56, r0
CALL writebyte
RET

newline:
MOV 10, r0
CALL writebyte
RET

print_result_prefix:
MOV 61, r0
CALL writebyte
MOV 32, r0
CALL writebyte
RET

prompt_a:
MOV 97, r0
CALL writebyte
MOV 61, r0
CALL writebyte
MOV 32, r0
CALL writebyte
RET

prompt_op:
MOV 111, r0
CALL writebyte
MOV 112, r0
CALL writebyte
MOV 61, r0
CALL writebyte
MOV 32, r0
CALL writebyte
RET

prompt_b:
MOV 98, r0
CALL writebyte
MOV 61, r0
CALL writebyte
MOV 32, r0
CALL writebyte
RET

print_div_zero:
MOV 69, r0
CALL writebyte
MOV 82, r0
CALL writebyte
MOV 82, r0
CALL writebyte
MOV 79, r0
CALL writebyte
MOV 82, r0
CALL writebyte
MOV 58, r0
CALL writebyte
MOV 32, r0
CALL writebyte
MOV 100, r0
CALL writebyte
MOV 105, r0
CALL writebyte
MOV 118, r0
CALL writebyte
MOV 48, r0
CALL writebyte
CALL newline
RET

print_bad_op:
MOV 69, r0
CALL writebyte
MOV 82, r0
CALL writebyte
MOV 82, r0
CALL writebyte
MOV 79, r0
CALL writebyte
MOV 82, r0
CALL writebyte
MOV 58, r0
CALL writebyte
MOV 32, r0
CALL writebyte
MOV 111, r0
CALL writebyte
MOV 112, r0
CALL writebyte
CALL newline
RET

print_bye:
MOV 98, r0
CALL writebyte
MOV 121, r0
CALL writebyte
MOV 101, r0
CALL writebyte
CALL newline
RET
