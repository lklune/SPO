[section code_ram]
	JMP main

main:
	MOV 1048576, r3
	MOV r3, sp
	MOV 0, r0
	ST r0, 0
	CALL writeint
	CALL newline

loop:
	CALL readop
	ST r0, 4
	CMP r0, 113
	JEQ quit

	CALL readint
	ST r0, 8

	LD 4, r0
	CMP r0, 43
	JEQ do_add
	CMP r0, 45
	JEQ do_sub
	CMP r0, 42
	JEQ do_mul
	CMP r0, 102
	JEQ do_fib
	CMP r0, 47
	JEQ do_div
	CALL print_bad_op
	JMP loop

do_add:
	LD 0, r0
	LD 8, r1
	ADD r0, r1
	ST r0, 0
	CALL writeint
	CALL newline
	JMP loop

do_sub:
	LD 0, r0
	LD 8, r1
	CMP r0, r1
	JLT do_sub_negative
	SUB r0, r1
	ST r0, 0
	CALL writeint
	CALL newline
	JMP loop

do_sub_negative:
	LD 8, r0
	LD 0, r1
	SUB r0, r1
	MOV 0, r1
	ST r1, 0
	MOV 45, r1
	MOV r1, r0
	CALL writebyte
	CALL writeint
	CALL newline
	JMP loop

do_mul:
	LD 0, r0
	LD 8, r1
	MUL r0, r1
	ST r0, 0
	CALL writeint
	CALL newline
	JMP loop

do_fib:
	LD 8, r0
	CALL fib
	ST r0, 0
	CALL writeint
	CALL newline
	JMP loop

do_div:
	LD 8, r1
	CMP r1, 0
	JEQ do_div_zero
	LD 0, r0
	DIV r0, r1
	ST r0, 0
	CALL writeint
	CALL newline
	JMP loop

do_div_zero:
	CALL print_div_zero
	JMP loop

quit:
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
	MOV 0, r2
readint_loop:
	CMP r1, 48
	JEQ readint_digit_0
	CMP r1, 49
	JEQ readint_digit_1
	CMP r1, 50
	JEQ readint_digit_2
	CMP r1, 51
	JEQ readint_digit_3
	CMP r1, 52
	JEQ readint_digit_4
	CMP r1, 53
	JEQ readint_digit_5
	CMP r1, 54
	JEQ readint_digit_6
	CMP r1, 55
	JEQ readint_digit_7
	CMP r1, 56
	JEQ readint_digit_8
	CMP r1, 57
	JEQ readint_digit_9
	JMP readint_done

readint_digit_0:
	MOV r2, r0
	MOV 10, r3
	MUL r0, r3
	MOV r0, r2
	JMP readint_next

readint_digit_1:
	MOV r2, r0
	MOV 10, r3
	MUL r0, r3
	MOV r0, r2
	MOV 1, r0
	ADD r2, r0
	JMP readint_next

readint_digit_2:
	MOV r2, r0
	MOV 10, r3
	MUL r0, r3
	MOV r0, r2
	MOV 2, r0
	ADD r2, r0
	JMP readint_next

readint_digit_3:
	MOV r2, r0
	MOV 10, r3
	MUL r0, r3
	MOV r0, r2
	MOV 3, r0
	ADD r2, r0
	JMP readint_next

readint_digit_4:
	MOV r2, r0
	MOV 10, r3
	MUL r0, r3
	MOV r0, r2
	MOV 4, r0
	ADD r2, r0
	JMP readint_next

readint_digit_5:
	MOV r2, r0
	MOV 10, r3
	MUL r0, r3
	MOV r0, r2
	MOV 5, r0
	ADD r2, r0
	JMP readint_next

readint_digit_6:
	MOV r2, r0
	MOV 10, r3
	MUL r0, r3
	MOV r0, r2
	MOV 6, r0
	ADD r2, r0
	JMP readint_next

readint_digit_7:
	MOV r2, r0
	MOV 10, r3
	MUL r0, r3
	MOV r0, r2
	MOV 7, r0
	ADD r2, r0
	JMP readint_next

readint_digit_8:
	MOV r2, r0
	MOV 10, r3
	MUL r0, r3
	MOV r0, r2
	MOV 8, r0
	ADD r2, r0
	JMP readint_next

readint_digit_9:
	MOV r2, r0
	MOV 10, r3
	MUL r0, r3
	MOV r0, r2
	MOV 9, r0
	ADD r2, r0
readint_next:
	CALL readbyte
	MOV r0, r1
	JMP readint_loop

readint_done:
	MOV r2, r0
	RET

writeint:
	MOV 268435455, r3
	BIT_AND r0, r3
	PUSH r0
	MOV r0, r1
	MOV 10, r2
	DIV r1, r2
	CMP r1, 0
	JEQ writeint_ones
	MOV r1, r0
	CALL writeint
writeint_ones:
	POP r0
	MOV r0, r1
	MOV 10, r2
	MOD r1, r2
	MOV r1, r0
	CALL write_digit
	RET

write_digit:
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
	CMP r0, 9
	JEQ digit_9
	MOV 48, r0
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

digit_9:
	MOV 57, r0
	CALL writebyte
	RET

digit_10:
	MOV 49, r0
	CALL writebyte
	MOV 48, r0
	CALL writebyte
	RET

fib:
	MOV 268435455, r3
	BIT_AND r0, r3
	MOV r0, r1
	CMP r1, 1
	JEQ fib_base
	CMP r1, 2
	JEQ fib_base
	PUSH r1
	MOV r1, r0
	MOV 2, r2
	SUB r0, r2
	CALL fib
	POP r1
	PUSH r0
	MOV r1, r0
	MOV 1, r2
	SUB r0, r2
	CALL fib
	POP r1
	ADD r1, r0
	MOV r1, r0
	RET

fib_base:
	MOV 1, r0
	RET

newline:
	MOV 10, r0
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
