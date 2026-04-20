# Nome do executável final
EXEC = g-v1

# Compilador C e flags
CC = gcc
CFLAGS = -Wall -Wextra -Wno-unused-function -std=c11 -D_POSIX_C_SOURCE=200809L

# Arquivos principais do Flex e Bison
LEX = g-v1.l
YACC = g-v1.y

# Arquivos gerados automaticamente
LEX_OUT = lex.yy.c
YACC_OUT = g-v1.tab.c
YACC_HDR = g-v1.tab.h

# Arquivos objeto
OBJS = lex.yy.o g-v1.tab.o ast.o symtab.o semantic.o codegen.o

# Regra principal: gera o executável
all: $(EXEC)

# Linkagem final
$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJS) -lfl

# Gera parser e header com Bison
$(YACC_OUT) $(YACC_HDR): $(YACC)
	bison -d $(YACC)

# Gera analisador léxico com Flex
$(LEX_OUT): $(LEX) $(YACC_HDR)
	flex $(LEX)

# Compila o arquivo gerado pelo Flex
lex.yy.o: $(LEX_OUT)
	$(CC) $(CFLAGS) -c $(LEX_OUT)

# Compila o arquivo gerado pelo Bison
g-v1.tab.o: $(YACC_OUT)
	$(CC) $(CFLAGS) -c $(YACC_OUT)

# Compila os módulos do projeto
ast.o: ast.c ast.h
	$(CC) $(CFLAGS) -c ast.c

symtab.o: symtab.c symtab.h ast.h
	$(CC) $(CFLAGS) -c symtab.c

semantic.o: semantic.c semantic.h symtab.h ast.h g-v1.tab.h
	$(CC) $(CFLAGS) -c semantic.c

codegen.o: codegen.c codegen.h ast.h g-v1.tab.h
	$(CC) $(CFLAGS) -c codegen.c

# Remove arquivos gerados
clean:
	rm -f $(EXEC) *.o $(LEX_OUT) $(YACC_OUT) $(YACC_HDR)
