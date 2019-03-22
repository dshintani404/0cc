#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>

enum {
	TK_NUM = 256,
	TK_EOF
};

typedef struct{
	int type;
	char* input;
	int value;
} Token;

Token token[100];

void tokenize(char* p){
	int i = 0;
	while(*p) {
		if(isspace(*p)) {
			p++;
			continue;
		}

		if(*p == '+' || *p == '-') {
			token[i].type = *p;
			token[i].input = p;
			i++;
			p++;
			continue;
		}

		if(isdigit(*p)) {
			token[i].type = TK_NUM;
			token[i].input = p;
			token[i].value = strtol(p, &p, 10);
			i++;
			continue;	
		}

		fprintf(stderr, "トークナイズできません: %s\n", p);
        exit(1);
	}

	token[i].type = TK_EOF;
	token[i].input = "";
}

void error(int i) {
	fprintf(stderr, "不適切な入力：'%s'\n", token[i].input);
	exit(1);
}

int main(int argc, char **argv) {
	if (argc != 2){
		fprintf(stderr, "引数の個数が正しくありません\n");
		return 1;
	}

	tokenize(argv[1]);

	printf(".intel_syntax noprefix\n");
	printf(".global _main\n");
	printf("_main:\n");

    if (token[0].type != TK_NUM) error(0);
	printf("	mov rax, %d\n", token[0].value);

    int i = 1;
	while(token[i].type != TK_EOF) {
		if(token[i].type == '+') {
			i++;
            if (token[i].type != TK_NUM) error(i);
			printf("	add rax, %d\n", token[i].value);
            i++;
			continue;
		}

		if(token[i].type == '-') {
			i++;
            if (token[i].type != TK_NUM) error(i);
			printf("	sub rax, %d\n", token[i].value);
			i++;
            continue;	
		}

        error(i);
	}

	printf("	ret\n");
	return 0;
}
