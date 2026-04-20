#include "semantic.h"
#include "symtab.h"
#include "g-v1.tab.h"
#include <stdio.h>
#include <stdlib.h>

static SymbolTableStack stack;

static void erro_semantico(const char *msg, int linha) {
    printf("ERRO SEMANTICO: %s %d\n", msg, linha);
    symtab_free(&stack);
    exit(1);
}

static ValueType tipo_do_no(ASTNode *no) {
    ValueType t1, t2;
    Symbol *s;

    if (no == NULL) {
        return TIPO_INVALIDO;
    }

    switch (no->type) {
        case NODE_CONST_INT:
            no->value_type = TIPO_INT;
            return TIPO_INT;

        case NODE_CONST_CAR:
            no->value_type = TIPO_CAR;
            return TIPO_CAR;

        case NODE_CONST_STRING:
            no->value_type = TIPO_INVALIDO;
            return TIPO_INVALIDO;

        case NODE_TIPO:
            return no->value_type;

        case NODE_IDENTIFICADOR:
            s = symtab_lookup(&stack, no->id_name);
            if (s == NULL) {
                erro_semantico("IDENTIFICADOR NAO DECLARADO", no->linha);
            }
            no->value_type = s->type;
            return s->type;

        case NODE_OPERACAO:
            if (no->op == '!') {
                t1 = tipo_do_no(no->dir);
                if (t1 != TIPO_INT) {
                    erro_semantico("OPERACAO LOGICA INVALIDA", no->linha);
                }
                no->value_type = TIPO_INT;
                return TIPO_INT;
            }

            if (no->esq == NULL && no->op == '-') {
                t1 = tipo_do_no(no->dir);
                if (t1 != TIPO_INT) {
                    erro_semantico("OPERACAO ARITMETICA INVALIDA", no->linha);
                }
                no->value_type = TIPO_INT;
                return TIPO_INT;
            }

            t1 = tipo_do_no(no->esq);
            t2 = tipo_do_no(no->dir);

            switch (no->op) {
                case '+':
                case '-':
                case '*':
                case '/':
                    if (t1 != TIPO_INT || t2 != TIPO_INT) {
                        erro_semantico("OPERACAO ARITMETICA INVALIDA", no->linha);
                    }
                    no->value_type = TIPO_INT;
                    return TIPO_INT;

                case '<':
                case '>':
                case MAIORIGUAL:
                case MENORIGUAL:
                case IGUAL:
                case DIFERENTE:
                    if (t1 != t2) {
                        erro_semantico("OPERACAO RELACIONAL ENTRE TIPOS DIFERENTES", no->linha);
                    }
                    no->value_type = TIPO_INT;
                    return TIPO_INT;

                case OU:
                case E:
                    if (t1 != TIPO_INT || t2 != TIPO_INT) {
                        erro_semantico("OPERACAO LOGICA INVALIDA", no->linha);
                    }
                    no->value_type = TIPO_INT;
                    return TIPO_INT;

                default:
                    return TIPO_INVALIDO;
            }

        case NODE_ATRIBUICAO:
            t1 = tipo_do_no(no->esq);
            t2 = tipo_do_no(no->dir);

            if (t1 != t2) {
                erro_semantico("ATRIBUICAO COM TIPOS INCOMPATIVEIS", no->linha);
            }

            no->value_type = t1;
            return t1;

        default:
            return TIPO_INVALIDO;
    }
}

static void analisar_lista_declaracoes(ASTNode *decl) {
    ValueType tipo;

    while (decl != NULL) {
        if (decl->type == NODE_DECLARACAO) {
            tipo = TIPO_INVALIDO;

            if (decl->esq != NULL && decl->esq->type == NODE_TIPO) {
                tipo = decl->esq->value_type;
            }

            if (!symtab_insert(&stack, decl->id_name, tipo, decl->linha)) {
                erro_semantico("IDENTIFICADOR JA DECLARADO NO ESCOPO", decl->linha);
            }
        }

        decl = decl->proximo;
    }
}

static void analisar_no(ASTNode *no) {
    Symbol *s;

    while (no != NULL) {
        switch (no->type) {
            case NODE_PROGRAMA:
                analisar_no(no->esq);
                break;

            case NODE_BLOCO:
                symtab_push_scope(&stack);
                analisar_lista_declaracoes(no->esq);
                analisar_no(no->dir);
                symtab_pop_scope(&stack);
                break;

            case NODE_DECLARACAO:
            case NODE_TIPO:
            case NODE_CONST_INT:
            case NODE_CONST_CAR:
            case NODE_CONST_STRING:
            case NODE_NOVALINHA:
                break;

            case NODE_ATRIBUICAO:
            case NODE_IDENTIFICADOR:
            case NODE_OPERACAO:
                tipo_do_no(no);
                break;

            case NODE_LEIA:
                if (no->esq != NULL && no->esq->type == NODE_IDENTIFICADOR) {
                    tipo_do_no(no->esq);
                } else if (no->id_name != NULL) {
                    s = symtab_lookup(&stack, no->id_name);
                    if (s == NULL) {
                        erro_semantico("IDENTIFICADOR NAO DECLARADO", no->linha);
                    }
                } else {
                    erro_semantico("COMANDO LEIA INVALIDO", no->linha);
                }
                break;

            case NODE_ESCREVA:
                if (no->esq != NULL && no->esq->type != NODE_CONST_STRING) {
                    tipo_do_no(no->esq);
                }
                break;

            case NODE_IF:
                if (tipo_do_no(no->esq) != TIPO_INT) {
                    erro_semantico("CONDICAO DO SE DEVE SER INT", no->linha);
                }
                analisar_no(no->dir);
                if (no->terceiro != NULL) {
                    analisar_no(no->terceiro);
                }
                break;

            case NODE_WHILE:
                if (tipo_do_no(no->esq) != TIPO_INT) {
                    erro_semantico("CONDICAO DO ENQUANTO DEVE SER INT", no->linha);
                }
                analisar_no(no->dir);
                break;

            default:
                break;
        }

        no = no->proximo;
    }
}

void analisar_semantica(ASTNode *raiz) {
    symtab_init(&stack);
    analisar_no(raiz);
    symtab_free(&stack);
}
