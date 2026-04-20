%{
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "semantic.h"
#include "codegen.h"

extern int yylex();
extern int yylineno;
extern char *yytext;
extern FILE *yyin;

void yyerror(const char *s);

ASTNode *raiz = NULL;

static ASTNode* criar_constante_car_de_lexema(char *lex, int linha) {
    char valor;

    if (lex[1] == '\\') {
        switch (lex[2]) {
            case 'n': valor = '\n'; break;
            case 't': valor = '\t'; break;
            case '\\': valor = '\\'; break;
            case '\'': valor = '\''; break;
            default: valor = lex[2]; break;
        }
    } else {
        valor = lex[1];
    }

    return criar_no_car(valor, linha);
}
%}

%union {
    int iValue;
    char *sIndex;
    struct ast_node *nPtr;
}

%token PRINCIPAL INT CAR LEIA ESCREVA NOVALINHA
%token SE ENTAO SENAO FIMSE ENQUANTO
%token OU E IGUAL DIFERENTE MAIORIGUAL MENORIGUAL

%token <sIndex> IDENTIFICADOR
%token <iValue> INTCONST
%token <sIndex> CARCONST
%token <sIndex> CADEIACARACTERES

%type <nPtr> Programa DeclPrograma Bloco VarSection ListaDeclVar DeclVar Tipo
%type <nPtr> ListaComando Comando Expr OrExpr AndExpr EqExpr DesigExpr
%type <nPtr> AddExpr MulExpr UnExpr PrimExpr

%start Programa

%%

Programa
    : DeclPrograma
      {
          raiz = $1;
          $$ = $1;
      }
    ;

DeclPrograma
    : PRINCIPAL Bloco
      {
          $$ = criar_no_programa($2, yylineno);
      }
    ;

Bloco
    : '{' ListaComando '}'
      {
          $$ = criar_no_bloco(NULL, $2, yylineno);
      }
    | VarSection '{' ListaComando '}'
      {
          $$ = criar_no_bloco($1, $3, yylineno);
      }
    ;

VarSection
    : '{' ListaDeclVar '}'
      {
          $$ = $2;
      }
    ;

ListaDeclVar
    : IDENTIFICADOR DeclVar ':' Tipo ';'
      {
          ASTNode *primeiro = criar_no_declaracao($1, $4, yylineno);
          ASTNode *p = $2;

          while (p != NULL) {
              p->esq = criar_no_tipo($4->value_type, p->linha);
              p = p->proximo;
          }

          $$ = anexar_no(primeiro, $2);
      }
    | ListaDeclVar IDENTIFICADOR DeclVar ':' Tipo ';'
      {
          ASTNode *primeiro = criar_no_declaracao($2, $5, yylineno);
          ASTNode *p = $3;

          while (p != NULL) {
              p->esq = criar_no_tipo($5->value_type, p->linha);
              p = p->proximo;
          }

          $$ = anexar_no($1, anexar_no(primeiro, $3));
      }
    ;

DeclVar
    : %empty
      {
          $$ = NULL;
      }
    | ',' IDENTIFICADOR DeclVar
      {
          ASTNode *no = criar_no_declaracao($2, NULL, yylineno);
          no->proximo = $3;
          $$ = no;
      }
    ;

Tipo
    : INT
      {
          $$ = criar_no_tipo(TIPO_INT, yylineno);
      }
    | CAR
      {
          $$ = criar_no_tipo(TIPO_CAR, yylineno);
      }
    ;

ListaComando
    : Comando
      {
          $$ = $1;
      }
    | Comando ListaComando
      {
          if ($1 == NULL) $$ = $2;
          else $$ = anexar_no($1, $2);
      }
    ;

Comando
    : ';'
      {
          $$ = NULL;
      }
    | Expr ';'
      {
          $$ = $1;
      }
    | LEIA IDENTIFICADOR ';'
      {
          $$ = criar_no_leia($2, yylineno);
      }
    | ESCREVA Expr ';'
      {
          $$ = criar_no_escreva($2, yylineno);
      }
    | ESCREVA CADEIACARACTERES ';'
      {
          $$ = criar_no_escreva(criar_no_string($2, yylineno), yylineno);
      }
    | NOVALINHA ';'
      {
          $$ = criar_no_novalinha(yylineno);
      }
    | SE '(' Expr ')' ENTAO Comando FIMSE
      {
          $$ = criar_no_if($3, $6, NULL, yylineno);
      }
    | SE '(' Expr ')' ENTAO Comando SENAO Comando FIMSE
      {
          $$ = criar_no_if($3, $6, $8, yylineno);
      }
    | ENQUANTO '(' Expr ')' Comando
      {
          $$ = criar_no_while($3, $5, yylineno);
      }
    | Bloco
      {
          $$ = $1;
      }
    ;

