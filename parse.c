#include"0cc.h"

Node* assign(Vector* tokens, Map* map);
Node* stmt(Vector* tokens, Map* map);

int val_num = 0;

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

Node* new_node_func(char* name, Vector* args) {
    Node* node = malloc(sizeof(Node));
    if (node != NULL){
        node->type = ND_FUNC;
        node->name = name;
        node->args = args; 
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
  
  if (token->type == TK_FUNC) {
    char* func_name = token->name;
    if (!consume(tokens, '(')) {
      fprintf(stderr, "関数の開きかっこがありません\n");
      error(tokens, pos);
    }

    Vector* args = new_vector();
    if(consume(tokens, ')')) return new_node_func(func_name, args);
    
    token = (Token*)tokens->data[pos];
    while (token->type == TK_NUM) {
      vec_push(args, (void*)(size_t)token->value);
      pos++;
      token = (Token*)tokens->data[pos];
      if(token->type == ',') {
        pos++;
        token = (Token*)tokens->data[pos];
      }
    }

    if(!consume(tokens, ')')) {
      fprintf(stderr, "開きかっこに対応する閉じかっこがありません：関数\n");
      error(tokens, pos);
    }
    
    return new_node_func(func_name, args);
  }

  fprintf(stderr, "数値・変数・かっこ・関数以外のトークンです\n");
  error(tokens, pos);
  // ダミーの返り値で、実際には使われない
  Node* node = NULL;
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

Node* block_items(Vector* tokens, Map* map) {
  Node* node = malloc(sizeof(Node));
  node->block_stmt = new_vector();
  Token* token = tokens->data[pos];
  node->type = ND_BLOCK;

  while(token->type != '}') {
    vec_push(node->block_stmt, stmt(tokens, map));
    token = tokens->data[pos];
  }

  return node;
}

Node* stmt(Vector* tokens, Map* map) {
  Node* node = NULL;
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

  } else if (consume(tokens, TK_IF)) {
    if (consume(tokens, '(')) {
      node = malloc(sizeof(Node));
      node->type = ND_IF_WITHOUT_ELSE;
      node->condition = equality(tokens, map);

      if (!consume(tokens, ')')) {
        fprintf(stderr, "開きかっこに対応する閉じかっこがありません:if\n");
        error(tokens, pos);
      }
    
      node->lhs = stmt(tokens, map);

      if(consume(tokens, TK_ELSE)) {
        node->type = ND_IF_WITH_ELSE;
        node->rhs = stmt(tokens, map);
      }

      return node;
    }

    fprintf(stderr, "whileの後の開きかっこがありません\n");
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

  } else if (consume(tokens, '{')) {
    node = block_items(tokens, map);
    if (!consume(tokens, '}')) {
      fprintf(stderr, "開きかっこに対応する閉じかっこがありません:ブロック\n");
      error(tokens, pos);
    }

    return node;

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


