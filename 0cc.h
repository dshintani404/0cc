#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include<string.h>

enum {
	TK_NUM = 256,
  TK_EQ, TK_NE, TK_LE, TK_GE, TK_IDENT, TK_EOF, TK_RETURN
};

enum {
  ND_NUM = 256,
  ND_EQ, ND_NE, ND_LE, ND_IDENT, ND_RETURN
};

typedef struct Node {
  int type;
  struct Node* lhs;
  struct Node* rhs;
  int value;
  char* name;
} Node;

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
