#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>


int pos = 0;

enum {
	TK_NUM = 256,
	TK_EOF
};

enum {
    ND_NUM = 256
};

typedef struct Node{
    int type;
    struct Node* lhs;
    struct Node* rhs;
    int value;
} Node;

typedef struct{
	int type;
	char* input;
	int value;
} Token;

Token tokens[100];

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

Node* add();
Node* mul();
Node* term();

void tokenize(char* p){
	int i = 0;
	while(*p) {
		if(isspace(*p)) {
			p++;
			continue;
		}

		if(*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')') {
			tokens[i].type = *p;
			tokens[i].input = p;
			i++;
			p++;
			continue;
		}

		if(isdigit(*p)) {
			tokens[i].type = TK_NUM;
			tokens[i].input = p;
			tokens[i].value = strtol(p, &p, 10);
			i++;
			continue;	
		}

		fprintf(stderr, "トークナイズできません: %s\n", p);
        exit(1);
	}

	tokens[i].type = TK_EOF;
	tokens[i].input = "";
}

void error(int i) {
	fprintf(stderr, "不適切な入力：'%s'\n", tokens[i].input);
	exit(1);
}

int consume(int type){
  if (tokens[pos].type != type) return 0;
  pos++;
  return 1;
}

Node* term(){
  if (consume('(')) {
    Node* node = add();
    if(!consume(')')) error(pos);
    return node;
  }
  return new_node_num(tokens[pos++].value);
}

Node* mul(){
  Node* node = term();

  for(;;){
    if (consume('*')) node = new_node('*', node, term());
    else if (consume('/')) node = new_node('/', node, term()); 
    else return node;
  }
}

Node* add(){
  Node* node = mul();
  
  for(;;){
    if (consume('+')) node = new_node('+', node, mul());
    else if (consume('-')) node = new_node('-', node, mul()); 
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

	tokenize(argv[1]);
  
  Node* node = add();

	printf(".intel_syntax noprefix\n");
	printf(".global _main\n");
	printf("_main:\n");

  gen(node);
    
  printf("    pop rax\n");
  printf("	ret\n");
	
  return 0;
}
