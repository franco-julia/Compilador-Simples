#include "codegen.h"
#include "g-v1.tab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct DeclInfo {
    ASTNode *decl_node;
    char *nome;
    ValueType tipo;
    char rotulo[64];
    struct DeclInfo *next;
} DeclInfo;

typedef struct StringInfo {
    ASTNode *string_node;
    char rotulo[64];
    struct StringInfo *next;
} StringInfo;

typedef struct ScopeBinding {
    char *nome;
    DeclInfo *decl;
    struct ScopeBinding *next;
} ScopeBinding;

typedef struct ScopeFrame {
    ScopeBinding *bindings;
    struct ScopeFrame *prev;
} ScopeFrame;

static DeclInfo *decls = NULL;
static StringInfo *strings = NULL;
static ScopeFrame *scope_top = NULL;

static int decl_counter = 0;
static int string_counter = 0;
static int label_counter = 0;
static int temp_top = 0;

static void erro_codegen(const char *msg) {
    fprintf(stderr, "Erro na geracao de codigo: %s\n", msg);
    exit(1);
}

static char *dupstr(const char *s) {
    if (s == NULL) return NULL;
    char *c = (char *) malloc(strlen(s) + 1);
    if (!c) erro_codegen("falha de memoria");
    strcpy(c, s);
    return c;
}

static void push_scope(void) {
    ScopeFrame *f = (ScopeFrame *) malloc(sizeof(ScopeFrame));
    if (!f) erro_codegen("falha de memoria");
    f->bindings = NULL;
    f->prev = scope_top;
    scope_top = f;
}

static void pop_scope(void) {
    if (!scope_top) return;

    ScopeBinding *b = scope_top->bindings;
    while (b) {
        ScopeBinding *next = b->next;
        free(b->nome);
        free(b);
        b = next;
    }

    ScopeFrame *old = scope_top;
    scope_top = scope_top->prev;
    free(old);
}

static void bind_name(const char *nome, DeclInfo *decl) {
    if (!scope_top) erro_codegen("escopo inexistente");

    ScopeBinding *b = (ScopeBinding *) malloc(sizeof(ScopeBinding));
    if (!b) erro_codegen("falha de memoria");

    b->nome = dupstr(nome);
    b->decl = decl;
    b->next = scope_top->bindings;
    scope_top->bindings = b;
}

static DeclInfo *lookup_decl_in_scope_chain(const char *nome) {
    ScopeFrame *f = scope_top;
    while (f) {
        ScopeBinding *b = f->bindings;
        while (b) {
            if (strcmp(b->nome, nome) == 0) {
                return b->decl;
            }
            b = b->next;
        }
        f = f->prev;
    }
    return NULL;
}

static DeclInfo *add_decl(ASTNode *decl_node) {
    DeclInfo *d = (DeclInfo *) malloc(sizeof(DeclInfo));
    if (!d) erro_codegen("falha de memoria");

    d->decl_node = decl_node;
    d->nome = dupstr(decl_node->id_name);
    d->tipo = (decl_node->esq != NULL) ? decl_node->esq->value_type : TIPO_INVALIDO;
    snprintf(d->rotulo, sizeof(d->rotulo), "var_%s_%d", d->nome, decl_counter++);
    d->next = decls;
    decls = d;
    return d;
}

static DeclInfo *find_decl_by_node(ASTNode *decl_node) {
    DeclInfo *d = decls;
    while (d) {
        if (d->decl_node == decl_node) return d;
        d = d->next;
    }
    return NULL;
}

static StringInfo *add_string(ASTNode *string_node) {
    StringInfo *s = (StringInfo *) malloc(sizeof(StringInfo));
    if (!s) erro_codegen("falha de memoria");

    s->string_node = string_node;
    snprintf(s->rotulo, sizeof(s->rotulo), "str_%d", string_counter++);
    s->next = strings;
    strings = s;
    return s;
}

static StringInfo *find_string_by_node(ASTNode *string_node) {
    StringInfo *s = strings;
    while (s) {
        if (s->string_node == string_node) return s;
        s = s->next;
    }
    return NULL;
}

static int nova_label(void) {
    return label_counter++;
}

