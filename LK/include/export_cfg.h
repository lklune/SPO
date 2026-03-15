#ifndef EXPORT_CFG_H
#define EXPORT_CFG_H

#include "cfg.h"

void exportCFGToDot(Function* func, const char* filepath);

void exportCallGraphToDot(FunctionCollection* functions, const char* filepath);

#endif /* EXPORT_CFG_H */