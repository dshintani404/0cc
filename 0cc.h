#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include<string.h>

enum {
	TK_NUM = 256,
  TK_EQ, TK_NE, TK_LE, TK_GE, TK_IDENT, TK_EOF, TK_RETURN,
  TK_WHILE, TK_FOR, TK_IF, TK_ELSE, TK_FUNC,
  TK_INT
};

enum {
  ND_NUM = 256,
  ND_EQ, ND_NE, ND_LE, ND_IDENT, ND_RETURN,
  ND_WHILE, ND_FOR, ND_IF_WITH_ELSE, ND_IF_WITHOUT_ELSE,
  ND_BLOCK, ND_FUNC, ND_DEFFUNC, ND_FUNCSTMT,
  ND_INT, ND_DEREF, ND_ADDR
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
  struct Node* expr;
  int value; // numbers
  char* name; // variables, func
  struct Node* condition; // for, if
  struct Node* increment; // for
  Vector* block_stmt;
  Vector* args; // func
  Vector* func_stmt; // func_def
  int dtype; // data type for func and var
} Node;

typedef struct Type {
  enum {INT, PTR} type;
  struct Type* pointer_of;
} Type;

typedef struct {
  Vector* keys;
  Vector* vals;
  Vector* types;
} Map;

extern Node* code[10][100];
extern int pos;
extern Map* map;
extern Vector* tokens;

void gen_lval(Node* node);
void gen(Node* node);

Vector* new_vector();
void vec_push(Vector* vec, void* elem);

Map* new_map();
void map_put(Map* map, char* key, void* val, Type* type);
void* map_get(Map* map, char* key);
void* map_get_type(Map* map, char* key);

void tokenize(char* p);
void program();
void runtest();