static const char *alloc_temp(void) {
    static char regs[10][4] = {
        "$t0", "$t1", "$t2", "$t3", "$t4",
        "$t5", "$t6", "$t7", "$t8", "$t9"
    };

    if (temp_top >= 10) {
        erro_codegen("expressao profunda demais para registradores temporarios");
    }

    return regs[temp_top++];
}

static void free_temp(void) {
    if (temp_top > 0) temp_top--;
}


static void coletar_infos(ASTNode *no) {
    while (no) {
        switch (no->type) {
            case NODE_PROGRAMA:
                coletar_infos(no->esq);
                break;

            case NODE_BLOCO:
                coletar_infos(no->esq);
                coletar_infos(no->dir);
                break;

            case NODE_DECLARACAO:
                if (!find_decl_by_node(no)) {
                    add_decl(no);
                }
                break;

            case NODE_CONST_STRING:
                if (!find_string_by_node(no)) {
                    add_string(no);
                }
                break;

            case NODE_IF:
                coletar_infos(no->esq);
                coletar_infos(no->dir);
                coletar_infos(no->terceiro);
                break;

            default:
                coletar_infos(no->esq);
                coletar_infos(no->dir);
                coletar_infos(no->terceiro);
                break;
        }

        no = no->proximo;
    }
}

static void emitir_data(FILE *out) {
    fprintf(out, ".data\n");

    DeclInfo *d = decls;
    while (d) {
        if (d->tipo == TIPO_INT) {
            fprintf(out, "%s: .word 0\n", d->rotulo);
        } else if (d->tipo == TIPO_CAR) {
            fprintf(out, "%s: .byte 0\n", d->rotulo);
        } else {
            fprintf(out, "%s: .word 0\n", d->rotulo);
        }
        d = d->next;
    }

    StringInfo *s = strings;
    while (s) {
        const char *txt = s->string_node->str_val ? s->string_node->str_val : "\"\"";
        fprintf(out, "%s: .asciiz %s\n", s->rotulo, txt);
        s = s->next;
    }

    fprintf(out, "nl: .asciiz \"\\n\"\n\n");
}

static void registrar_decls_do_bloco(ASTNode *decls_lista) {
    ASTNode *d = decls_lista;
    while (d) {
        if (d->type == NODE_DECLARACAO) {
            DeclInfo *info = find_decl_by_node(d);
            if (!info) erro_codegen("declaracao nao encontrada");
            bind_name(d->id_name, info);
        }
        d = d->proximo;
    }
}

static const char *gerar_expr(ASTNode *no, FILE *out);

static void gerar_operacao_binaria(ASTNode *no, FILE *out, const char *dst) {
    const char *r1 = gerar_expr(no->esq, out);
    const char *r2 = gerar_expr(no->dir, out);

    switch (no->op) {
        case '+':
            fprintf(out, "add %s, %s, %s\n", dst, r1, r2);
            break;
        case '-':
            fprintf(out, "sub %s, %s, %s\n", dst, r1, r2);
            break;
        case '*':
            fprintf(out, "mul %s, %s, %s\n", dst, r1, r2);
            break;
        case '/':
            fprintf(out, "div %s, %s\nmflo %s\n", r1, r2, dst);
            break;
        case '<':
            fprintf(out, "slt %s, %s, %s\n", dst, r1, r2);
            break;
        case '>':
            fprintf(out, "slt %s, %s, %s\n", dst, r2, r1);
            break;
        case MENORIGUAL:
            fprintf(out, "slt %s, %s, %s\nxori %s, %s, 1\n", dst, r2, r1, dst, dst);
            break;
        case MAIORIGUAL:
            fprintf(out, "slt %s, %s, %s\nxori %s, %s, 1\n", dst, r1, r2, dst, dst);
            break;
        case IGUAL:
            fprintf(out, "sub %s, %s, %s\nsltiu %s, %s, 1\n", dst, r1, r2, dst, dst);
            break;
        case DIFERENTE:
            fprintf(out, "sub %s, %s, %s\nsltu %s, $zero, %s\n", dst, r1, r2, dst, dst);
            break;
        case E:
            fprintf(out, "sltu %s, $zero, %s\n", dst, r1);
            fprintf(out, "sltu %s, $zero, %s\n", r2, r2);
            fprintf(out, "and %s, %s, %s\n", dst, dst, r2);
            break;
        case OU:
            fprintf(out, "or %s, %s, %s\n", dst, r1, r2);
            fprintf(out, "sltu %s, $zero, %s\n", dst, dst);
            break;
        default:
            erro_codegen("operador binario desconhecido");
    }

    free_temp();
    free_temp();
}