Expr
    : OrExpr
      {
          $$ = $1;
      }
    | IDENTIFICADOR '=' Expr
      {
          $$ = criar_no_atribuicao(criar_no_id($1, yylineno), $3, yylineno);
      }
    ;

OrExpr
    : AndExpr
      {
          $$ = $1;
      }
    | OrExpr OU AndExpr
      {
          $$ = criar_no_op(OU, $1, $3, yylineno);
      }
    ;

AndExpr
    : EqExpr
      {
          $$ = $1;
      }
    | AndExpr E EqExpr
      {
          $$ = criar_no_op(E, $1, $3, yylineno);
      }
    ;

EqExpr
    : DesigExpr
      {
          $$ = $1;
      }
    | EqExpr IGUAL DesigExpr
      {
          $$ = criar_no_op(IGUAL, $1, $3, yylineno);
      }
    | EqExpr DIFERENTE DesigExpr
      {
          $$ = criar_no_op(DIFERENTE, $1, $3, yylineno);
      }
    ;

DesigExpr
    : AddExpr
      {
          $$ = $1;
      }
    | DesigExpr '<' AddExpr
      {
          $$ = criar_no_op('<', $1, $3, yylineno);
      }
    | DesigExpr '>' AddExpr
      {
          $$ = criar_no_op('>', $1, $3, yylineno);
      }
    | DesigExpr MAIORIGUAL AddExpr
      {
          $$ = criar_no_op(MAIORIGUAL, $1, $3, yylineno);
      }
    | DesigExpr MENORIGUAL AddExpr
      {
          $$ = criar_no_op(MENORIGUAL, $1, $3, yylineno);
      }
    ;

AddExpr
    : MulExpr
      {
          $$ = $1;
      }
    | AddExpr '+' MulExpr
      {
          $$ = criar_no_op('+', $1, $3, yylineno);
      }
    | AddExpr '-' MulExpr
      {
          $$ = criar_no_op('-', $1, $3, yylineno);
      }
    ;

MulExpr
    : UnExpr
      {
          $$ = $1;
      }
    | MulExpr '*' UnExpr
      {
          $$ = criar_no_op('*', $1, $3, yylineno);
      }
    | MulExpr '/' UnExpr
      {
          $$ = criar_no_op('/', $1, $3, yylineno);
      }
    ;

UnExpr
    : PrimExpr
      {
          $$ = $1;
      }
    | '-' PrimExpr
      {
          $$ = criar_no_op('-', NULL, $2, yylineno);
      }
    | '!' PrimExpr
      {
          $$ = criar_no_op('!', NULL, $2, yylineno);
      }
    ;

PrimExpr
    : IDENTIFICADOR
      {
          $$ = criar_no_id($1, yylineno);
      }
    | CARCONST
      {
          $$ = criar_constante_car_de_lexema($1, yylineno);
      }
    | INTCONST
      {
          $$ = criar_no_int($1, yylineno);
      }
    | '(' Expr ')'
      {
          $$ = $2;
      }
    ;

%%

void yyerror(const char *s) {
    (void)s;
    printf("ERRO SINTATICO: %d\n", yylineno);
}

int main(int argc, char **argv) {
    int parse_result;
    char arquivo_saida[1024];

    if (argc < 2) {
        fprintf(stderr, "Uso: %s arquivo.g\n", argv[0]);
        return 1;
    }

    yyin = fopen(argv[1], "r");
    if (!yyin) {
        perror("Erro ao abrir arquivo");
        return 1;
    }

    parse_result = yyparse();

    if (parse_result == 0 && raiz != NULL) {
        analisar_semantica(raiz);
        imprimir_ast(raiz, 0);

        snprintf(arquivo_saida, sizeof(arquivo_saida), "%s.s", argv[1]);
        gerar_codigo(raiz, arquivo_saida);

        liberar_ast(raiz);
    }

    fclose(yyin);
    return parse_result;
}
