#ifndef EXPORT_CODE_H
#define EXPORT_CODE_H

#include "codegen.h"

const char* instructionToMnemonic(InstructionType type);

void operandToString(Operand* operand, char* buffer, int buffer_size);

void exportCompiledFunction(CompiledFunction* func, const char* filepath);

void exportAllCompiledFunctions(CompiledFunctionCollection* collection,
    const char* output_directory);

void exportCompiledFunctionsToSingleFile(CompiledFunctionCollection* collection,
    const char* filepath);

#endif // EXPORT_CODE_H