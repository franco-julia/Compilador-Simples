# Compilador Educacional com AST, Análise Semântica e Geração de Código MIPS

## Descrição

Este projeto implementa um compilador para uma linguagem imperativa simples, contemplando as principais etapas do processo de compilação:

* Análise léxica (Flex)
* Análise sintática (Bison)
* Construção da Árvore Sintática Abstrata (AST)
* Análise semântica
* Geração de código em Assembly MIPS

O objetivo é demonstrar, de forma modular e didática, como um compilador é estruturado, desde o código-fonte até a geração de código executável.

---

## Estrutura do Projeto

```
.
├── ast.c / ast.h           → Construção e manipulação da AST
├── semantic.c / semantic.h → Análise semântica
├── symtab.c / symtab.h     → Tabela de símbolos com escopo
├── codegen.c / codegen.h   → Geração de código MIPS
├── g-v1.l                  → Analisador léxico (Flex)
├── g-v1.y                  → Analisador sintático (Bison)
├── Makefile                → Automação da compilação
```

---

## Árvore Sintática Abstrata (AST)

A AST representa a estrutura lógica do programa.

Cada nó contém:

* Tipo do nó (`NodeType`)
* Tipo de valor (`ValueType`)
* Filhos (`esq`, `dir`, `terceiro`)
* Encadeamento (`proximo`)

Exemplo de nós:

* Programa
* Bloco
* Declaração
* Operações
* Controle (`if`, `while`)
* Entrada/Saída (`leia`, `escreva`) 

---

## Análise Semântica

Responsável por garantir a consistência do programa.

Validações realizadas:

* Variáveis declaradas antes do uso
* Tipos compatíveis em expressões
* Operações válidas (aritméticas, lógicas e relacionais)
* Escopo de variáveis

---

## Tabela de Símbolos

Gerencia:

* Declaração de variáveis
* Tipos (`int`, `car`)
* Escopos aninhados

Funcionalidades:

* Inserção (`symtab_insert`)
* Busca (`symtab_lookup`)
* Controle de escopo (push/pop)

---

## Geração de Código

O compilador gera código Assembly MIPS.

Implementação principal: 

### Recursos suportados:

* Variáveis (`.word`, `.byte`)
* Expressões aritméticas e lógicas
* Estruturas de controle:

  * `if / else`
  * `while`
* Entrada e saída:

  * leitura (`syscall 5` e `12`)
  * escrita (`syscall 1`, `4`, `11`)

### Exemplo de saída MIPS:

```asm
.data
var_x_0: .word 0

.text
.globl main
main:
li $t0, 10
sw $t0, var_x_0
li $v0, 10
syscall
```

---

## Fluxo do Compilador

```
Código Fonte
   ↓
Análise Léxica (Flex)
   ↓
Análise Sintática (Bison)
   ↓
AST
   ↓
Análise Semântica
   ↓
Geração de Código (MIPS)
   ↓
Arquivo .asm
```

---

## Como Compilar e Executar

### 1. Gerar analisadores

```bash
bison -d g-v1.y
flex g-v1.l
```

### 2. Compilar o projeto

```bash
gcc *.c -o compilador
```

### 3. Executar

```bash
./compilador entrada.txt
```

### 4. Executar o código MIPS

Use simuladores como:

* MARS
* SPIM

---

## Exemplos de Linguagem

### Entrada:

```
int x;
x = 10;
escreva x;
```

### Saída (MIPS):

```asm
li $t0, 10
sw $t0, var_x_0
move $a0, $t0
li $v0, 1
syscall
```

---

## Autor

Projeto desenvolvido para fins acadêmicos, com foco em:

* Compiladores
* Estruturas de linguagem
* Sistemas de tradução de código

---

Alexandre Moura Caldeira
Júlia Rodrigues Franco
