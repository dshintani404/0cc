#include"0cc.h"

Node* assign();
Node* stmt();
Node* equality();

int val_num;

void error(char* message) {
  Token* token = (Token*)tokens->data[pos];
	fprintf(stderr, "%s：'%s'\n", message, token->input);
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
  node->type = type;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node* new_node_num(int value) {
  Node* node = malloc(sizeof(Node));
  node->type = ND_NUM;
  node->value = value;
  return node;
}

Node* new_node_defvar(char* name, Type* type) {
  Node* node = malloc(sizeof(Node));
  node->type = ND_IDENT;
  node->name = name;
  node->dtype = type;

  void* offset = map_get(map, name);

  if (offset == NULL) {
    val_num++;
    map_put(map, name, (void*)(size_t)(val_num * 8), type);
  } else {
    map_put(map, name, offset, type);
  }

  return node;
}

Node* new_node_var(char* name) {
  void* offset = map_get(map, name);
  if (offset == NULL) error("未定義の変数です");

  Type* type = map_get_type(map, name);
  if (type == NULL) error("変数型が未定義です");

  map_put(map, name, offset, type);

  Node* node = malloc(sizeof(Node));
  node->type = ND_IDENT;
  node->name = name;
  node->dtype = type;

  return node;
}

Node* new_node_func(char* name, Vector* args) {
  Node* node = malloc(sizeof(Node));
  node->type = ND_FUNC;
  node->name = name;
  node->args = args; 
  return node;
}

Type* ptr_to(Type* base) {
  Type* type = malloc(sizeof(Type));
  type->type = PTR;
  type->pointer_of = base;
  return type;
}

Node* term(){
  Node* node;
  if (consume('(')) {
    node = assign();
    if (!consume(')')) error("開きかっこに対応する閉じかっこがありません");
    return node;
  }

  Token* token;

  if (consume(TK_INT)) {
    Type* type = malloc(sizeof(Type));
    type->type = INT;
    type->pointer_of = NULL;

    while(consume('*')) {
      type = ptr_to(type);
    }

    token = (Token*)tokens->data[pos++];
    if (token->type == TK_IDENT) return new_node_defvar(token->name, type);
    else error("宣言の後の変数がありません");
  }

  token = (Token*)tokens->data[pos++];
  if (token->type == TK_IDENT) return new_node_var(token->name);
  if (token->type == TK_NUM) return new_node_num(token->value);
  
  if (token->type == TK_FUNC) {
    char* func_name = token->name;
    if (!consume('(')) error("関数の開きかっこがありません");

    Vector* args = new_vector();
    if(consume(')')) return new_node_func(func_name, args);
    
    while (!consume(')')) {
      vec_push(args, (void*)equality());
      consume(',');
    }
    
    return new_node_func(func_name, args);
  }

  error("数値・変数・かっこ・関数以外のトークンです");

  // ダミーの返り値で、実際には使われない
  return NULL;
}

Node* new_expr(int type, Node* expr) {
  Node* node = malloc(sizeof(Node));
  node->type = type;
  node->expr = expr;
  return node;
}

Node* unary() {
  if(consume('+')) return term();
  if(consume('-')) return new_node('-', new_node_num(0), term());
  if(consume('*')) return new_expr(ND_DEREF, unary());
  if(consume('&')) return new_expr(ND_ADDR, unary());
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

  while(consume('=')) node = new_node('=', node, assign());
  
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

  if(!consume(';')) error("';'ではないトークンです");

  return node;
}
 
Node* block() {
  Node* node = NULL;
  if (consume('{')) {
    node = block_items();

    if (!consume('}')) error("開きかっこに対応する閉じかっこがありません:ブロック");

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
      if(!consume(';')) error("';'ではないトークンです:forの一つ目");

      node->condition = assign();
      if(!consume(';')) error("';'ではないトークンです:forの二つ目");

      node->increment = assign();
      if (!consume(')')) error("開きかっこに対応する閉じかっこがありません:for");
    
      node->rhs = block();
      return node;
    }

    error("forの直後に開きかっこがありません");

  } else if (consume(TK_IF)) {

    if (consume('(')) {
      node->type = ND_IF_WITHOUT_ELSE;
      node->condition = equality();

      if (!consume(')')) error("開きかっこに対応する閉じかっこがありません:if");
    
      node->lhs = block();

      if(consume(TK_ELSE)) {
        node->type = ND_IF_WITH_ELSE;
        node->rhs = block();
      }

      return node;
    }

    error("whileの後の開きかっこがありません");
 
  } else if (consume(TK_WHILE)) {
    if (consume('(')) {
      node->type = ND_WHILE;
      node->lhs = assign();

      if (!consume(')')) error("開きかっこに対応する閉じかっこがありません:while");
    
      node->rhs = block();
      return node;
    }

    error("whileの後の開きかっこがありません");
  }

  return essential_stmt();
}

Node* def_func() {
  Node* node = malloc(sizeof(Node));

  if (!consume(TK_INT)) error("型がありません：関数定義");
  
  Token* token = tokens->data[pos++];
    
  if (token->type != TK_FUNC) error("関数定義がありません");
    
  node->type = ND_DEFFUNC;
  node->name = token->name;

  if (!consume('(')) error("関数定義の開きかっこがありません");

  Vector* args = new_vector();
    
  token = (Token*)tokens->data[pos];
  while (token->type == TK_INT) {
    Type* type = malloc(sizeof(Type));
    type->type = INT;
    type->pointer_of = NULL;

    token = (Token*)tokens->data[++pos];
    if(token->type != TK_IDENT) error("引数の型の後に変数がありません");
    
    val_num++;
    map_put(map, token->name, (void*)(size_t)(val_num * 8), (void*)type);

    vec_push(args, (void*)token->name);
    token = (Token*)tokens->data[++pos];
      
    if(token->type == ',') token = (Token*)tokens->data[++pos];
  }

  node->args = args; 

  if(!consume(')')) error("開きかっこに対応する閉じかっこがありません：関数定義");
  
  return node; 
}

void program(){
  int i = 0;
  int j;
  Token* token = tokens->data[pos];

  while (token->type != TK_EOF) {
    val_num = 0;
    code[i][0] = def_func();
    token = tokens->data[pos];
    
    if (!consume('{')) error("開き中かっこがありません：関数定義");

    j = 1;
    while (token->type != '}') {
      if(token->type == TK_EOF) error("関数定義の最後のかっこがありません");

      code[i][j++] = stmt();
      token = tokens->data[pos];
    }

    token = tokens->data[++pos];
    i++;
  }

  code[i][j] = NULL;
}

