#include"0cc.h"

Node* equality(Vector* tokens, Map* map);

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

    if (is_alnum(*p)) {
      p++;
      int cnt = 1;
      while (is_alnum(*p)) {
        cnt++;
        p++;
      }

      token->type = TK_IDENT;
      token->name = malloc(sizeof(char)*cnt);
      strncpy(token->name, p-cnt, cnt);
      token->input = p-cnt;
      vec_push(tokens, token);
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

int val_num = 0;
Node* new_node_var(char* name, Map* map) {
  void* offset = map_get(map, name);
  if (offset == NULL) {
    val_num++;
    map_put(map, name, (void*)(size_t)(val_num * 8));
  } else {
    map_put(map, name, offset);
  }

  Node* node = malloc(sizeof(Node));
  if (node != NULL) {
    node->type = ND_IDENT;
    node->name = name;
  }
  return node;
}

Node* term(Vector* tokens, Map* map){
  if (consume(tokens, '(')) {
    Node* node = equality(tokens, map);

    if(!consume(tokens, ')')) {
      fprintf(stderr, "開きかっこに対応する閉じかっこがありません\n");
      error(tokens, pos);
    }
   
    return node;
  }

  Token* token = (Token*)tokens->data[pos++];
  if (token->type == TK_IDENT) return new_node_var(token->name, map);
  if(token->type == TK_NUM) return new_node_num(token->value);

  fprintf(stderr, "数値・変数・かっこ以外のトークンです\n");
  return new_node_num(token->value);
}

Node* unary(Vector* tokens, Map* map) {
  if(consume(tokens, '+')) return term(tokens, map);
  if(consume(tokens, '-')) return new_node('-', new_node_num(0), term(tokens, map));
  return term(tokens, map);
}

Node* mul(Vector* tokens, Map* map){
  Node* node = unary(tokens, map);

  for(;;){
    if (consume(tokens, '*')) node = new_node('*', node, unary(tokens, map));
    else if (consume(tokens, '/')) node = new_node('/', node, unary(tokens, map)); 
    else return node;
  }
}

Node* add(Vector* tokens, Map* map){
  Node* node = mul(tokens, map);
  
  for(;;){
    if (consume(tokens, '+')) node = new_node('+', node, mul(tokens, map));
    else if (consume(tokens, '-')) node = new_node('-', node, mul(tokens, map)); 
    else return node;
  }
}

Node* relational(Vector* tokens, Map* map) {
  Node* node = add(tokens, map);

  for(;;) {
    if(consume(tokens, TK_LE)) node = new_node(ND_LE, node, add(tokens, map));
    else if (consume(tokens, TK_GE)) node = new_node(ND_LE, add(tokens, map), node);
    else if (consume(tokens, '<')) node = new_node('<', node, add(tokens, map));
    else if (consume(tokens, '>')) node = new_node('<', add(tokens, map), node);
    else return node;
  }
}

Node* equality(Vector* tokens, Map* map) {
  Node* node = relational(tokens, map);

  for(;;){
    if (consume(tokens, TK_EQ)) node = new_node(ND_EQ, node, relational(tokens, map));
    else if (consume(tokens, TK_NE)) node = new_node(ND_NE, node, relational(tokens, map));
    else return node;
  }
}

Node* assign(Vector* tokens, Map* map) {
  Node* node = equality(tokens, map);

  while(consume(tokens, '='))
    node = new_node('=', node, assign(tokens, map));
  
  return node;
}

Node* stmt(Vector* tokens, Map* map) {
  Node* node;

  if (consume(tokens, TK_RETURN)) {
    node = malloc(sizeof(Node));
    node->type = ND_RETURN;
    node->lhs = assign(tokens, map);
  } else {
    node = assign(tokens, map);
  }

  if(!consume(tokens, ';')){
    fprintf(stderr, "';'ではないトークンです\n");
  }

  return node;
}

void program(Vector* tokens, Map* map) {
  int i = 0;
  Token* token = tokens->data[pos];

  while(token->type != TK_EOF) {
    code[i++] = stmt(tokens, map);
    token = tokens->data[pos];
  }

  code[i] = NULL;
}


