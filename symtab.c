#define _POSIX_C_SOURCE 200809L
#include "symtab.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static char *dupstr_local(const char *s) {
    if (s == NULL) return NULL;

    char *copy = strdup(s);
    if (copy == NULL) {
        fprintf(stderr, "Erro de alocacao de memoria.\n");
        exit(1);
    }
    return copy;
}

void symtab_init(SymbolTableStack *stack) {
    if (stack == NULL) return;
    stack->top = NULL;
}

void symtab_push_scope(SymbolTableStack *stack) {
    if (stack == NULL) return;

    Scope *novo = (Scope *) malloc(sizeof(Scope));
    if (novo == NULL) {
        fprintf(stderr, "Erro de alocacao de memoria.\n");
        exit(1);
    }

    novo->symbols = NULL;
    novo->next = stack->top;
    stack->top = novo;
}

void symtab_pop_scope(SymbolTableStack *stack) {
    if (stack == NULL || stack->top == NULL) return;

    Scope *atual = stack->top;
    Symbol *s = atual->symbols;

    while (s != NULL) {
        Symbol *prox = s->next;
        free(s->name);
        free(s);
        s = prox;
    }

    stack->top = atual->next;
    free(atual);
}

int symtab_insert(SymbolTableStack *stack, const char *name, ValueType type, int linha) {
    if (stack == NULL || stack->top == NULL || name == NULL) {
        return 0;
    }

    Symbol *it = stack->top->symbols;
    while (it != NULL) {
        if (strcmp(it->name, name) == 0) {
            return 0; /* ja declarado no escopo atual */
        }
        it = it->next;
    }

    Symbol *novo = (Symbol *) malloc(sizeof(Symbol));
    if (novo == NULL) {
        fprintf(stderr, "Erro de alocacao de memoria.\n");
        exit(1);
    }

    novo->name = dupstr_local(name);
    novo->type = type;
    novo->linha = linha;
    novo->next = stack->top->symbols;
    stack->top->symbols = novo;

    return 1;
}

Symbol *symtab_lookup(SymbolTableStack *stack, const char *name) {
    if (stack == NULL || name == NULL) return NULL;

    Scope *escopo = stack->top;
    while (escopo != NULL) {
        Symbol *s = escopo->symbols;
        while (s != NULL) {
            if (strcmp(s->name, name) == 0) {
                return s;
            }
            s = s->next;
        }
        escopo = escopo->next;
    }

    return NULL;
}

void symtab_free(SymbolTableStack *stack) {
    if (stack == NULL) return;

    while (stack->top != NULL) {
        symtab_pop_scope(stack);
    }
}
