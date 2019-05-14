#include"0cc.h"

Node* assign();
Node* stmt();
Node* equality();

int val_num = 0;

void error(int i) {
  Token* token = (Token*)tokens->data[i];
	fprintf(stderr, "不適切な入力：'%s'\n", token->input);
	exit(1);
}

int consume(int type){
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

Node* new_node_var(char* name) {
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

Node* term(){
  if (consume('(')) {
    Node* node = assign();

    if (!consume(')')) {
      fprintf(stderr, "開きかっこに対応する閉じかっこがありません\n");
      error(pos);
    }
    
    return node;
  }

  Token* token = (Token*)tokens->data[pos++];
  if (token->type == TK_IDENT) return new_node_var(token->name);
  if (token->type == TK_NUM) return new_node_num(token->value);
  
  if (token->type == TK_FUNC) {
    char* func_name = token->name;
    if (!consume('(')) {
      fprintf(stderr, "関数の開きかっこがありません\n");
      error(pos);
    }

    Vector* args = new_vector();
    if(consume(')')) return new_node_func(func_name, args);
    
    while (!consume(')')) {
      vec_push(args, (void*)equality());
      consume(',');
    }
    
    return new_node_func(func_name, args);
  }

  fprintf(stderr, "数値・変数・かっこ・関数以外のトークンです\n");
  error(pos);
  // ダミーの返り値で、実際には使われない
  Node* node = NULL;
  return node;
}

Node* unary() {
  if(consume('+')) return term();
  if(consume('-')) return new_node('-', new_node_num(0), term());
  return term();
}

Node* mul() {
  Node* node = unary();

  for(;;){
    if (consume('*')) node = new_node('*', node, unary());
    else if (consume('/')) node = new_node('/', node, unary()); 
    else return node;
  }
}

Node* add() {
  Node* node = mul();
  
  for(;;){
    if (consume('+')) node = new_node('+', node, mul());
    else if (consume('-')) node = new_node('-', node, mul()); 
    else return node;
  }
}

Node* relational() {
  Node* node = add();

  for(;;) {
    if(consume(TK_LE)) node = new_node(ND_LE, node, add());
    else if (consume(TK_GE)) node = new_node(ND_LE, add(), node);
    else if (consume('<')) node = new_node('<', node, add());
    else if (consume('>')) node = new_node('<', add(), node);
    else return node;
  }
}

Node* equality() {
  Node* node = relational();

  for(;;){
    if (consume(TK_EQ)) node = new_node(ND_EQ, node, relational());
    else if (consume(TK_NE)) node = new_node(ND_NE, node, relational());
    else return node;
  }
}

Node* assign() {
  Node* node = equality();

  while(consume('='))
    node = new_node('=', node, assign());
  
  return node;
}

Node* block_items() {
  Node* node = malloc(sizeof(Node));
  node->block_stmt = new_vector();
  Token* token = tokens->data[pos];
  node->type = ND_BLOCK;

  while(token->type != '}') {
    vec_push(node->block_stmt, stmt());
    token = tokens->data[pos];
  }

  return node;
}

Node* essential_stmt() {
  Node* node = NULL;
 
  if (consume(TK_RETURN)) {
    node = malloc(sizeof(Node));
    node->type = ND_RETURN;
    node->lhs = assign();

  } else {
    node = assign();
  }

  if(!consume(';')){
    fprintf(stderr, "';'ではないトークンです\n");
    error(pos);
  }

  return node;
}
 
Node* block() {
  Node* node = NULL;
  if (consume('{')) {
    node = block_items();
    if (!consume('}')) {
      fprintf(stderr, "開きかっこに対応する閉じかっこがありません:ブロック\n");
      error(pos);
    }

    return node;

  }

  return essential_stmt();
}

Node* stmt() {
  Node* node = malloc(sizeof(Node));
  if (consume(TK_FOR)) {
    if (consume('(')) {
      node->type = ND_FOR;

      node->lhs = assign();
      if(!consume(';')){
        fprintf(stderr, "';'ではないトークンです:forの一つ目\n");
        error(pos);
      }

      node->condition = assign();
      if(!consume(';')){
        fprintf(stderr, "';'ではないトークンです:forの二つ目\n");
        error(pos);
      }

      node->increment = assign();
      if (!consume(')')) {
        fprintf(stderr, "開きかっこに対応する閉じかっこがありません:for\n");
        error(pos);
      }
    
      node->rhs = block();
      return node;
    }

    fprintf(stderr, "forの直後に開きかっこがありません\n");
    error(pos);

  } else if (consume(TK_IF)) {
    if (consume('(')) {
      node->type = ND_IF_WITHOUT_ELSE;
      node->condition = equality();

      if (!consume(')')) {
        fprintf(stderr, "開きかっこに対応する閉じかっこがありません:if\n");
        error(pos);
      }
    
      node->lhs = block();

      if(consume(TK_ELSE)) {
        node->type = ND_IF_WITH_ELSE;
        node->rhs = block();
      }

      return node;
    }

    fprintf(stderr, "whileの後の開きかっこがありません\n");
    error(pos);
 
  } else if (consume(TK_WHILE)) {
    if (consume('(')) {
      node->type = ND_WHILE;
      node->lhs = assign();

      if (!consume(')')) {
        fprintf(stderr, "開きかっこに対応する閉じかっこがありません:while\n");
        error(pos);
      }
    
      node->rhs = block();
      return node;
    }

    fprintf(stderr, "whileの後の開きかっこがありません\n");
    error(pos);

  }

  return essential_stmt();
}

Node* def_func() {
  Node* node = malloc(sizeof(Node));
  Token* token = tokens->data[pos];

  if (token->type == TK_FUNC) {
    node->type = ND_DEFFUNC;
    node->name = token->name;
    pos++;

    if (!consume('(')) {
      fprintf(stderr, "関数定義の開きかっこがありません\n");
      error(pos);
    }

    Vector* args = new_vector();
    
    token = (Token*)tokens->data[pos];
    while (token->type == TK_IDENT) {
      vec_push(args, (void*)token->name);
      pos++;
      token = (Token*)tokens->data[pos];
      
      if(token->type == ',') {
        pos++;
        token = (Token*)tokens->data[pos];
      }
    }

    node->args = args; 

    if(!consume(')')) {
      fprintf(stderr, "開きかっこに対応する閉じかっこがありません：関数定義\n");
      error(pos);
    }

  }

  return node; 
}

void program(){
  int i = 0;
  int j;
  Token* token = tokens->data[pos];

  while (token->type != TK_EOF) {
    code[i][0] = def_func();
    token = tokens->data[pos];

    j = 1;
    if (!consume('{')) {
      fprintf(stderr, "開き中かっこがありません：関数定義\n");
      error(pos);
    }

    while (token->type != '}') {
      if(token->type == TK_EOF) {
        fprintf(stderr, "関数定義の最後のかっこがありません\n");
        error(pos);
      }

      code[i][j++] = stmt();
      token = tokens->data[pos];
    }

    token = tokens->data[++pos];
    i++;
  }

  code[i][j] = NULL;
}