static const char *gerar_expr(ASTNode *no, FILE *out) {
    if (!no) erro_codegen("expressao nula");

    const char *dst = alloc_temp();

    switch (no->type) {
        case NODE_CONST_INT:
            fprintf(out, "li %s, %d\n", dst, no->int_val);
            return dst;

        case NODE_CONST_CAR:
            fprintf(out, "li %s, %d\n", dst, (unsigned char) no->car_val);
            return dst;

        case NODE_IDENTIFICADOR: {
            DeclInfo *d = lookup_decl_in_scope_chain(no->id_name);
            if (!d) {
                fprintf(stderr, "Variavel nao encontrada no codegen: %s\n", no->id_name);
                exit(1);
            }

            if (d->tipo == TIPO_CAR) {
                fprintf(out, "lb %s, %s\n", dst, d->rotulo);
            } else {
                fprintf(out, "lw %s, %s\n", dst, d->rotulo);
            }
            return dst;
        }

        case NODE_ATRIBUICAO: {
            const char *rhs = gerar_expr(no->dir, out);
            ASTNode *id = no->esq;
            DeclInfo *d = lookup_decl_in_scope_chain(id->id_name);
            if (!d) erro_codegen("variavel de atribuicao nao encontrada");

            if (d->tipo == TIPO_CAR) {
                fprintf(out, "sb %s, %s\n", rhs, d->rotulo);
            } else {
                fprintf(out, "sw %s, %s\n", rhs, d->rotulo);
            }
            fprintf(out, "move %s, %s\n", dst, rhs);
            free_temp();
            return dst;
        }

        case NODE_OPERACAO:
            if (no->op == '!' && no->esq == NULL) {
                const char *r = gerar_expr(no->dir, out);
                fprintf(out, "sltiu %s, %s, 1\n", dst, r);
                free_temp();
                return dst;
            }

            if (no->op == '-' && no->esq == NULL) {
                const char *r = gerar_expr(no->dir, out);
                fprintf(out, "sub %s, $zero, %s\n", dst, r);
                free_temp();
                return dst;
            }

            gerar_operacao_binaria(no, out, dst);
            return dst;

        default:
            erro_codegen("tipo de no invalido em expressao");
            return NULL;
    }
}

static void gerar_comandos(ASTNode *no, FILE *out);

static void gerar_bloco(ASTNode *bloco, FILE *out) {
    if (!bloco || bloco->type != NODE_BLOCO) return;

    push_scope();
    registrar_decls_do_bloco(bloco->esq);
    gerar_comandos(bloco->dir, out);
    pop_scope();
}

static void gerar_if(ASTNode *no, FILE *out) {
    int l_else = nova_label();
    int l_end = nova_label();

    const char *cond = gerar_expr(no->esq, out);
    fprintf(out, "beq %s, $zero, L%d\n", cond, l_else);
    free_temp();

    if (no->dir && no->dir->type == NODE_BLOCO) {
        gerar_bloco(no->dir, out);
    } else {
        gerar_comandos(no->dir, out);
    }

    fprintf(out, "j L%d\n", l_end);
    fprintf(out, "L%d:\n", l_else);

    if (no->terceiro) {
        if (no->terceiro->type == NODE_BLOCO) {
            gerar_bloco(no->terceiro, out);
        } else {
            gerar_comandos(no->terceiro, out);
        }
    }

    fprintf(out, "L%d:\n", l_end);
}

