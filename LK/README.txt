1. При запуске ./run_compiler input.txt должны получаться:
- input.main.dot
- main.linear_code.txt

dot -Tpng generated/input.main.dot -o generated/main.png

2. Команды для RemoteTasks
Алиас tt:
alias tt=/mnt/c/Users/december/Desktop/SPO/lab1/LK/RemoteTasks/Portable.RemoteTasks.Manager.exe -ul 472626 -up a72ac3ca-c226-422f-bcbe-a2b70133fd16

Параметры архитектуры

- stdinRegStName = sin
- stdoutRegStName = sout
- ipRegStorageName = ipst
- finishMnemonicName = END
- codeRamBankName = code_ram


Сборка main.linear_code.txt в бинарник:
    export id=$(tt -w -id -s Assemble asmListing main.linear_code.txt definitionFile vm.target.pdsl archName vm)
    tt -g $id -r stderr.txt
    tt -g $id -r stdout.txt
    tt -g $id -r out.ptptb > out.ptptb


Запуск бинарника с input.txt:
    export id=$(tt -w -id -s ExecuteBinaryWithInput inputFile input.txt definitionFile vm.target.pdsl stdinRegStName sin stdoutRegStName sout archName vm binaryFileToRun out.ptptb ipRegStorageName ipst finishMnemonicName END codeRamBankName code_ram)
    tt -g $id -r stdout.txt
    tt -g $id -r stderr.txt


Интерактивный запуск:
    tt -il -s ExecuteBinaryWithInteractiveInput definitionFile vm.target.pdsl stdinRegStName sin stdoutRegStName sout archName vm binaryFileToRun out.ptptb ipRegStorageName ipst finishMnemonicName END codeRamBankName code_ram


Дизассемблирование:
    export id=$(tt -w -id -s Disassemble in.ptptb out.ptptb definitionFile vm.target.pdsl archName vm)
    tt -g $id -r stderr.txt
    tt -g $id -r stdout.txt
    tt -g $id -r disasmListing.txt > disasmListing.txt
