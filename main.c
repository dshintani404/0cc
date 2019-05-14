#include"0cc.h"

int pos = 0;
Node* code[10][100];
Map* map;
Vector* tokens;

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "invalid numbers of args\n");
    return 1;
  }

  if (strncmp(argv[1], "-test", 5) == 0) {
    runtest();
    return 0;
  }
  
  tokens = new_vector();
  map = new_map();
  tokenize(argv[1]);
 
  program();

  printf(".intel_syntax noprefix\n");
  printf(".global ");
  for (int i=0; code[i][0];i++) {
    if (i > 0) printf(", ");
  	printf("_%s", code[i][0]->name);
  }
  printf("\n\n");

  for (int i=0; code[i][0]; i++){
    gen(code[i][0]);
    for (int j=1; code[i][j]; j++) {
       gen(code[i][j]);
       printf("  pop rax\n");
    }
  }

  printf("  leave\n");
  printf("  ret\n");
 
  return 0;
}

