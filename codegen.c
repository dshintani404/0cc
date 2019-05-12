#include"0cc.h"

int stackpoint = 0;

void gen_lval(Node* node) {
  if (node->type != ND_IDENT){
    fprintf(stderr, "代入の左辺値が変数ではありません\n");
    exit(1);
  }

  int offset = (int)map_get(map, node->name);
  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", offset);
  printf("  push rax\n");
  
  stackpoint += 8;
}

void gen(Node* node){
  if (node->type == ND_BLOCK) {
    for(int i=0;i<node->block_stmt->len;i++) {
      gen(node->block_stmt->data[i]);
    }
    return;
  }

  if (node->type == ND_IF_WITHOUT_ELSE) {
    gen(node->condition);
    printf("    pop rax\n");
    printf("    cmp rax, 0\n");
    printf("    je .LendXXX\n");
    gen(node->lhs);
    printf("  .LendXXX:\n");
    
    stackpoint -= 8;
    return;
  }

  if (node->type == ND_IF_WITH_ELSE) {
    gen(node->condition);
    printf("    pop rax\n");
    printf("    cmp rax, 0\n");
    printf("    je .LelseXXX\n");
    gen(node->lhs);
    printf("    jmp .LendXXX\n");
    printf("  .LelseXXX:\n");
    gen(node->rhs);
    printf("  .LendXXX:\n");

    stackpoint -= 8;
    return;
  }

  if(node->type == ND_FOR) {
    gen(node->lhs);
    printf("  .LbeginXXX:\n");
    gen(node->condition);
    printf("    pop rax\n");
    printf("    cmp rax, 0\n");
    printf("    je .LendXXX\n");
    gen(node->rhs);
    gen(node->increment);
    printf("    jmp .LbeginXXX\n");
    printf("  .LendXXX:\n");

    stackpoint -= 8;
    return;
  }

  if(node->type == ND_WHILE) {
    printf("  .LbeginXXX:\n");
    gen(node->lhs);
    printf("    pop rax\n");
    printf("    cmp rax, 0\n");
    printf("    je .LendXXX\n");
    gen(node->rhs);
    printf("    jmp .LbeginXXX\n");
    printf("  .LendXXX:\n");

    stackpoint -= 8;
    return;
  }

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

  if(node->type == ND_FUNC){
    if (node->args->len > 0) {
      printf("  mov rdi, %d\n", (int)node->args->data[0]);
      if (node->args->len > 1) {
        printf("  mov rsi, %d\n", (int)node->args->data[1]);
      }
    }

    if(stackpoint % 16 != 0) printf("  sub rsp, 8\n");

    printf("  call _%s\n", node->name);
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

