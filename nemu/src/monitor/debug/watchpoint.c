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
	free_ = free->next;
	if (head->NO < tmp->NO){
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

