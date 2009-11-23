/*
 * Thread management in libl4thread.
 *
 * Copyright (C) 2009 B Labs Ltd.
 */
#include <stdio.h>
#include <malloc/malloc.h>
#include <l4/api/errno.h>
#include <l4/api/thread.h>
#include <l4lib/tcb.h>
#include <l4/macros.h>

/* Global task list. */
struct global_list global_tasks = {
	.list = { &global_tasks.list, &global_tasks.list },
	.total = 0,
};

/* Function definitions */
void global_add_task(struct tcb *task)
{
	BUG_ON(!list_empty(&task->list));
	list_insert_tail(&task->list, &global_tasks.list);
	global_tasks.total++;
}

void global_remove_task(struct tcb *task)
{
	BUG_ON(list_empty(&task->list));
	list_remove_init(&task->list);
	BUG_ON(--global_tasks.total < 0);
}

struct tcb *find_task(int tid)
{
	struct tcb *t;

	list_foreach_struct(t, &global_tasks.list, list)
		if (t->tid == tid)
			return t;
	return 0;
}

struct tcb *l4_tcb_alloc_init(struct tcb *parent, unsigned int flags)
{
	struct tcb *task;

	if (!(task = kzalloc(sizeof(struct tcb))))
		return PTR_ERR(-ENOMEM);

	link_init(&task->list);

	if (flags & TC_SHARE_SPACE)
		task->utcb_head = parent->utcb_head;
	else {
		/* COPY or NEW space */
		if (!(task->utcb_head = kzalloc(sizeof(struct utcb_head)))) {
			kfree(task);
			return PTR_ERR(-ENOMEM);
		}
		link_init(&task->utcb_head->list);
	}

	return task;
}