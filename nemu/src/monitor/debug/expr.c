#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256,
  TK_EQ,
  TK_HEX,
  TK_DEC,
  TK_REG,
  LeftBracket,
  RightBracket,
  PLUS,
  MINUS,
  TIMES,
  DIVIDE

  /* TODO: Add more token types */

};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */
  {" +", TK_NOTYPE},    // spaces
  {"==", TK_EQ},        // equal
  {"0x\\d+", TK_HEX},
  {"[1-9][0-9]*", TK_DEC},
  {"\\$e..", TK_REG},	
  {"\\(", LeftBracket},	
  {"\\)", RightBracket},	
  {"\\+", PLUS},         // plus
  {"\\-", MINUS},         
  {"\\*", TIMES},         
  {"\\/", DIVIDE}         
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

		int flag = 0;
        switch (rules[i].token_type) {
         case TK_EQ:
			 tokens[nr_token].type = TK_EQ;
			 break;
         case TK_HEX:
			 tokens[nr_token].type = TK_HEX;
			 break;
         case TK_DEC:
			 tokens[nr_token].type = TK_DEC;
			 break;
         case TK_REG:
			 tokens[nr_token].type = TK_REG;
			 break;
         case LeftBracket:
			 tokens[nr_token].type = LeftBracket;
			 break;
         case RightBracket:
			 tokens[nr_token].type = RightBracket;
			 break;
         case PLUS:
			 tokens[nr_token].type = PLUS;
			 break;
         case MINUS:
			 tokens[nr_token].type = MINUS;
			 break;
         case TIMES:
			 tokens[nr_token].type = TIMES;
			 break;
		 case DIVIDE:
			 tokens[nr_token].type = DIVIDE;
			 break;
         default: flag = 1;
        }
		if (flag == 0){
			strncpy(tokens[nr_token++].str, e + position, substr_len);
			for (int j = 0; j < nr_token; j++){
				printf("|%s| ", tokens[j].str);
			}
			puts("");

		}
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}
bool check_parentheses(int p, int q){
    int sta = 0, pos = 0;
	while (p + pos != q + 1){
		printf("strcmp:%s %d\n", tokens[p + pos].str, strcmp(tokens[p + pos].str, "("));
		if (strcmp(tokens[p + pos].str, "(") == 0)
			sta++, puts("yes");
		else if (strcmp(tokens[p + pos].str, ")") == 0)
			sta--;
		else if (p + pos == q || pos == 0)
			return false;
		if (((p + pos) != q && sta <= 0) || sta < 0)
			return false;
		pos++;
	}
	return true;
}

uint32_t eval(int p, int q) {
    if (p > q) {
        /* Bad expression */
    }
    else if (p == q) {
        /* Single token.
        * For now this token should be a number.
        * Return the value of the number.
        */
    }
    else if (check_parentheses(p, q) == true) {
        /* The expression is surrounded by a matched pair of parentheses.
        * If that is the case, just throw away the parentheses.
        */
        return eval(p + 1, q - 1);
    }
    else {
        /* We should do more things here. */
    }
	return 0;
}
// find_dominated_op(int p, int q){

// }
uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  printf("%d\n", check_parentheses(0, nr_token - 1));
//    printf("%d\n", check_parentheses("(4 + 3 * (2 - 1))"));
//    printf("%d\n", check_parentheses("4 + 3 * (2 - 1)"));
//    printf("%d\n", check_parentheses("(4 + 3)) * ((2 - 1)"));
//    printf("%d\n", check_parentheses("(4 + 3) * (2 - 1)"));
  /* TODO: Insert codes to evaluate the expression. */
//   TODO();

  return 0;
}
