#include"0cc.h"

Node* assign(Vector* tokens, Map* map);

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
    Node* node = assign(tokens, map);

    if (!consume(tokens, ')')) {
      fprintf(stderr, "開きかっこに対応する閉じかっこがありません\n");
      error(tokens, pos);
    }
    
    return node;
  }

  Token* token = (Token*)tokens->data[pos++];
  if (token->type == TK_IDENT) return new_node_var(token->name, map);
  if (token->type == TK_NUM) return new_node_num(token->value);

  fprintf(stderr, "数値・変数・かっこ以外のトークンです\n");
  error(tokens, pos);
  // ダミーの返り値で、実際には使われない
  Node* node;
  return node;
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
  if (consume(tokens, TK_FOR)) {
    if (consume(tokens, '(')) {
      node = malloc(sizeof(Node));
      node->type = ND_FOR;

      node->lhs = assign(tokens, map);
      if(!consume(tokens, ';')){
        fprintf(stderr, "';'ではないトークンです:forの一つ目\n");
        error(tokens, pos);
      }

      node->condition = assign(tokens, map);
      if(!consume(tokens, ';')){
        fprintf(stderr, "';'ではないトークンです:forの二つ目\n");
        error(tokens, pos);
      }

      node->increment = assign(tokens, map);
      if (!consume(tokens, ')')) {
        fprintf(stderr, "開きかっこに対応する閉じかっこがありません:for\n");
        error(tokens, pos);
      }
    
      node->rhs = stmt(tokens, map);
      return node;
    }

    fprintf(stderr, "forの直後に開きかっこがありません\n");
    error(tokens, pos);

  } else if (consume(tokens, TK_WHILE)) {
    if (consume(tokens, '(')) {
      node = malloc(sizeof(Node));
      node->type = ND_WHILE;
      node->lhs = equality(tokens, map);

      if (!consume(tokens, ')')) {
        fprintf(stderr, "開きかっこに対応する閉じかっこがありません:while\n");
        error(tokens, pos);
      }
    
      node->rhs = stmt(tokens, map);
      return node;
    }

    fprintf(stderr, "whileの後の開きかっこがありません\n");
    error(tokens, pos);

  } else if (consume(tokens, TK_RETURN)) {
    node = malloc(sizeof(Node));
    node->type = ND_RETURN;
    node->lhs = assign(tokens, map);

  } else {
    node = assign(tokens, map);
  }

  // 以下は制御構文以外の場合の共通処理
  if(!consume(tokens, ';')){
    fprintf(stderr, "';'ではないトークンです\n");
    error(tokens, pos);
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