static void gerar_while(ASTNode *no, FILE *out) {
    int l_ini = nova_label();
    int l_fim = nova_label();

    fprintf(out, "L%d:\n", l_ini);
    const char *cond = gerar_expr(no->esq, out);
    fprintf(out, "beq %s, $zero, L%d\n", cond, l_fim);
    free_temp();

    if (no->dir && no->dir->type == NODE_BLOCO) {
        gerar_bloco(no->dir, out);
    } else {
        gerar_comandos(no->dir, out);
    }

    fprintf(out, "j L%d\n", l_ini);
    fprintf(out, "L%d:\n", l_fim);
}

static void gerar_escreva(ASTNode *no, FILE *out) {
    if (!no->esq) return;

    if (no->esq->type == NODE_CONST_STRING) {
        StringInfo *s = find_string_by_node(no->esq);
        if (!s) erro_codegen("string nao encontrada");

        fprintf(out, "la $a0, %s\n", s->rotulo);
        fprintf(out, "li $v0, 4\nsyscall\n");
        return;
    }

    const char *r = gerar_expr(no->esq, out);
    ValueType t = no->esq->value_type;

    if (t == TIPO_CAR) {
        fprintf(out, "move $a0, %s\n", r);
        fprintf(out, "li $v0, 11\nsyscall\n");
    } else {
        fprintf(out, "move $a0, %s\n", r);
        fprintf(out, "li $v0, 1\nsyscall\n");
    }

    free_temp();
}

static void gerar_leia(ASTNode *no, FILE *out) {
    const char *nome = NULL;

    if (no->esq && no->esq->type == NODE_IDENTIFICADOR) {
        nome = no->esq->id_name;
    } else {
        nome = no->id_name;
    }

    if (!nome) erro_codegen("comando leia sem identificador");

    DeclInfo *d = lookup_decl_in_scope_chain(nome);
    if (!d) erro_codegen("variavel do leia nao encontrada");

    if (d->tipo == TIPO_CAR) {
        fprintf(out, "li $v0, 12\nsyscall\n");
        fprintf(out, "sb $v0, %s\n", d->rotulo);
    } else {
        fprintf(out, "li $v0, 5\nsyscall\n");
        fprintf(out, "sw $v0, %s\n", d->rotulo);
    }
}

static void gerar_comandos(ASTNode *no, FILE *out) {
    while (no) {
        switch (no->type) {
            case NODE_BLOCO:
                gerar_bloco(no, out);
                break;

            case NODE_ATRIBUICAO: {
                const char *r = gerar_expr(no, out);
                (void) r;
                free_temp();
                break;
            }

            case NODE_ESCREVA:
                gerar_escreva(no, out);
                break;

            case NODE_LEIA:
                gerar_leia(no, out);
                break;

            case NODE_NOVALINHA:
                fprintf(out, "la $a0, nl\n");
                fprintf(out, "li $v0, 4\nsyscall\n");
                break;

            case NODE_IF:
                gerar_if(no, out);
                break;

            case NODE_WHILE:
                gerar_while(no, out);
                break;

            case NODE_OPERACAO: {
                const char *r = gerar_expr(no, out);
                (void) r;
                free_temp();
                break;
            }

            case NODE_IDENTIFICADOR:
            case NODE_CONST_INT:
            case NODE_CONST_CAR:
            case NODE_CONST_STRING:
                break;

            default:
                break;
        }

        no = no->proximo;
    }
}

void gerar_codigo(ASTNode *raiz, const char *arquivo_saida) {
    if (!raiz || !arquivo_saida) {
        erro_codegen("parametros invalidos");
    }

    FILE *out = fopen(arquivo_saida, "w");
    if (!out) {
        erro_codegen("nao foi possivel abrir arquivo de saida");
    }

    coletar_infos(raiz);

    emitir_data(out);

    fprintf(out, ".text\n");
    fprintf(out, ".globl main\n");
    fprintf(out, "main:\n");

    if (raiz->type == NODE_PROGRAMA) {
        if (raiz->esq && raiz->esq->type == NODE_BLOCO) {
            gerar_bloco(raiz->esq, out);
        } else {
            gerar_comandos(raiz->esq, out);
        }
    } else if (raiz->type == NODE_BLOCO) {
        gerar_bloco(raiz, out);
    } else {
        gerar_comandos(raiz, out);
    }

    fprintf(out, "li $v0, 10\n");
    fprintf(out, "syscall\n");

    fclose(out);
}
