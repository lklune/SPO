Общие команды

make clean
make run
make vm-run
    
Генерация парсера (генерируются автоматически):
bison -d parser/parser.y -o parser/parser.tab.c
flex -o parser/lex.yy.c parser/lexems.l

После make run должны появляться:
- generated/input.main.dot
- generated/main.linear_code.txt

set PATH=$PATH:/mnt/c/Users/december/Documents/RemoteTasks

export PATH=$PATH:/mnt/c/Users/december/Documents/RemoteTasks
export Portable_RemoteTasks_Manager_Login=472626
export Portable_RemoteTasks_Manager_Password=a72ac3ca-c226-422f-bcbe-a2b70133fd16


make vm-run выполняет след команды (кроме stderr на 20260328, так как висит и ничего не выдаёт)
    alias tt="/mnt/c/Users/december/Documents/RemoteTasks/Portable.RemoteTasks.Manager.exe -ul 472626 -up a72ac3ca-c226-422f-bcbe-a2b70133fd16"
    export id=$(tt -ws -w -id -s Assemble asmListing generated/program.asm definitionFile vm.target.pdsl archName vm | head -n 1)
    tt -ws -g "$id" -r stderr.txt
    tt -ws -g "$id" -r stdout.txt
    tt -ws -g "$id" -r out.ptptb -o out.ptptb
Запуск

 калькулятора
    tt -ws -il -s ExecuteBinaryWithInteractiveInput definitionFile vm.target.pdsl stdinRegStName sin stdoutRegStName sout archName vm binaryFileToRun out.ptptb ipRegStorageName ipst finishMnemonicName END codeRamBankName code_ram


Если нужен png:
dot -Tpng generated/input.main.dot -o generated/input.main.png

RemoteTasks
Алиас tt:
alias tt="/mnt/c/Users/december/Desktop/SPO/260321/SPO/LK/RemoteTasks/Portable.RemoteTasks.Manager.exe -ul 472626 -up a72ac3ca-c226-422f-bcbe-a2b70133fd16"

Параметры архитектуры:
- stdinRegStName = sin
- stdoutRegStName = sout
- ipRegStorageName = ipst
- finishMnemonicName = END
- codeRamBankName = code_ram

Сборка generated/main.linear_code.txt в бинарник:
export id=$(tt -w -id -s Assemble asmListing generated/main.linear_code.txt definitionFile vm.target.pdsl archName vm)
tt -g "$id" -r stderr.txt
tt -g "$id" -r stdout.txt
tt -g "$id" -r out.ptptb > out.ptptb

Запуск бинарника с input.txt:
export id=$(tt -w -id -s ExecuteBinaryWithInput inputFile input.txt definitionFile vm.target.pdsl stdinRegStName sin stdoutRegStName sout archName vm binaryFileToRun out.ptptb ipRegStorageName ipst finishMnemonicName END codeRamBankName code_ram)
tt -g "$id" -r stdout.txt
tt -g "$id" -r stderr.txt

Интерактивный запуск:
tt -il -s ExecuteBinaryWithInteractiveInput definitionFile vm.target.pdsl stdinRegStName sin stdoutRegStName sout archName vm binaryFileToRun out.ptptb ipRegStorageName ipst finishMnemonicName END codeRamBankName code_ram

Дизассемблирование:
export id=$(tt -w -id -s Disassemble in.ptptb out.ptptb definitionFile vm.target.pdsl archName vm)
tt -g "$id" -r stderr.txt
tt -g "$id" -r stdout.txt
tt -g "$id" -r disasmListing.txt > disasmListing.txt

expr EQUAL EQUAL expr
$1 — левое выражение
$2 — первый =
$3 — второй =
$4 — правое выражение

MOV — копирование значения
LD — загрузить из памяти в регистр
    LD 0, r0 -- взять значение из памяти по адресу 0 и положить в r0.
    У калькулятора по адресу 0 хранится acc.

ST — сохранить из регистра в память
    ST r0, 0 -- взять значение из r0 и записать в память по адресу 0.

CMP — сравнить два значения
    Команда вычисляет разницу и кладёт её во внутренний регистр сравнения cmp.

JEQ — перейти, если равно
    Сработает, если предыдущий CMP показал равенство, то есть cmp == 0.

CALL — вызвать подпрограмму
RET — вернуться из подпрограммы


generated/program.asm -- результат работы компилятора
calculator.asm -- шаблон



stack_ram [0x00000000 .. 0x000fffff], то есть от 0 до 1048575.
1048576 = 0x100000

// Не сделано
4 лабораторная. Испектор должен поддерживать команды для:
    чтения регистров
    чтения памяти
    дизассемблирования
    выполнения одной инструкции
    продолжения выполнения
    установки/снятия breakpoint по адресу
    отображения аргументов и локальных переменных текущей функции
    выполнения до конца одного выражения
    показа текущего места в исходнике
    установки/снятия breakpoint по номеру строки
    показа стека вызовов
