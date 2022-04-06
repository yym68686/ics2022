#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include <stdlib.h>
#include <string.h>
enum {
  TK_NOTYPE = 256,
  TK_EQ,
  TK_NEQ,
  TK_AND,
  TK_OR,
  TK_NOT,
  TK_HEX,
  TK_DEC,
  TK_REG,
  TK_NEG,
  DEREF,
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
  {"!=", TK_NEQ},       // not equal
  {"&&", TK_AND},       // and
  {"\\|\\|", TK_OR},    // or
  {"!", TK_NOT},        // not
  {"0x[0-9a-zA-Z]+", TK_HEX},
  {"[1-9][0-9]*|0", TK_DEC},
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
  memset(tokens, 0, sizeof(Token) * 32);

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

		int flag = 0;
        switch (rules[i].token_type) {
         case TK_EQ:
			 tokens[nr_token].type = TK_EQ;
			 break;
         case TK_NEQ:
			 tokens[nr_token].type = TK_NEQ;
			 break;
         case TK_AND:
			 tokens[nr_token].type = TK_AND;
			 break;
         case TK_OR:
			 tokens[nr_token].type = TK_OR;
			 break;
         case TK_NOT:
			 tokens[nr_token].type = TK_NOT;
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
		if (!flag)
			strncpy(tokens[nr_token++].str, e + position, substr_len);
        position += substr_len;
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
		if (tokens[p + pos].str[0] == '(') sta++;
		else if (tokens[p + pos].str[0] == ')') sta--;
		else if (p + pos == q || pos == 0)
			return false;
		if (((p + pos++) != q && sta <= 0) || sta < 0)
			return false;
	}
	return true;
}
int find_dominated_op(int p, int q);
int check_error(int p, int q){
    int sta = 0, pos = 0;
	while (p + pos != q + 1){
		if (tokens[p + pos].str[0] == '(') sta++;
		else if (tokens[p + pos].str[0] == ')'){
			sta--;
			if (sta < 0 || (p + pos == q && sta != 0)){
				printf("Invalid expression: ");
				int len = 0;
				for (int i = 0; i < nr_token; i++){
					printf("%s", tokens[i].str);
					if (i < pos)
						len += strlen(tokens[i].str);
				}
				printf("\n%*c\n", len + 21, '^');
				return 0;
			}
		}
		else if (p + pos == q && sta != 0){
			printf("Invalid expression: ");
			int len = 0;
			for (int i = 0; i < nr_token; i++){
				printf("%s", tokens[i].str);
				len += strlen(tokens[i].str);
			}
			printf("\n%*c\n", len + 21, '^');
			return 0;
		}
		pos++;
	}
	return 1;
}
uint32_t eval(int p, int q) {
	printf("%d %d %s %s %d:\n",  p, q, tokens[p].str,  tokens[q].str, check_parentheses(p, q));

    if (p > q) {
        /* Bad expression */
		printf("%d %d %s %s %d:\n",  p, q, tokens[p].str,  tokens[q].str, tokens[q].type == MINUS);
		if (q == -1) exit(0);
		printf("Invalid expression: ");
		int len = 0;
		for (int i = 0; i < nr_token; i++){
			printf("%s", tokens[i].str);
			if (i < q)
				len += strlen(tokens[i].str);
		}
		printf("\n%*c\n", len + 21, '^');
		return 0x3f3f3f3f;
    }
    else if (p == q) {
		if (tokens[p].type == TK_DEC)
			return (uint32_t)atol(tokens[p].str);
		if (tokens[p].type == TK_REG){
			char *reg = strtok(tokens[p].str, "$");
			if (!strcmp(reg, "eip")) return cpu.eip;
			for (int i = 0; i < 8; i++)
				if (!strcmp(reg, regsl[i]))
					return cpu.gpr[i]._32;
		}
		if (tokens[p].type == TK_HEX){
			uint32_t n = 0;
			for (int i = 2; (tokens[p].str[i] >= '0' && tokens[p].str[i] <= '9') || (tokens[p].str[i] >= 'a' && tokens[p].str[i] <= 'z') || (tokens[p].str[i] >='A' && tokens[p].str[i] <= 'Z'); i++){
				if (tokens[p].str[i] > '9')
					n = 16 * n + (10 + tokens[p].str[i] - 'a');
				else
					n = 16 * n + (tokens[p].str[i] - '0');
			}
			return n;
		}
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
		int op = find_dominated_op(p, q);

		printf("op:%d %s\n", op, tokens[op].str);
		uint32_t val1 = 0, val2 = 0;
		if (tokens[op].type != TK_NOT && tokens[op].type != DEREF && tokens[op].type != TK_NEG)
			val1 = eval(p, op - 1);
		val2 = eval(op + 1, q);
		printf("val2:%d\n", val2);
		return 0;
        switch (tokens[op].type) {
			case TK_EQ: return val1 == val2;
			case TK_NEQ: return val1 != val2;
			case TK_AND: return val1 && val2;
			case TK_OR: return val1 || val2;
			case TK_NOT: return !val2;
			case TK_NEG: return 0 - val2;
			case DEREF: return vaddr_read(val2, 4);
            case PLUS: return val1 + val2;
            case MINUS: return val1 - val2;
            case TIMES: return val1 * val2;
            case DIVIDE: return val1 / val2;
            default: assert(0);
		}
    }
	return 0;
}
int find_dominated_op(int p, int q){
	int pos = q, stack = 0;
	while (pos != p){
		if(tokens[pos].str[0] == ')')
			stack++;
		else if (tokens[pos].str[0] == '(')
			stack--;
		else if ((tokens[pos].str[0] == '+' \
			   || tokens[pos].type == MINUS \
			   || !strcmp(tokens[pos].str, "==") \
			   || !strcmp(tokens[pos].str, "!=") \
			   || !strcmp(tokens[pos].str, "||") \
			   || !strcmp(tokens[pos].str, "!") \
			   || !strcmp(tokens[pos].str, "&&"))\
			   && !stack)
			return pos;
		pos--;
	}
	pos = q, stack = 0;
	while (pos != p){
		if(tokens[pos].str[0] == ')')
			stack++;
		else if (tokens[pos].str[0] == '(')
			stack--;
		else if ((tokens[pos].type == TIMES || tokens[pos].str[0] == '/') && !stack)
			return pos;
		pos--;
	}
	return 0;
}
uint32_t expr(char *e, bool *success) {
	if (!make_token(e)) {
		*success = false;
		return 0;
	}
	for (int i = 0; i < nr_token; i++) {
		if (tokens[i].str[0] == '*' && (i == 0 \
				|| tokens[i - 1].type == DIVIDE \
				|| tokens[i - 1].type == LeftBracket \
				|| tokens[i - 1].type == PLUS \
				|| tokens[i - 1].type == MINUS \
				|| tokens[i - 1].type == TIMES) ) {
			tokens[i].type = DEREF;
		}
	}
	for (int i = 0; i < nr_token; i++) {
		if (tokens[i].str[0] == '-' && (i == 0 \
                 || tokens[i - 1].type == DIVIDE \
                 || tokens[i - 1].type == LeftBracket \
                 || tokens[i - 1].type == PLUS \
                 || tokens[i - 1].type == MINUS \
                 || tokens[i - 1].type == TIMES) ) {
			tokens[i].type = TK_NEG;
		}
	}
	if (!check_error(0, nr_token - 1)) {
		*success = false;
		return 0;
	}
  /* TODO: Insert codes to evaluate the expression. */
//   TODO();

  return eval(0, nr_token - 1);
}
