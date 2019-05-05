#include"0cc.h"

int pos = 0;
Node* code[100];

int main(int argc, char **argv) {
	if (argc != 2){
		fprintf(stderr, "invalid numbers of args\n");
		return 1;
	}

  if (strncmp(argv[1], "-test", 5) == 0) {
    runtest();
    return 0;
  }

	Vector* tokens = new_vector();
  Map* map = new_map();
  tokenize(tokens, argv[1]);
 
  program(tokens, map);

	printf(".intel_syntax noprefix\n");
	printf(".global _main\n");
	printf("_main:\n");

  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, 208\n");  
  
  for(int i=0; code[i]; i++) {
    gen(code[i], map);
    printf("  pop rax\n");
  }
  
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
	
  return 0;
}

