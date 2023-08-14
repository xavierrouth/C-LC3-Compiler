#ifndef ANALYSIS_H
#define ANALYSIS_H

#include <stdbool.h>

#include "AST.h"
#include "types.h"

void analyze_ast_node(ast_node_t root);
void analysis_exit_ast_node(ast_node_t root);
#endif
