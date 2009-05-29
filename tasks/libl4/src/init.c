/*
 * Initialise system call offsets and utcb reference.
 *
 * Copyright (C) 2007-2009 Bahadir Bilgehan Balban
 */
#include <l4lib/kip.h>
#include <l4lib/arch/syslib.h>
#include <l4lib/arch/utcb.h>
#include <l4lib/ipcdefs.h>
#include <l4/macros.h>
#include INC_GLUE(memlayout.h)
#include <stdio.h>

__l4_ipc_t __l4_ipc = 0;
__l4_map_t __l4_map = 0;
__l4_unmap_t __l4_unmap = 0;
__l4_kread_t __l4_kread = 0;
__l4_getid_t __l4_getid = 0;
__l4_thread_switch_t __l4_thread_switch = 0;
__l4_thread_control_t __l4_thread_control = 0;
__l4_ipc_control_t __l4_ipc_control = 0;
__l4_space_control_t __l4_space_control = 0;
__l4_exchange_registers_t __l4_exchange_registers = 0;
__l4_kmem_control_t __l4_kmem_control = 0;
__l4_time_t __l4_time = 0;
__l4_mutex_control_t __l4_mutex_control = 0;

struct kip *kip;

/*
 * Reference to private UTCB of this thread.
 * Used only for pushing/reading ipc message registers.
 */
struct utcb **kip_utcb_ref;


void __l4_init(void)
{
	/* Kernel interface page */
	kip = l4_kernel_interface(0, 0, 0);

	/* Reference to utcb field of KIP */
	kip_utcb_ref = (struct utcb **)&kip->utcb;

	__l4_ipc =		(__l4_ipc_t)kip->ipc;
	__l4_map =		(__l4_map_t)kip->map;
	__l4_unmap =		(__l4_unmap_t)kip->unmap;
	__l4_kread =		(__l4_kread_t)kip->kread;
	__l4_getid =		(__l4_getid_t)kip->getid;
	__l4_thread_switch =	(__l4_thread_switch_t)kip->thread_switch;
	__l4_thread_control=	(__l4_thread_control_t)kip->thread_control;
	__l4_ipc_control=	(__l4_ipc_control_t)kip->ipc_control;
	__l4_space_control=	(__l4_space_control_t)kip->space_control;
	__l4_exchange_registers =
			(__l4_exchange_registers_t)kip->exchange_registers;
	__l4_kmem_control =	(__l4_kmem_control_t)kip->kmem_control;
	__l4_time =		(__l4_time_t)kip->time;
	__l4_mutex_control=	(__l4_mutex_control_t)kip->mutex_control;
}

