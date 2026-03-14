//
// export_code.h - Ёкспорт линейного кода в мнемонический формат
//

#ifndef EXPORT_CODE_H
#define EXPORT_CODE_H

#include "codegen.h"

// ѕолучить мнемоническое представление типа инструкции
const char* instructionToMnemonic(InstructionType type);

// ѕолучить строковое представление операнда
void operandToString(Operand* operand, char* buffer, int buffer_size);

// Ёкспортировать одну скомпилированную функцию в текстовый файл
void exportCompiledFunction(CompiledFunction* func, const char* filepath);

// Ёкспортировать все скомпилированные функции в отдельные файлы
void exportAllCompiledFunctions(CompiledFunctionCollection* collection,
    const char* output_directory);

// Ёкспортировать в один файл со всеми функци€ми
void exportCompiledFunctionsToSingleFile(CompiledFunctionCollection* collection,
    const char* filepath);

#endif // EXPORT_CODE_H