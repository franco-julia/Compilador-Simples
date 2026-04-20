#define _POSIX_C_SOURCE 200809L
#include "ast.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static char* duplicar_string(const char *s) {
    if (s == NULL) return NULL;

    char *copia = strdup(s);
    if (copia == NULL) {
        fprintf(stderr, "Erro de alocacao de memoria.\n");
        exit(1);
    }
    return copia;
}

ASTNode* criar_no(NodeType type, int linha) {
    ASTNode *no = (ASTNode*) malloc(sizeof(ASTNode));
    if (no == NULL) {
        fprintf(stderr, "Erro de alocacao de memoria.\n");
        exit(1);
    }

    no->type = type;
    no->linha = linha;

    no->esq = NULL;
    no->dir = NULL;
    no->terceiro = NULL;
    no->proximo = NULL;

    no->id_name = NULL;
    no->str_val = NULL;
    no->int_val = 0;
    no->car_val = 0;
    no->op = 0;
    no->value_type = TIPO_INVALIDO;

    return no;
}

ASTNode* criar_no_programa(ASTNode *bloco, int linha) {
    ASTNode *no = criar_no(NODE_PROGRAMA, linha);
    no->esq = bloco;
    return no;
}

ASTNode* criar_no_bloco(ASTNode *decls, ASTNode *comandos, int linha) {
    ASTNode *no = criar_no(NODE_BLOCO, linha);
    no->esq = decls;
    no->dir = comandos;
    return no;
}

ASTNode* criar_no_declaracao(char *name, ASTNode *tipo, int linha) {
    ASTNode *no = criar_no(NODE_DECLARACAO, linha);
    no->id_name = duplicar_string(name);
    no->esq = tipo;
    return no;
}

ASTNode* criar_no_tipo(ValueType tipo, int linha) {
    ASTNode *no = criar_no(NODE_TIPO, linha);
    no->value_type = tipo;
    return no;
}

ASTNode* criar_no_id(char *name, int linha) {
    ASTNode *no = criar_no(NODE_IDENTIFICADOR, linha);
    no->id_name = duplicar_string(name);
    return no;
}

ASTNode* criar_no_int(int val, int linha) {
    ASTNode *no = criar_no(NODE_CONST_INT, linha);
    no->int_val = val;
    no->value_type = TIPO_INT;
    return no;
}

ASTNode* criar_no_car(char value, int linha) {
    ASTNode *no = criar_no(NODE_CONST_CAR, linha);
    no->car_val = value;
    no->value_type = TIPO_CAR;
    return no;
}

ASTNode* criar_no_string(char *text, int linha) {
    ASTNode *no = criar_no(NODE_CONST_STRING, linha);
    no->str_val = duplicar_string(text);
    return no;
}

ASTNode* criar_no_op(int op, ASTNode *esq, ASTNode *dir, int linha) {
    ASTNode *no = criar_no(NODE_OPERACAO, linha);
    no->op = op;
    no->esq = esq;
    no->dir = dir;
    return no;
}

ASTNode* criar_no_leia(char *name, int linha) {
    ASTNode *no = criar_no(NODE_LEIA, linha);

    /* Mantive id_name para compatibilidade com seu semantic/codegen atuais */
    no->id_name = duplicar_string(name);

    /* E também guardo o identificador como filho, para deixar a AST mais consistente */
    no->esq = criar_no_id(name, linha);

    return no;
}

ASTNode* criar_no_escreva(ASTNode *expr, int linha) {
    ASTNode *no = criar_no(NODE_ESCREVA, linha);
    no->esq = expr;
    return no;
}

ASTNode* criar_no_novalinha(int linha) {
    return criar_no(NODE_NOVALINHA, linha);
}

ASTNode* criar_no_atribuicao(ASTNode *id, ASTNode *expr, int linha) {
    ASTNode *no = criar_no(NODE_ATRIBUICAO, linha);
    no->esq = id;
    no->dir = expr;
    return no;
}

ASTNode* criar_no_if(ASTNode *cond, ASTNode *entao, ASTNode *senao, int linha) {
    ASTNode *no = criar_no(NODE_IF, linha);
    no->esq = cond;
    no->dir = entao;
    no->terceiro = senao; /* correção principal */
    return no;
}

ASTNode* criar_no_while(ASTNode *cond, ASTNode *corpo, int linha) {
    ASTNode *no = criar_no(NODE_WHILE, linha);
    no->esq = cond;
    no->dir = corpo;
    return no;
}

ASTNode* anexar_no(ASTNode *lista, ASTNode *no) {
    if (lista == NULL) return no;
    if (no == NULL) return lista;

    ASTNode *p = lista;
    while (p->proximo != NULL) {
        p = p->proximo;
    }
    p->proximo = no;
    return lista;
}

