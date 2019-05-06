#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include<string.h>

enum {
	TK_NUM = 256,
  TK_EQ, TK_NE, TK_LE, TK_GE, TK_IDENT, TK_EOF, TK_RETURN,
  TK_WHILE, TK_FOR, TK_IF, TK_ELSE
};

enum {
  ND_NUM = 256,
  ND_EQ, ND_NE, ND_LE, ND_IDENT, ND_RETURN,
  ND_WHILE, ND_FOR, ND_IF_WITH_ELSE, ND_IF_WITHOUT_ELSE, ND_BLOCK
};

typedef struct {
	int type;
	int value;
  char* name;
  char* input;
} Token;

typedef struct {
  void** data;
  int capacity;
  int len;
} Vector;

typedef struct Node {
  int type;
  struct Node* lhs;
  struct Node* rhs;
  int value; // numbers
  char* name; // variables
  struct Node* condition; // for, if
  struct Node* increment; // for
  Vector* block_stmt;
} Node;

typedef struct {
  Vector* keys;
  Vector* vals;
} Map;

extern Node* code[100];
extern int pos;

void gen_lval(Node* node, Map* map);
void gen(Node* node, Map* map);

Vector* new_vector();
void vec_push(Vector* vec, void* elem);

Map* new_map();
void map_put(Map* map, char* key, void* val);
void* map_get(Map* map, char* key);

void tokenize(Vector* tokens, char* p);
void program(Vector* tokens, Map* map);
void runtest();
