#ifndef __LIB_WAIT_H__
#define __LIB_WAIT_H__

#include <l4/lib/list.h>
#include <l4/lib/spinlock.h>

struct ktcb;
struct waitqueue {
	struct list_head task_list;
	struct ktcb *task;
};

#define WAKEUP_ASYNC			0

enum wakeup_flags {
	WAKEUP_INTERRUPT = (1 << 0),
	WAKEUP_SYNC	 = (1 << 1)
};

#define CREATE_WAITQUEUE_ON_STACK(wq, tsk)		\
struct waitqueue wq = {					\
	.task_list = { &wq.task_list, &wq.task_list },	\
	.task = tsk,					\
};

struct waitqueue_head {
	int sleepers;
	struct spinlock slock;
	struct list_head task_list;
};

static inline void waitqueue_head_init(struct waitqueue_head *head)
{
	memset(head, 0, sizeof(struct waitqueue_head));
	INIT_LIST_HEAD(&head->task_list);
}

void task_set_wqh(struct ktcb *task, struct waitqueue_head *wqh,
		  struct waitqueue *wq);

void task_unset_wqh(struct ktcb *task);


/*
 * Sleep if the given condition isn't true.
 * ret will tell whether condition was met
 * or we got interrupted.
 */
#define WAIT_EVENT(wqh, condition, ret)				\
do {								\
	ret = 0;						\
	for (;;) {						\
		spin_lock(&(wqh)->slock);			\
		if (condition) {				\
			spin_unlock(&(wqh)->slock);		\
			break;					\
		}						\
		CREATE_WAITQUEUE_ON_STACK(wq, current);		\
		task_set_wqh(current, wqh, &wq);		\
		(wqh)->sleepers++;				\
		list_add_tail(&wq.task_list, &(wqh)->task_list);\
		printk("(%d) waiting...\n", current->tid);	\
		sched_prepare_sleep();				\
		spin_unlock(&(wqh)->slock);			\
		schedule();					\
		/* Did we wake up normally or get interrupted */\
		if (current->flags & TASK_INTERRUPTED) {	\
			current->flags &= ~TASK_INTERRUPTED;	\
			ret = -EINTR;				\
			break;					\
		}						\
	}							\
} while(0);

void wake_up(struct waitqueue_head *wqh, unsigned int flags);
int wake_up_task(struct ktcb *task, unsigned int flags);
void wake_up_all(struct waitqueue_head *wqh, unsigned int flags);

int wait_on(struct waitqueue_head *wqh);

#endif /* __LIB_WAIT_H__ */

