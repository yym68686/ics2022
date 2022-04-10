#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP* new_wp(){
	if (free_ == NULL) assert(0);
	if (head == NULL)
		head = free_;
	WP* tmp = free_;
	free_ = free_->next;
	tmp->next = NULL;
// 	printf("new_wp free_->NO:%d\n", free_->NO);
	if (head->NO > tmp->NO){
		tmp->next = head;
		head = tmp;
		return tmp;
	}

	WP *temp = head, *last = temp;
// 	while (temp) last = temp, temp = temp->next;
// 	last->next = tmp;

// 	temp = head;
	while (temp){
		last = temp;
		temp = temp->next;
		if (temp && temp->NO > tmp->NO){
			last->next = tmp;
			tmp->next = temp;
			break;
		}
		else if(!temp){
			last->next = tmp;
			tmp->next = NULL;
			break;
		}
	} 
	return tmp;
}
void free_wp(WP *wp){
	if (free_ == NULL) {
		free_ = wp;
		wp->next = NULL;
	}
	if (wp->NO < free_->NO){
		if (head == wp) head = head->next;
		else {
			WP *t = head, *pre = t;
			while (t){
				if (t->NO == wp->NO){
					pre->next = wp->next;
					break;
				}
				pre = t;
				t = t->next;
			}
		}
		wp->next = free_;
		free_ = wp;
		if (head && !head->next && head == free_) head = NULL;
		return;
	}

	//这里是为了把想要释放的节点的上一个节点的next指针指向待释放节点的下一个节点
	WP *temp = head, *last = temp;
	while (temp) {
		if (temp->NO == wp->NO){
			last->next = wp->next;
			break;
		}
		last = temp;
		temp = temp->next;
	}
	

	//以下代码功能如图所示：
	//    head                      +-------------------------->...               head
	//     +                        |                                              +
	//     |            +-------------------------+                                |
	//     |            |           |             |                                |
	//     |            |           |             |                                |
	//+----v---+   +----+---+   +---+----+   +----v---+                       +----v---+   +--------+   +--------+   +--------+
	//|        |   |        |   |        |   |        +-------->...    --->   |        |   |        +--->        +--->        +-------->...
	//+----+---+   +----+---+   +---^----+   +--------+                       +----+---+   +----+---+   +--------+   +--------+
	//     |            ^           |                                              |            ^
	//     |            |           |                                              |            |
	//     |            +           |                                              |            +
	//     |           free_        |                                              |           free_
	//     |                        |                                              |
	//     |                        |                                              |
	//     +------------------------+                                              +------------------------->...

	temp = free_, last = temp;
	while (temp->next){
		last = temp;
		temp = temp->next;
		if (temp->NO > wp->NO){
			if (head == wp) head = head->next;
			last->next = wp;
			wp->next = temp;
			break;
		}
		else if (!temp->next){
			temp->next = wp;
			wp->next = NULL;
			break;
		}
	}
	return;
}
//给予一个表达式e，构造以该表达式为监视目标的监视点，并返回编号
int set_watchpoint(char *e){
	WP* new = new_wp();
// 	printf("set_watchpoint new->NO:%d\n", new->NO);
// 	printf("set_watchpoint free_->NO:%d\n", free_->NO);
	strcpy(new->expr, e);
    printf("Set watchpoint #%d\n", new->NO);
    printf("expr      = %s\n", e);
	bool success = true;
    uint32_t result = expr(e, &success);
    if (success == true && result != -1162167624){
        printf("old value = 0x%x\n", result);
        new->old_val = result;
    }
	return new->NO;
}
//给予一个监视点编号，从已使用的监视点中归还该监视点到池中
bool delete_watchpoint(int NO){
	if (!head) return false;
	WP* tmp = head;
	int flag = 0;
	while (tmp) {
		if (tmp->NO == NO) flag = 1;;
		tmp = tmp->next;
	}
	if (!flag) return false;
	tmp = head;
	while (tmp){
// 		printf("delete_watchpoint:%d free_->NO:%d head->NO:%d\n", tmp->NO, free_->NO, head->NO);
		if (tmp->NO == NO){
			free_wp(tmp);
			printf("Watchpoint %d deleted", tmp->NO);
// 			WP* temp = head;
// 			while(temp) printf("%d ", temp->NO), temp = temp->next;
// 			puts("");
			return true;
		}
		tmp = tmp->next;
	}
	puts("");
	return false;
} 

//显示当前在使用状态中的监视点列表
void list_watchpoint(void){
	if (!head) return;
	puts("NO Expr      Old Value");
	WP* tmp = head;
	while (tmp){
		bool success = true;
		uint32_t result = expr(tmp->expr, &success);
		if (success == true && result != -1162167624) printf("%2d %-10s0x%x\n", tmp->NO, tmp->expr, tmp->old_val);
		else break;
		tmp = tmp->next;
	}
	return;
}
//扫描所有使用中的监视点，返回触发的监视点指针，若无触发返回NULL
WP* scan_watchpoint(void){
	WP *cur = head;
	while (cur->next){
		bool success = true;
		uint32_t result = expr(cur->expr, &success);
		if (success == true && result != -1162167624) cur->new_val = result;
		else return NULL;
		if (cur->old_val != cur->new_val){
			printf("Hit watchpoint %d at address 0x%d", cur->NO, cur->NO);
			printf("expr      = %s\n", cur->expr);
			printf("old value = %d\nnew value = %d\n", cur->old_val, cur->new_val);
			puts("program paused");
			cur->old_val = cur->new_val;
			return cur;
		}
		cur = cur->next;
	}
	return NULL;
}

