#ifndef SYMTAB_H
#define SYMTAB_H

#include "ast.h"

typedef struct Symbol {
    char *name;
    ValueType type;
    int linha;
    struct Symbol *next;
} Symbol;

typedef struct Scope {
    Symbol *symbols;
    struct Scope *next;
} Scope;

typedef struct {
    Scope *top;
} SymbolTableStack;

void symtab_init(SymbolTableStack *stack);
void symtab_push_scope(SymbolTableStack *stack);
void symtab_pop_scope(SymbolTableStack *stack);
int symtab_insert(SymbolTableStack *stack, const char *name, ValueType type, int linha);
Symbol *symtab_lookup(SymbolTableStack *stack, const char *name);
void symtab_free(SymbolTableStack *stack);

#endif
