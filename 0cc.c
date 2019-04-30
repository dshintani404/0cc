#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include<string.h>

int pos = 0;

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
    char name;
} Node;

typedef struct {
	int type;
	char* input;
	int value;
} Token;

typedef struct {
  void** data;
  int capacity;
  int len;
} Vector;

Vector* new_vector() {
  Vector* vec = malloc(sizeof(Vector));
  vec->data = malloc(sizeof(void*) * 16);
  vec->capacity = 16;
  vec->len = 0;
  return vec;
}

void vec_push(Vector* vec, void* elem) {
  if (vec->capacity == vec->len) {
    vec->capacity *= 2;
    vec->data = realloc(vec->data, sizeof(void*) * vec->capacity);
  }
  vec->data[vec->len++] = elem;
}

void runtest();

Node* code[100];

Node* equality();

int is_alnum(char c) {
  return ('a' <= c && c <= 'z') ||  ('A' <= c && c <= 'Z') ||  ('0' <= c && c <= '9') ||  (c == '_'); 
}

void tokenize(Vector* tokens, char* p){
	while(*p) {
	  Token* token = malloc(sizeof(Token));

    if(isspace(*p)) {
			p++;
			continue;
		}

    if (strncmp(p, "return", 6) == 0 && !is_alnum(p[6])) {
      token->type = TK_RETURN;
      token->input = p;
      vec_push(tokens, token);
	    p = p + 6;
      continue;
    }

    if (strncmp(p, "==", 2) == 0) {
      token->type = TK_EQ;
      token->input = p;
      vec_push(tokens, token);
	    p = p + 2;
      continue;
    }

    if (strncmp(p, "!=", 2) == 0) {
      token->type = TK_NE;
      token->input = p;
      vec_push(tokens, token);
	    p = p + 2;
      continue;
    }

    if (strncmp(p, "<=", 2) == 0) {
      token->type = TK_LE;
      token->input = p;
      vec_push(tokens, token);
	    p = p + 2;
      continue;
    }

    if (strncmp(p, ">=", 2) == 0) {
      token->type = TK_GE;
      token->input = p;
      vec_push(tokens, token);
	    p = p + 2;
      continue;
    }

		if( *p=='+' || *p=='-' || *p=='*' || *p=='/'|| *p=='(' || *p==')'
        || *p==';' || *p=='=' || *p=='<'|| *p=='>' ) {
      token->type = *p;
			token->input = p;
      vec_push(tokens, token);
			p++;
			continue;
		}

    if(isdigit(*p)) {
			token->type = TK_NUM;
			token->input = p;
			token->value = strtol(p, &p, 10);
      vec_push(tokens, token);
			continue;	
		}

    if ('a' <= *p && *p <= 'z') {
      token->type = TK_IDENT;
      token->input = p;
      vec_push(tokens, token);
      p++;
      continue;
    }

		fprintf(stderr, "トークナイズできません: %s\n", p);
        exit(1);
	}
	
  Token* token = malloc(sizeof(Token));
  
	token->type = TK_EOF;
  token->input = p;
  vec_push(tokens, token);
}	

void error(Vector* tokens, int i) {
  Token* token = (Token*)tokens->data[i];
	fprintf(stderr, "不適切な入力：'%s'\n", token->input);
	exit(1);
}

int consume(Vector* tokens, int type){
  Token* token = (Token*)tokens->data[pos];
  if (token->type != type) return 0;
  
  pos++;
  return 1;
}

Node* new_node(int type, Node* lhs, Node* rhs) {
    Node* node = malloc(sizeof(Node));
    if (node != NULL){
        node->type = type;
        node->lhs = lhs;
        node->rhs = rhs;
    }
    return node;
}

Node* new_node_num(int value) {
    Node* node = malloc(sizeof(Node));
    if (node != NULL){
        node->type = ND_NUM;
        node->value = value;
    }
    return node;
}

Node* new_node_var(int value) {
    Node* node = malloc(sizeof(Node));
    if (node != NULL){
        node->type = ND_IDENT;
        node->value = value;
    }
    return node;
}

Node* term(Vector* tokens){
  if (consume(tokens, '(')) {
    Node* node = equality(tokens);

    if(!consume(tokens, ')')) {
      fprintf(stderr, "開きかっこに対応する閉じかっこがありません\n");
      error(tokens, pos);
    }
   
    return node;
  }

  Token* token = (Token*)tokens->data[pos++];
  if (token->type == TK_IDENT) return new_node_var(token->value);
  if(token->type == TK_NUM) return new_node_num(token->value);

  fprintf(stderr, "数値・変数・かっこ以外のトークンです\n");
  return new_node_num(token->value);
}

Node* unary(Vector* tokens) {
  if(consume(tokens, '+')) return term(tokens);
  if(consume(tokens, '-')) return new_node('-', new_node_num(0), term(tokens));
  return term(tokens);
}

Node* mul(Vector* tokens){
  Node* node = unary(tokens);

  for(;;){
    if (consume(tokens, '*')) node = new_node('*', node, unary(tokens));
    else if (consume(tokens, '/')) node = new_node('/', node, unary(tokens)); 
    else return node;
  }
}

