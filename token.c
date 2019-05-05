#include"0cc.h"

int is_alnum(char c) {
  return ('a' <= c && c <= 'z') ||  ('A' <= c && c <= 'Z') ||  ('0' <= c && c <= '9') ||  (c == '_'); 
}

void tokenize(Vector* tokens, char* p){
	while(*p) {
	  Token* token = malloc(sizeof(Token));

    if(isspace(*p)) {
			p++;
			continue;
		}

    if (strncmp(p, "if", 2) == 0 && !is_alnum(p[2])) {
      token->type = TK_IF;
      token->input = p;
      vec_push(tokens, token);
      p = p + 2;
      continue;
    }

    if (strncmp(p, "else", 4) == 0 && !is_alnum(p[4])) {
      token->type = TK_ELSE;
      token->input = p;
      vec_push(tokens, token);
      p = p + 4;
      continue;
    }

    if (strncmp(p, "while", 5) == 0 && !is_alnum(p[5])) {
      token->type = TK_WHILE;
      token->input = p;
      vec_push(tokens, token);
      p = p + 5;
      continue;
    }

    if (strncmp(p, "for", 3) == 0 && !is_alnum(p[3])) {
      token->type = TK_FOR;
      token->input = p;
      vec_push(tokens, token);
      p = p + 3;
      continue;
    }

    if (strncmp(p, "return", 6) == 0 && !is_alnum(p[6])) {
      token->type = TK_RETURN;
      token->input = p;
      vec_push(tokens, token);
	    p = p + 6;
      continue;
    }

    if (strncmp(p, "==", 2) == 0) {
      token->type = TK_EQ;
      token->input = p;
      vec_push(tokens, token);
	    p = p + 2;
      continue;
    }

    if (strncmp(p, "!=", 2) == 0) {
      token->type = TK_NE;
      token->input = p;
      vec_push(tokens, token);
	    p = p + 2;
      continue;
    }

    if (strncmp(p, "<=", 2) == 0) {
      token->type = TK_LE;
      token->input = p;
      vec_push(tokens, token);
	    p = p + 2;
      continue;
    }

    if (strncmp(p, ">=", 2) == 0) {
      token->type = TK_GE;
      token->input = p;
      vec_push(tokens, token);
	    p = p + 2;
      continue;
    }

		if( *p=='+' || *p=='-' || *p=='*' || *p=='/'|| *p=='(' || *p==')'
        || *p==';' || *p=='=' || *p=='<'|| *p=='>' ) {
      token->type = *p;
			token->input = p;
      vec_push(tokens, token);
			p++;
			continue;
		}

    if(isdigit(*p)) {
			token->type = TK_NUM;
			token->input = p;
			token->value = strtol(p, &p, 10);
      vec_push(tokens, token);
			continue;	
		}

    if (is_alnum(*p)) {
      p++;
      int cnt = 1;
      while (is_alnum(*p)) {
        cnt++;
        p++;
      }

      token->type = TK_IDENT;
      token->name = malloc(sizeof(char)*cnt);
      strncpy(token->name, p-cnt, cnt);
      token->input = p-cnt;
      vec_push(tokens, token);
      continue;
    }

		fprintf(stderr, "トークナイズできません: %s\n", p);
        exit(1);
	}
	
  Token* token = malloc(sizeof(Token));
  
	token->type = TK_EOF;
  token->input = p;
  vec_push(tokens, token);
}	