const char* nome_tipo(ValueType tipo) {
    switch (tipo) {
        case TIPO_INT: return "int";
        case TIPO_CAR: return "car";
        default: return "invalido";
    }
}

static void imprimir_indentacao(int nivel) {
    for (int i = 0; i < nivel; i++) {
        printf("  ");
    }
}

static void imprimir_operador(int op) {
    switch (op) {
        case '+': printf("+"); break;
        case '-': printf("-"); break;
        case '*': printf("*"); break;
        case '/': printf("/"); break;
        case '<': printf("<"); break;
        case '>': printf(">"); break;
        case '!': printf("!"); break;
        default:  printf("%d", op); break;
    }
}

void imprimir_ast(ASTNode *no, int nivel) {
    if (no == NULL) return;

    imprimir_indentacao(nivel);

    switch (no->type) {
        case NODE_PROGRAMA:
            printf("PROGRAMA (linha %d)\n", no->linha);
            break;

        case NODE_BLOCO:
            printf("BLOCO (linha %d)\n", no->linha);
            break;

        case NODE_DECLARACAO:
            printf("DECLARACAO: %s (linha %d)\n",
                   no->id_name ? no->id_name : "(null)", no->linha);
            break;

        case NODE_TIPO:
            printf("TIPO: %s (linha %d)\n", nome_tipo(no->value_type), no->linha);
            break;

        case NODE_ATRIBUICAO:
            printf("ATRIBUICAO (linha %d)\n", no->linha);
            break;

        case NODE_IF:
            printf("IF (linha %d)\n", no->linha);
            break;

        case NODE_WHILE:
            printf("WHILE (linha %d)\n", no->linha);
            break;

        case NODE_OPERACAO:
            printf("OPERACAO: ");
            imprimir_operador(no->op);
            printf(" (linha %d)\n", no->linha);
            break;

        case NODE_IDENTIFICADOR:
            printf("IDENTIFICADOR: %s (linha %d)\n",
                   no->id_name ? no->id_name : "(null)", no->linha);
            break;

        case NODE_CONST_INT:
            printf("CONST_INT: %d (linha %d)\n", no->int_val, no->linha);
            break;

        case NODE_CONST_CAR:
            if (no->car_val == '\n') {
                printf("CONST_CAR: '\\n' (linha %d)\n", no->linha);
            } else if (no->car_val == '\t') {
                printf("CONST_CAR: '\\t' (linha %d)\n", no->linha);
            } else if (no->car_val == '\'') {
                printf("CONST_CAR: '\\'' (linha %d)\n", no->linha);
            } else if (no->car_val == '\\') {
                printf("CONST_CAR: '\\\\' (linha %d)\n", no->linha);
            } else {
                printf("CONST_CAR: '%c' (linha %d)\n", no->car_val, no->linha);
            }
            break;

        case NODE_CONST_STRING:
            printf("CONST_STRING: %s (linha %d)\n",
                   no->str_val ? no->str_val : "(null)", no->linha);
            break;

        case NODE_LEIA:
            printf("LEIA (linha %d)\n", no->linha);
            break;

        case NODE_ESCREVA:
            printf("ESCREVA (linha %d)\n", no->linha);
            break;

        case NODE_NOVALINHA:
            printf("NOVALINHA (linha %d)\n", no->linha);
            break;

        default:
            printf("NO_DESCONHECIDO (linha %d)\n", no->linha);
            break;
    }

    if (no->esq != NULL) {
        imprimir_indentacao(nivel + 1);
        printf("esq:\n");
        imprimir_ast(no->esq, nivel + 2);
    }

    if (no->dir != NULL) {
        imprimir_indentacao(nivel + 1);
        printf("dir:\n");
        imprimir_ast(no->dir, nivel + 2);
    }

    if (no->terceiro != NULL) {
        imprimir_indentacao(nivel + 1);
        if (no->type == NODE_IF) {
            printf("senao:\n");
        } else {
            printf("terceiro:\n");
        }
        imprimir_ast(no->terceiro, nivel + 2);
    }

    if (no->proximo != NULL) {
        imprimir_indentacao(nivel + 1);
        printf("proximo:\n");
        imprimir_ast(no->proximo, nivel + 2);
    }
}

void liberar_ast(ASTNode *no) {
    if (no == NULL) return;

    liberar_ast(no->esq);
    liberar_ast(no->dir);
    liberar_ast(no->terceiro);
    liberar_ast(no->proximo);

    free(no->id_name);
    free(no->str_val);
    free(no);
}
