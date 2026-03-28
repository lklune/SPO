Общие команды

Генерация парсера:
bison -d parser/parser.y -o parser/parser.tab.c
flex -o parser/lex.yy.c parser/lexems.l
make clean
make run
./bin/run_compiler -o generated calc.src


export id=$(tt -ws -w -id -s Assemble asmListing generated/program.asm definitionFile vm.target.pdsl archName vm)
tt -ws -g "$id" -r stderr.txt
tt -ws -g "$id" -r stdout.txt
tt -ws -g "$id" -r out.ptptb > out.ptptb
tt -ws -il -s ExecuteBinaryWithInteractiveInput definitionFile vm.target.pdsl stdinRegStName sin stdoutRegStName sout archName vm binaryFileToRun out.ptptb ipRegStorageName ipst finishMnemonicName END codeRamBankName code_ram


После make run должны появляться:
- generated/input.main.dot
- generated/main.linear_code.txt

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