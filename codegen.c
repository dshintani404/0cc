#include"0cc.h"

int stackpoint = 0;
int cnt_lend = 0;
int cnt_lelse = 0;
int cnt_lbegin = 0;

void gen_lval(Node* node) {
  if (node->type == ND_DEREF) gen(node->expr);
  else {
    int offset = (int)map_get(map, node->name);
    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", offset);
    printf("  push rax\n");
    stackpoint += 8;
  }
}

void gen(Node* node){
  char args_reg[6][4] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

  if (node->type == ND_DEFFUNC) {
    stackpoint = 0;
    printf("_%s:\n", node->name);
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, 160\n");

    int offset;
    for(int i=0; i< node->args->len; i++) {
      offset = (int)map_get(map, node->args->data[i]);
   
      printf("  mov rax, rbp\n");
      printf("  sub rax, %d\n", offset);
      printf("  mov [rax], %s\n", args_reg[i]);
      printf("  push rdi\n");
      
      stackpoint += 8;
    }

    return;
  }

  if (node->type == ND_BLOCK) {
    for(int i=0;i<node->block_stmt->len;i++) gen(node->block_stmt->data[i]);
    return;
  }

  if (node->type == ND_IF_WITHOUT_ELSE) {
    char end_buff[10];
    sprintf(end_buff, "%d", cnt_lend);

    gen(node->condition);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je .Lend%s\n", end_buff);
    gen(node->lhs);
    printf("  .Lend%s:\n", end_buff);

    stackpoint -= 8;
    cnt_lend++;
    return;
  }

  if (node->type == ND_IF_WITH_ELSE) {
    char else_buff[10];
    char end_buff[10];
    sprintf(else_buff, "%d", cnt_lelse);
    sprintf(end_buff, "%d", cnt_lend);

    gen(node->condition);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");

    printf("  je .Lelse%s\n", else_buff);
    gen(node->lhs);
    printf("  jmp .Lend%s\n", end_buff);
    printf("  .Lelse%s:\n", else_buff);
    gen(node->rhs);
    printf("  .Lend%s:\n", end_buff);
    
    stackpoint -= 8;
    cnt_lelse++;
    cnt_lend++;
    return;
  }

  if(node->type == ND_FOR) {
    char begin_buff[10];
    char end_buff[10];
    sprintf(begin_buff, "%d", cnt_lbegin);
    sprintf(end_buff, "%d", cnt_lend);

    gen(node->lhs);
    printf("  .Lbegin%s:\n", begin_buff);
    gen(node->condition);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je .Lend%s\n", end_buff);
    gen(node->rhs);
    gen(node->increment);
    printf("  jmp .Lbegin%s\n", begin_buff);
    printf("  .Lend%s:\n", end_buff);

    stackpoint -= 8;
    cnt_lbegin++;
    cnt_lend++;
    return;
  }

  if(node->type == ND_WHILE) {
    char begin_buff[10];
    char end_buff[10];
    sprintf(begin_buff, "%d", cnt_lbegin);
    sprintf(end_buff, "%d", cnt_lend);

    printf("  .Lbegin%s:\n", begin_buff);
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je .Lend%s\n", end_buff);
    gen(node->rhs);
    printf("  jmp .Lbegin%s\n", begin_buff);
    printf("  .Lend%s:\n", end_buff);

    stackpoint -= 8;
    cnt_lend++;
    cnt_lbegin++;
    return;
  }

  if(node->type == ND_RETURN) {
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  leave\n");
    printf("  ret\n");
    stackpoint -= 8;
    return;
  }

  if(node->type == ND_NUM){
    printf("  push %d\n", node->value);
    stackpoint += 8;
    return;
  }

  if(node->type == ND_IDENT) {
    gen_lval(node);
    printf("  pop rax\n");
    printf("  mov rax, [rax]\n");
    printf("  push rax\n");
    return;
  }

  if (node->type == ND_FUNC){
    for (int i=0; i< node->args->len; i++) {
      gen((Node*)node->args->data[i]);
      printf("  pop rax\n");
      printf("  mov %s, rax\n", args_reg[i]);
      stackpoint -=8;
    }
    
    if(stackpoint % 16 != 0) printf("  sub rsp, 8\n");
    printf("  call _%s\n", node->name);
    stackpoint = -8;

    printf("  push rax\n");
    stackpoint -= 8;
    return;
  }

  if (node->type == ND_ADDR) {
    gen_lval(node->expr);
    return;
  }

  if (node->type == ND_DEREF) {
    gen_lval(node->expr);
    printf("  pop rax\n");
    printf("  mov rax, [rax]\n");
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

    stackpoint -= 8;
    return;
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
  stackpoint += 8;
}

