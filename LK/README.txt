Пошагово для разбора исходной программы, построения CFG и генерации кода для учебной VM

Что поддерживается
-- функции
-- локальные переменные и аргументы
-- арифметические выражения
-- ветвления и циклы
-- генерация AST
-- генерация CFG
-- генерация графа вызовов
-- генерация линейного кода
-- генерация итогового program.asm
-- пользовательские типы
-- наследование полей через of
-- вложенные пользовательские типы
-- методы пользовательских типов
-- интерфейсы
-- implements
-- проверка переопределения методов по полной сигнатуре

Что нужно для сборки
-- gcc
-- make
-- bison
-- flex
-- graphviz dot для автоматического AST png


Сборка
Генерация парсера выполняется автоматически во время сборки
make rebuild          -- команда очищает старые объектные файлы, заново собирает парсер и собирает bin/run_compiler
make tests            -- проверка всех тестов
./bin/run_compiler -o generated/test_230526 test_input/test_230526 -- это тест, который давал преподаватель для проверки 5 лабораторной на интерфейсы

Основные команды
make run-input        -- прогон test_input/input.txt
make run-calc         -- компиляция calc.src и сохранение результата в generated/calc
make test             -- базовый тест пользовательских типов и наследования полей
make test-nested      -- тест вложенных пользовательских типов
make test-method      -- тест методов пользовательских типов
make test-interface   -- тест интерфейсов, implements и переопределения
make tests            -- полный прогон всех основных проверок
make vm-run           -- запуск калькулятора на VM


В результате выполнения make tests должны появиться каталоги с результатами
-- generated/input
-- generated/calc
-- generated/test_basic
-- generated/test_nested
-- generated/test_method
-- generated/test_interface


AST и картинки
После запуска компилятора автоматически создаются
-- *.ast.json
-- *.ast.dot
-- *.ast.png, если установлен graphviz dot


Дополнительно создаются
-- *.linear_code.txt
-- program.asm
-- call_graph.dot
-- sourceName.functionName.dot


Порядок такой
-- исходный текст
-- парсер строит AST
-- из AST строится CFG
-- из CFG генерируется линейный код
-- потом собирается program.asm


Куда смотреть результат
ls -la generated/test_basic
cat generated/test_basic/program.asm
cat generated/test_basic/main.linear_code.txt
cat generated/test_basic/call_graph.dot


Если нужно проверить калькулятор отдельно
make run-calc
make vm-run


Если нужно собрать png для графа вызовов вручную
dot -Tpng generated/test_interface/call_graph.dot -o generated/test_interface/call_graph.png

--------------------------  Общий блок сборки и тестирования --------------------------
make rebuild
make run-input
make run-calc
make test
make test-nested
make test-method
make test-interface
make tests
make vm-run



Справка по 5 работе
в 5 лабораторной добавлены пользовательские типы, наследование, методы, вложенные типы, интерфейсы и переопределение методов ^^