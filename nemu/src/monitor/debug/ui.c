#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  return -1;
}

static int cmd_help(char *args);
static int cmd_si(char *args){
    char *arg = strtok(NULL, " ");
	if (arg == NULL) cpu_exec(1);
	else cpu_exec(atoi(arg));
	return 0;
}
static int cmd_info(char *args){
    char *arg = strtok(NULL, " ");
    // 分割字符
	if (arg == NULL){
		puts("Missing parameter.");
		return 0;
	}
    if (strcmp(arg, "r") == 0)
    {
        // 依次打印所有寄存器
        // 这里给个例子：打印出 eax 寄存器的值
		for (int i = 0; i < 8; i++)
			printf("%s:\t0x%08x\t%d\n", regsl[i], cpu.gpr[i]._32, cpu.gpr[i]._32);

    }
	else if (strcmp(arg, "w") == 0)
    {
        // 这里我们会在 PA1.3 中实现
    }
    return 0;
}
static int cmd_x(char *args){
    //分割字符串，得到起始位置和要读取的次数
    char *arg1 = strtok(NULL, " ");
	if (arg1 == NULL){
		puts("Missing parameter.");
		return 0;
	}
    char *s = strtok(NULL, " ");
	if (s == NULL){
		puts("Missing parameter.");
        return 0;
    }
	uint32_t n = 0;
    for (int i = 2; (s[i] >= '0' && s[i] <= '9') || (s[i] >= 'a' && s[i] <= 'z') || (s[i] >='A' && s[i] <= 'Z'); i++){
        if (s[i] > '9')
            n = 16 * n + (10 + s[i] - 'a');
        else
            n = 16 * n + (s[i] - '0');
    }
    //循环使用 vaddr_read 函数来读取内存
	puts("Address         Dword block\tByte sequence");
    for(int i = 0; i < atoi(arg1); i++){
        uint32_t instr = vaddr_read(n, 4);    //如何调用，怎么传递参数，请阅读代码
		uint8_t *p_instr = (void *)&instr;
		printf("0x%08x\t0x%08x\t", n, instr);
		for (int i = 0; i < 4; i++) 
			printf("%02x ", p_instr[i]);
		n += 4;
		puts("");
    }
	return 0;
}
static int cmd_p(char *args){
	puts(args);
// 	if (arg == NULL){
// 		puts("Missing parameter.");
// 		return 0;
// 	}
//     printf("%d\n", check_parentheses("(2 - 1)"));
//     printf("%d\n", check_parentheses("(4 + 3 * (2 - 1))"));
//     printf("%d\n", check_parentheses("4 + 3 * (2 - 1)"));
//     printf("%d\n", check_parentheses("(4 + 3)) * ((2 - 1)"));
//     printf("%d\n", check_parentheses("(4 + 3) * (2 - 1)"));
	bool *success = 0;
	expr(args, success);
	return 0;
}	

static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "Single step execution", cmd_si },
  { "info", "Print program status", cmd_info },
  { "x", "Scan memory", cmd_x },
  { "p", "Expression evaluation", cmd_p },

  /* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  while (1) {
    char *str = rl_gets();
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}
