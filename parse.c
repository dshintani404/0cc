#include"0cc.h"

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


