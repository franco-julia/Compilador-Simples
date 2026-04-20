#ifndef AST_H
#define AST_H

typedef enum {
    NODE_PROGRAMA,
    NODE_BLOCO,
    NODE_DECLARACAO,
    NODE_TIPO,
    NODE_ATRIBUICAO,
    NODE_IF,
    NODE_WHILE,
    NODE_OPERACAO,
    NODE_IDENTIFICADOR,
    NODE_CONST_INT,
    NODE_CONST_CAR,
    NODE_CONST_STRING,
    NODE_LEIA,
    NODE_ESCREVA,
    NODE_NOVALINHA
} NodeType;

typedef enum {
    TIPO_INT,
    TIPO_CAR,
    TIPO_INVALIDO
} ValueType;

typedef struct ast_node {
    NodeType type;
    int linha;

    struct ast_node *esq;
    struct ast_node *dir;
    struct ast_node *terceiro; /* usado principalmente no IF para o SENAO */
    struct ast_node *proximo;  /* usado somente para listas */

    char *id_name;
    char *str_val;
    int int_val;
    char car_val;
    int op;

    ValueType value_type;
} ASTNode;

ASTNode* criar_no(NodeType type, int linha);
ASTNode* criar_no_programa(ASTNode *bloco, int linha);
ASTNode* criar_no_bloco(ASTNode *decls, ASTNode *comandos, int linha);
ASTNode* criar_no_declaracao(char *name, ASTNode *tipo, int linha);
ASTNode* criar_no_tipo(ValueType tipo, int linha);
ASTNode* criar_no_id(char *name, int linha);
ASTNode* criar_no_int(int val, int linha);
ASTNode* criar_no_car(char value, int linha);
ASTNode* criar_no_string(char *text, int linha);
ASTNode* criar_no_op(int op, ASTNode *esq, ASTNode *dir, int linha);
ASTNode* criar_no_leia(char *name, int linha);
ASTNode* criar_no_escreva(ASTNode *expr, int linha);
ASTNode* criar_no_novalinha(int linha);
ASTNode* criar_no_atribuicao(ASTNode *id, ASTNode *expr, int linha);
ASTNode* criar_no_if(ASTNode *cond, ASTNode *entao, ASTNode *senao, int linha);
ASTNode* criar_no_while(ASTNode *cond, ASTNode *corpo, int linha);

ASTNode* anexar_no(ASTNode *lista, ASTNode *no);

void imprimir_ast(ASTNode *no, int nivel);
void liberar_ast(ASTNode *no);

const char* nome_tipo(ValueType tipo);

#endif