Node* add(Vector* tokens){
  Node* node = mul(tokens);
  
  for(;;){
    if (consume(tokens, '+')) node = new_node('+', node, mul(tokens));
    else if (consume(tokens, '-')) node = new_node('-', node, mul(tokens)); 
    else return node;
  }
}

Node* relational(Vector* tokens) {
  Node* node = add(tokens);

  for(;;) {
    if(consume(tokens, TK_LE)) node = new_node(ND_LE, node, add(tokens));
    else if (consume(tokens, TK_GE)) node = new_node(ND_LE, add(tokens), node);
    else if (consume(tokens, '<')) node = new_node('<', node, add(tokens));
    else if (consume(tokens, '>')) node = new_node('<', add(tokens), node);
    else return node;
  }
}

Node* equality(Vector* tokens) {
  Node* node = relational(tokens);

  for(;;){
    if (consume(tokens, TK_EQ)) node = new_node(ND_EQ, node, relational(tokens));
    else if (consume(tokens, TK_NE)) node = new_node(ND_NE, node, relational(tokens));
    else return node;
  }
}

Node* assign(Vector* tokens) {
  Node* node = equality(tokens);

  while(consume(tokens, '='))
    node = new_node('=', node, assign(tokens));
  
  return node;
}

Node* stmt(Vector* tokens) {
  Node* node;

  if (consume(tokens, TK_RETURN)) {
    node = malloc(sizeof(Node));
    node->type = ND_RETURN;
    node->lhs = assign(tokens);
  } else {
    node = assign(tokens);
  }

  if(!consume(tokens, ';')){
    fprintf(stderr, "';'ではないトークンです\n");
  }

  return node;
}

void program(Vector* tokens) {
  int i = 0;
  Token* token = tokens->data[pos];

  while(token->type != TK_EOF) {
    code[i++] = stmt(tokens);
    token = tokens->data[pos];
  }

  code[i] = NULL;
}

void gen_lval(Node* node) {
  if (node->type != ND_IDENT){
    fprintf(stderr, "代入の左辺値が変数ではありません\n");
    exit(1);
  }
 
  int offset = ('z' - node->name + 1) * 8;
  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", offset);
  printf("  push rax\n");
}

void gen(Node* node){
  if(node->type == ND_RETURN) {
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return;
  }

  if(node->type == ND_NUM){
    printf("  push %d\n", node->value);
    return;
  }

  if(node->type == ND_IDENT) {
    gen_lval(node);
    printf("  pop rax\n");
    printf("  mov rax, [rax]\n");
    printf("  push rax\n");
    return;
  }

  if (node->type == '=') {
    gen_lval(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  mov [rax], rdi\n");
    printf("  push rdi\n");
  }

  gen(node->lhs);
  gen(node->rhs);
  
  printf("  pop rdi\n");
  printf("  pop rax\n");
 
  switch (node->type){
    case ND_EQ:
      printf("  cmp rax, rdi\n");
      printf("  sete al\n");
      printf("  movzx rax, al\n");
      break;

    case ND_NE:
      printf("  cmp rax, rdi\n");
      printf("  setne al\n");
      printf("  movzx rax, al\n");
      break;

    case ND_LE:
      printf("  cmp rax, rdi\n");
      printf("  setle al\n");
      printf("  movzx rax, al\n");
      break;
  
    case '<':
      printf("  cmp rax, rdi\n");
      printf("  setl al\n");
      printf("  movzx rax, al\n");
      break;

    case '+':
      printf("  add rax, rdi\n");
      break;

    case '-':
      printf("  sub rax, rdi\n");
      break;

    case '*':
      printf("  mul rdi\n");
      break;

    case '/':
      printf("  mov rdx, 0\n");
      printf("  div rdi\n");
  }
    
  printf("  push rax\n");
}

int main(int argc, char **argv) {
	if (argc != 2){
		fprintf(stderr, "invalid numbers of args\n");
		return 1;
	}

  if (strncmp(argv[1], "-test", 5) == 0) {
    runtest();
    return 0;
  }

	Vector* tokens = new_vector();
  tokenize(tokens, argv[1]);
 
  program(tokens);

	printf(".intel_syntax noprefix\n");
	printf(".global _main\n");
	printf("_main:\n");

  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, 208\n");
  
  for(int i=0; code[i]; i++) {
    gen(code[i]);
    printf("  pop rax\n");
  }
  
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
	
  return 0;
}

// test for vector
int expect(int line, int expected, int actual) {
  if(expected == actual) return 0;
  fprintf(stderr, "%d: %d expected, but got %d\n", line, expected, actual);
  return 1;
} 

void runtest() {
  Vector* vec = new_vector();
  expect(__LINE__, 0, vec->len);

  for(int i=0;i<100;i++) vec_push(vec, (void*)(size_t)i);
  
  expect(__LINE__, 100, vec->len);
  expect(__LINE__, 0, (int)vec->data[0]);
  expect(__LINE__, 50, (int)vec->data[50]);
  expect(__LINE__, 99, (int)vec->data[99]);

  printf("OK\n");
}

