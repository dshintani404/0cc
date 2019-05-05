#include"0cc.h"

void gen_lval(Node* node, Map* map) {
  if (node->type != ND_IDENT){
    fprintf(stderr, "代入の左辺値が変数ではありません\n");
    exit(1);
  }

  int offset = (int)map_get(map, node->name);
  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", offset);
  printf("  push rax\n");
}

void gen(Node* node, Map* map){
  if(node->type == ND_FOR) {
    gen(node->lhs, map);
    printf("  .LbeginXXX:\n");
    gen(node->condition, map);
    printf("    pop rax\n");
    printf("    cmp rax, 0\n");
    printf("    je .LendXXX\n");
    gen(node->rhs, map);
    gen(node->increment, map);
    printf("    jmp .LbeginXXX\n");
    printf("  .LendXXX:\n");
    return;
  }

  if(node->type == ND_WHILE) {
    printf("  .LbeginXXX:\n");
    gen(node->lhs, map);
    printf("    pop rax\n");
    printf("    cmp rax, 0\n");
    printf("    je .LendXXX\n");
    gen(node->rhs, map);
    printf("    jmp .LbeginXXX\n");
    printf("  .LendXXX:\n");
    return;
  }

  if(node->type == ND_RETURN) {
    gen(node->lhs, map);
    printf("  pop rax\n");
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return;
  }

  if(node->type == ND_NUM){
    printf("  push %d\n", node->value);
    return;
  }

  if(node->type == ND_IDENT) {
    gen_lval(node, map);
    printf("  pop rax\n");
    printf("  mov rax, [rax]\n");
    printf("  push rax\n");
    return;
  }

  if (node->type == '=') {
    gen_lval(node->lhs, map);
    gen(node->rhs, map);

    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  mov [rax], rdi\n");
    printf("  push rdi\n");
    return;
  }

  gen(node->lhs, map);
  gen(node->rhs, map);
  
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
}

