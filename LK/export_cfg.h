/*
 * export_cfg.h
 * Экспорт CFG и графа вызовов в формат GraphViz DOT
 */

#ifndef EXPORT_CFG_H
#define EXPORT_CFG_H

#include "cfg.h"

/* Экспорт CFG одной функции в файл .dot */
void exportCFGToDot(Function* func, const char* filepath);

/* Построить и экспортировать граф вызовов всех функций */
void exportCallGraphToDot(FunctionCollection* functions, const char* filepath);

#endif /* EXPORT_CFG_H */
