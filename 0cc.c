#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include<string.h>

int pos = 0;

enum {
	TK_NUM = 256,
	TK_EOF
};

enum {
    ND_NUM = 256
};

typedef struct Node {
    int type;
    struct Node* lhs;
    struct Node* rhs;
    int value;
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

void runtest();

Node* add();
Node* mul();
Node* term();

void tokenize(Vector* tokens, char* p){
	while(*p) {
	  Token* token = malloc(sizeof(Token));

    if(isspace(*p)) {
			p++;
			continue;
		}

		if(*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')') {
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

		fprintf(stderr, "トークナイズできません: %s\n", p);
        exit(1);
	}
	
  Token* token = malloc(sizeof(Token));
  
	token->type = TK_EOF;
	token->input = "";
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

Node* term(Vector* tokens){
  if (consume(tokens, '(')) {
    Node* node = add(tokens);
    if(!consume(tokens, ')')) error(tokens, pos);
    return node;
  }
  Token* token = (Token*)tokens->data[pos++];	
  return new_node_num(token->value);
}

Node* mul(Vector* tokens){
  Node* node = term(tokens);

  for(;;){
    if (consume(tokens, '*')) node = new_node('*', node, term(tokens));
    else if (consume(tokens, '/')) node = new_node('/', node, term(tokens)); 
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

void gen(Node* node){
  if(node->type == ND_NUM){
    printf("    push %d\n", node->value);
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("    pop rdi\n");
  printf("    pop rax\n");

  switch (node->type){
    case '+':
      printf("    add rax, rdi\n");
      break;
    case '-':
      printf("    sub rax, rdi\n");
      break;
    case '*':
      printf("    mul rdi\n");
      break;
    case '/':
      printf("    mov rdx, 0\n");
      printf("    div rdi\n");
  }
    
  printf("    push rax\n");
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
  
  Node* node = add(tokens);

	printf(".intel_syntax noprefix\n");
	printf(".global _main\n");
	printf("_main:\n");

  gen(node);
    
  printf("    pop rax\n");
  printf("	ret\n");
	
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

