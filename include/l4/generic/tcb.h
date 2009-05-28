/*
 * Thread Control Block, kernel portion.
 *
 * Copyright (C) 2007-2009 Bahadir Bilgehan Balban
 */
#ifndef __TCB_H__
#define __TCB_H__

#include <l4/lib/list.h>
#include <l4/lib/mutex.h>
#include <l4/generic/scheduler.h>
#include <l4/generic/pgalloc.h>
#include <l4/generic/space.h>
#include INC_GLUE(memory.h)
#include INC_GLUE(syscall.h)
#include INC_GLUE(message.h)
#include INC_GLUE(context.h)
#include INC_SUBARCH(mm.h)


/*
 * These are a mixture of flags that indicate the task is
 * in a transitional state that could include one or more
 * scheduling states.
 */
#define TASK_INTERRUPTED		(1 << 0)
#define TASK_SUSPENDING			(1 << 1)
#define TASK_RESUMING			(1 << 2)

/* Task states */
enum task_state {
	TASK_INACTIVE	= 0,
	TASK_SLEEPING	= 1,
	TASK_RUNNABLE	= 2,
};

#define TASK_ID_INVALID			-1
struct task_ids {
	l4id_t tid;
	l4id_t spid;
	l4id_t tgid;
};

struct ktcb {
	/* User context */
	task_context_t context;

	/*
	 * Reference to the context on stack
	 * saved at the beginning of a syscall trap.
	 */
	syscall_context_t *syscall_regs;

	/* Runqueue related */
	struct list_head rq_list;
	struct runqueue *rq;

	/* Thread information */
	l4id_t tid;		/* Global thread id */
	l4id_t tgid;		/* Global thread group id */
	/* See space for space id */

	/* Flags to indicate various task status */
	unsigned int flags;

	/* IPC flags */
	unsigned int ipc_flags;

	/* Lock for blocking thread state modifications via a syscall */
	struct mutex thread_control_lock;

	/* Other related threads */
	l4id_t pagerid;

	u32 ts_need_resched;	/* Scheduling flag */
	enum task_state state;
	struct list_head task_list; /* Global task list. */

	/* UTCB related, see utcb.txt in docs */
	unsigned long utcb_address;	/* Virtual ref to task's utcb area */

	/* Thread times */
	u32 kernel_time;	/* Ticks spent in kernel */
	u32 user_time;		/* Ticks spent in userland */
	u32 ticks_left;		/* Timeslice ticks left for reschedule */
	u32 ticks_assigned;	/* Ticks assigned to this task on this HZ */
	u32 sched_granule;	/* Granularity ticks left for reschedule */
	int priority;		/* Task's fixed, default priority */

	/* Number of locks the task currently has acquired */
	int nlocks;

	/* Page table information */
	struct address_space *space;

	/* Fields for ipc rendezvous */
	struct waitqueue_head wqh_recv;
	struct waitqueue_head wqh_send;
	l4id_t expected_sender;

	/* Waitqueue for pagers to wait for task states */
	struct waitqueue_head wqh_pager;

	/* Tells where we are when we sleep */
	struct spinlock waitlock;
	struct waitqueue_head *waiting_on;
	struct waitqueue *wq;

	/*
	 * Extended ipc size and buffer that
	 * points to the space after ktcb
	 */
	unsigned long extended_ipc_size;
	char extended_ipc_buffer[];
};

/* Per thread kernel stack unified on a single page. */
union ktcb_union {
	struct ktcb ktcb;
	char kstack[PAGE_SIZE];
};


/*
 * Each task is allocated a unique global id. A thread group can only belong to
 * a single leader, and every thread can only belong to a single thread group.
 * These rules allow the fact that every global id can be used to define a
 * unique thread group id. Thread local ids are used as an index into the thread
 * group's utcb area to discover the per-thread utcb structure.
 */
static inline void set_task_ids(struct ktcb *task, struct task_ids *ids)
{
	task->tid = ids->tid;
	task->tgid = ids->tgid;
}

static inline void tcb_set_ipc_flags(struct ktcb *task,
				     unsigned int flags)
{
	task->ipc_flags = flags;
}

static inline unsigned int tcb_get_ipc_flags(struct ktcb *task)
{
	return task->ipc_flags;
}

static inline unsigned int
ipc_flags_set_type(unsigned int flags, unsigned int type)
{
	flags &= ~IPC_FLAGS_TYPE_MASK;
	flags |= type & IPC_FLAGS_TYPE_MASK;
	return flags;
}

static inline unsigned int ipc_flags_get_type(unsigned int flags)
{
	return flags & IPC_FLAGS_TYPE_MASK;
}

static inline void tcb_set_ipc_type(struct ktcb *task,
				    unsigned int type)
{
	task->ipc_flags = ipc_flags_set_type(task->ipc_flags,
					     type);
}

static inline unsigned int tcb_get_ipc_type(struct ktcb *task)
{
	return ipc_flags_get_type(task->ipc_flags);
}

#define THREAD_IDS_MAX		1024
#define SPACE_IDS_MAX		1024


extern struct id_pool *thread_id_pool;
extern struct id_pool *space_id_pool;

struct ktcb *tcb_find(l4id_t tid);
void tcb_add(struct ktcb *tcb);
void tcb_remove(struct ktcb *tcb);

void tcb_init(struct ktcb *tcb);
struct ktcb *tcb_alloc_init(void);
void tcb_delete(struct ktcb *tcb);

void init_ktcb_list(void);
void task_update_utcb(struct ktcb *cur, struct ktcb *next);
int tcb_check_and_lazy_map_utcb(struct ktcb *task);

#endif /* __TCB_H__ */

