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
	tmp->next = NULL;
	printf("new_wp free_ == NULL:%d\n", free_ == NULL);
	free_ = free_->next;
	if (head->NO > tmp->NO){
		tmp->next = head;
		head = tmp;
		return tmp;
	}

	WP *temp = head, *last = temp;
	while (temp->next){
		last = temp;
		temp = temp->next;
		if (temp->NO < tmp->NO){
			last->next = tmp;
			tmp->next = temp;
			break;
		}
		else if(!temp->next){
			temp->next = tmp;
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
		wp->next = free_;
		free_ = wp;
		if (head == free_) head = NULL;
		return;
	}
	WP *temp = free_, *last = temp;
	while (temp->next){
		last = temp;
		temp = temp->next;
		if (temp->NO < wp->NO){
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
	printf("set_watchpoint free_ == NULL:%d\n", free_ == NULL);
	printf("set_watchpoint free_->NO:%d\n", free_->NO);
	strcpy(new->expr, e);
    printf("Set watchpoint #%d\n", new->NO);
    printf("expr = %s\n", e);
	bool success = true;
    uint32_t result = expr(e, &success);
    if (success == true && result != -1162167624){
        printf("old value = %d\n", result);
        new->old_val = result;
    }
	return new->NO;
}
//给予一个监视点编号，从已使用的监视点中归还该监视点到池中
bool delete_watchpoint(int NO){
	if (!head) return false;
	WP* tmp = head;
	while (tmp){
		printf("delete_watchpoint:%d free_->NO:%d\n", tmp->NO, free_->NO);
		if (tmp->NO == NO){
			free_wp(tmp);
			return true;
		}
		tmp = tmp->next;
	}
	return false;
} 
//显示当前在使用状态中的监视点列表
void list_watchpoint(void){
	if (!head) return;
	puts("NO expr");
	WP* tmp = head;
	while (tmp){
		printf("%2d %s\n", tmp->NO, tmp->expr);
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
			cur->old_val = cur->new_val;
			return cur;
		}
		cur = cur->next;
	}
	return NULL;
}

