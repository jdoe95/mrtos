/** ************************************************************************
 * @file thread.h
 * @brief Thread management
 * @author John Yu buyi.yu@wne.edu
 *
 * This file is part of mRTOS.
 *
 * This implementation incorporates the fixed priority preemptive scheduling
 * algorithm with round-robin scheduling. A higher priority always preempts
 * a lower priority thread, while threads of the same priority share the
 * CPU time.
 *
 * Copyright (C) 2018 John Buyi Yu
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *****************************************************************************/
#include "../include/thread.h"
#include "../include/list.h"
#include "../include/global.h"

/*
 * Convert scheduler queue item to base class lstitem_t
 */
#define TO_LSTITEM(P_SCHQ_ITEM) \
	((lstitem_t*)(P_SCHQ_ITEM))

/*
 * Initialize a queue item
 */
UTIL_UNSAFE
void sch_qitem_init( sch_qitem_t *p_item, thd_cblk_t *p_thd, uint_t tag )
{
	/*
	 * If failed:
	 * NULL pointer passed to p_item or p_thd
	 */
	UTIL_ASSERT( p_item != NULL );
	UTIL_ASSERT( p_thd != NULL );

	lstitem_init(TO_LSTITEM(p_item));
	p_item->p_thd = p_thd;
	p_item->p_q = NULL;
	p_item->tag = tag;
}

/*
 * Initialize a priority queue header
 */
UTIL_UNSAFE
void sch_q_init( void *p_q )
{
	sch_q_t *p_generic_q;

	/*
	 * If failed:
	 * NULL pointer passed to p_q
	 */
	UTIL_ASSERT( p_q != NULL );
	p_generic_q = (sch_q_t*)p_q;

	p_generic_q->p_head = NULL;
}

/*
 * Enqueue an item FIFO
 */
UTIL_UNSAFE
void sch_qitem_enq_fifo( sch_qitem_t *p_item, sch_qfifo_t *p_q )
{
	/*
	 * If failed:
	 * NULL pointer passed to p_item or p_q
	 */
	UTIL_ASSERT( p_item != NULL );
	UTIL_ASSERT( p_q != NULL );

	/*
	 * If failed:
	 * Item corrupted, uninitialized, or already in queue
	 */
	UTIL_ASSERT( p_item->p_q == NULL );

	/* set queue pointer */
	p_item->p_q = p_q;

	/* inserting first item */
	if( p_q->p_head == NULL )
	{
		p_q->p_head = p_item;
	}

	/* insert as last item */
	else
	{
		lstitem_prepend( TO_LSTITEM(p_item), TO_LSTITEM(p_q->p_head) );
	}
}

/*
 * Enqueue an item priority
 */
UTIL_UNSAFE
void sch_qitem_enq_prio( sch_qitem_t *p_item, sch_qprio_t *p_q )
{
	sch_qitem_t *p_i;

	/*
	 * If failed:
	 * NULL pointer passed to p_item or p_q
	 */
	UTIL_ASSERT( p_item != NULL );
	UTIL_ASSERT( p_q != NULL );

	/*
	 * If failed:
	 * Item corrupted, uninitialized, or already in queue
	 */
	UTIL_ASSERT( p_item->p_q == NULL );

	/* set queue pointer */
	p_item->p_q = p_q;

	/* inserting first item */
	if( p_q ->p_head == NULL )
	{
		p_q->p_head = p_item;
	}

	/* inserting last item */
	else if( p_item->tag >= p_q->p_head->p_prev->tag )
	{
		lstitem_prepend( TO_LSTITEM(p_item), TO_LSTITEM(p_q->p_head ) );
	}

	/* insert as first item */
	else if( p_item->tag < p_q->p_head->tag )
	{
		lstitem_prepend( TO_LSTITEM(p_item), TO_LSTITEM(p_q->p_head ) );
		p_q->p_head = p_item;
	}
	else
	{
		/* search starts from second item */
		p_i = p_q->p_head->p_next;

		do
		{
			/*
			 * If failed:
			 * Broken link
			 */
			UTIL_ASSERT( p_i != NULL );
			UTIL_ASSERT( p_i->p_next != NULL );
			UTIL_ASSERT( p_i->p_next->p_prev == p_i );

			if( p_item->tag < p_i->tag )
			{
				lstitem_prepend( TO_LSTITEM(p_item), TO_LSTITEM(p_i) );
				break;
			}

			p_i = p_i->p_next;

		} while(true);
	}
}

/*
 * Dequeue an item
 */
UTIL_UNSAFE
sch_qitem_t* sch_qitem_deq( void *p_q )
{
	sch_qitem_t *p_ret = NULL;
	sch_q_t *p_generic_q;

	/*
	 * If failed:
	 * NULL pointer passed to p_q
	 */
	UTIL_ASSERT( p_q != NULL );

	p_generic_q = (sch_q_t*)(p_q);

	if( p_generic_q->p_head != NULL )
	{
		p_ret = p_generic_q->p_head;
		sch_qitem_remove( p_generic_q->p_head );
	}

	return p_ret;
}

/*
 * Remove an item from its queue
 */
UTIL_UNSAFE
void sch_qitem_remove( sch_qitem_t *p_item )
{
	sch_q_t *p_generic_q;

	/*
	 * If failed:
	 * NULL pointer passed to p_item
	 */
	UTIL_ASSERT( p_item != NULL );

	/*
	 * If failed:
	 * Item corrupted, or already removed from queue
	 */
	UTIL_ASSERT( p_item->p_q != NULL );

	p_generic_q = (sch_q_t*)(p_item->p_q);

	/* set queue pointer */
	p_item->p_q = NULL;

	/* removing only item */
	if( p_item->p_next == p_item )
	{
		/*
		 * If failed:
		 * Item claims that it is the only item in queue,
		 * but queue header says otherwise.
		 * Corrupted queue.
		 */
		UTIL_ASSERT( p_generic_q->p_head == p_item );

		p_generic_q->p_head = NULL;
	}

	/* removing first item */
	else if( p_item == p_generic_q->p_head )
	{
		/*
		 * If failed:
		 * Broken link
		 */
		UTIL_ASSERT( p_generic_q->p_head->p_next != NULL );

		p_generic_q->p_head = p_generic_q->p_head->p_next;
		lstitem_remove( TO_LSTITEM(p_item) );
	}
	else
	{
		lstitem_remove( TO_LSTITEM(p_item) );
	}
}

/*
 * Initialize scheduler control block
 */
UTIL_UNSAFE
void sch_init( sch_cblk_t *p_sch )
{
	uint_t counter;

	/*
	 * If failed:
	 * NULL pointer passed to p_sch
	 */
	UTIL_ASSERT( p_sch != NULL );

	/* initialize ready queues */
	for( counter = 0; counter < OSPORT_NUM_PRIOS; counter++ )
	{
		sch_q_init( &p_sch->q_ready[counter] );
	}

	sch_q_init( &p_sch->q_delay1);
	sch_q_init( &p_sch->q_delay2 );

	p_sch->lock_depth = 0;
	p_sch->timestamp = 0;
	p_sch->p_current = NULL;
	p_sch->p_next = NULL;
	p_sch->p_delayq_normal = &p_sch->q_delay1;
	p_sch->p_delayq_overflow = &p_sch->q_delay2;
}

/*
 * Unconditionally sets the next thread
 */
UTIL_UNSAFE
void sch_set_next_thread( sch_cblk_t *p_sch )
{
	uint_t counter;

	/*
	 * If failed:
	 * NULL pointer passed to p_sch
	 */
	UTIL_ASSERT( p_sch != NULL );

	for( counter = 0; counter < OSPORT_NUM_PRIOS; counter++ )
	{
		if( p_sch->q_ready[counter].p_head != NULL )
			break;
	}

	/*
	 * If failed:
	 * Idle thread missing
	 */
	UTIL_ASSERT( counter < OSPORT_NUM_PRIOS );

	/*
	 * If failed:
	 * Cannot obtain thread from item
	 */
	UTIL_ASSERT( p_sch->q_ready[counter].p_head->p_thd != NULL );

	p_sch->p_next = p_sch->q_ready[counter].p_head->p_thd;

	/*
	 * If failed:
	 * Broken link
	 */
	UTIL_ASSERT( p_sch->q_ready[counter].p_head->p_next != NULL);

	p_sch->q_ready[counter].p_head =
			p_sch->q_ready[counter].p_head->p_next;
}

/*
 * Lock local interrupts nested
 */
UTIL_SAFE
void sch_lock_int( sch_cblk_t *p_sch )
{
	uint_t int_depth;

	/*
	 * If failed:
	 * Interrupt nested over 100 times, completely impossible.
	 * Underflow?
	 */
	UTIL_ASSERT( p_sch->lock_depth < 100 );

	int_depth = p_sch->lock_depth + 1;

	if( int_depth == 1 )
	{
		OSPORT_DISABLE_INT();
		p_sch->lock_depth = int_depth;
	}
}

/*
 * Unlock local interrupts nested
 */
UTIL_SAFE
void sch_unlock_int( sch_cblk_t *p_sch )
{
	/*
	 * If failed:
	 * Trying to release a lock you do not own
	 * lock/unlock must be used in pairs
	 */
	UTIL_ASSERT( p_sch->lock_depth > 0 );

	/*
	 * If failed:
	 * Interrupt nested over 100 times, completely impossible.
	 * Underflow?
	 */
	UTIL_ASSERT( p_sch->lock_depth < 100 );

	p_sch->lock_depth--;

	if( p_sch->lock_depth == 0)
	{
		OSPORT_ENABLE_INT();
	}
}

/*
 * Reschedule threads, only sets next thread if has
 * a higher priority than current thread, will request
 * context switch if needed
 */
UTIL_UNSAFE
void sch_reschedule_req( sch_cblk_t *p_sch )
{
	uint_t counter;

	/*
	 * If failed:
	 * NULL pointer passed to p_sch
	 */
	UTIL_ASSERT( p_sch != NULL );

	for( counter = 0; counter < OSPORT_NUM_PRIOS; counter++ )
	{
		if( p_sch->q_ready[counter].p_head != NULL )
			break;
	}

	/*
	 * If failed:
	 * Idle thread missing
	 */
	UTIL_ASSERT( counter < OSPORT_NUM_PRIOS );

	/*
	 * If failed:
	 * Invalid current thread
	 */
	UTIL_ASSERT( p_sch->p_current != NULL );

	/*
	 * If failed:
	 * Invalid current thread priority
	 */
	UTIL_ASSERT( p_sch->p_current->item_sch.tag < OSPORT_NUM_PRIOS );

	if( counter < p_sch->p_current->item_sch.tag )
	{
		/*
		 * If failed:
		 * Cannot obtain thread from item
		 */
		UTIL_ASSERT( p_sch->q_ready[counter].p_head->p_thd != NULL );

		p_sch->p_next = p_sch->q_ready[counter].p_head->p_thd;

		/*
		 * If failed:
		 * Broken link
		 */
		UTIL_ASSERT( p_sch->q_ready[counter].p_head->p_next != NULL);

		p_sch->q_ready[counter].p_head =
				p_sch->q_ready[counter].p_head->p_next;

		if( p_sch->p_current != p_sch->p_next )
			OSPORT_CONTEXTSW_REQ();
	}
}

/*
 * Unload current thread, similar to reschedule, but doesn't
 * compare priority, instead, switches to a different thread
 */
UTIL_UNSAFE
void sch_unload_current( sch_cblk_t *p_sch )
{
	uint_t lock_depth;

	/*
	 * If failed:
	 * NULL pointer passed to p_sch
	 */
	UTIL_ASSERT( p_sch != NULL );

	sch_set_next_thread(p_sch);

	/*
	 * when yielding, it is possible that current thread
	 * still gets rescheduled. When this happens, only
	 * generate context switch when switching to a different
	 * thread
	 */
	if( p_sch->p_current != p_sch->p_next )
	{
		/*
		 * If failed:
		 * Invalid lock depth
		 */
		UTIL_ASSERT( p_sch->lock_depth > 0);

		/* save lock depth locally */
		lock_depth = p_sch->lock_depth;
		p_sch->lock_depth = 0;

		/* open a natural preemption point */
		OSPORT_ENABLE_INT();

		OSPORT_CONTEXTSW_REQ();

		/* close the preemption point */
		OSPORT_DISABLE_INT();

		/* restore lock depth */
		p_sch->lock_depth = lock_depth;
	}
}

/*
 * Handle heart beat
 */
UTIL_UNSAFE
void sch_handle_heartbeat( sch_cblk_t *p_sch )
{
	sch_qprio_t *p_q_temp;
	uint_t timestamp;
	sch_qitem_t *p_item;
	thd_cblk_t *p_thd;

	/*
	 * If failed:
	 * NULL pointer passed to p_sch
	 */
	UTIL_ASSERT( p_sch != NULL );

	/* increase time stamp */
	timestamp = p_sch->timestamp + 1;
	p_sch->timestamp = timestamp;

	/*
	 * If failed:
	 * Invalid delay queue pointers
	 */
	UTIL_ASSERT( p_sch->p_delayq_normal != NULL );
	UTIL_ASSERT( p_sch->p_delayq_overflow != NULL );

	/* time stamp overflowed */
	if( timestamp == 0 )
	{
		/*
		 * If failed:
		 * Normal queue not totally evicted
		 */
		UTIL_ASSERT( p_sch->p_delayq_normal->p_head == NULL );

		/* swap queues */
		p_q_temp = p_sch->p_delayq_normal;
		p_sch->p_delayq_normal = p_sch->p_delayq_overflow;
		p_sch->p_delayq_overflow = p_q_temp;
	}

	/* evict normal queue */
	while( p_sch->p_delayq_normal->p_head != NULL )
	{
		p_item = p_sch->p_delayq_normal->p_head;

		if( timestamp >= p_item->tag )
		{
			/*
			 * If failed:
			 * Cannot obtain thread from item
			 */
			UTIL_ASSERT( p_item->p_thd != NULL );

			/* obtain thread */
			p_thd = p_item->p_thd;

			thd_ready( p_thd, p_sch );
		}
		else
			break;
	}

	sch_set_next_thread( p_sch );

	if( p_sch->p_current != p_sch->p_next )
		OSPORT_CONTEXTSW_REQ();
}

/*
 * Insert an item onto the ready queue
 */
UTIL_UNSAFE void sch_insert_ready( sch_cblk_t *p_sch, sch_qitem_t *p_item )
{
	/*
	 * If failed:
	 * NULL pointer passed to p_sch or p_item
	 */
	UTIL_ASSERT( p_sch != NULL );
	UTIL_ASSERT( p_item != NULL );

	/*
	 * If failed:
	 * item not dequeued
	 */
	UTIL_ASSERT( p_item->p_q == NULL );

	/*
	 * If failed:
	 * Invalid priority
	 */
	UTIL_ASSERT( p_item->tag < OSPORT_NUM_PRIOS );

	/*
	 * If failed:
	 * Parent thread missing
	 */
	UTIL_ASSERT( p_item->p_thd != NULL );

	sch_qitem_enq_fifo( p_item, &p_sch->q_ready[p_item->tag] );
}

/*
 * Insert an item onto the delay queue
 */
UTIL_UNSAFE void sch_insert_delay( sch_cblk_t *p_sch, sch_qitem_t *p_item, uint_t timeout )
{
	uint_t wakeup, timestamp;

	/*
	 * If failed:
	 * NULL pointer passed to p_sch or p_item
	 */
	UTIL_ASSERT( p_sch != NULL );
	UTIL_ASSERT( p_item != NULL );

	/*
	 * If failed:
	 * item not dequeued
	 */
	UTIL_ASSERT( p_item->p_q == NULL );

	/*
	 * If failed:
	 * Parent thread missing
	 */
	UTIL_ASSERT( p_item->p_thd != NULL );

	/*
	 * If failed:
	 * Invalid timeout
	 */
	UTIL_ASSERT( timeout != 0);

	/* calculate wakeup */
	timestamp = p_sch->timestamp;
	wakeup = timestamp + timeout;

	p_item->tag = wakeup;

	/*
	 * If failed:
	 * Invalid delay queue pointers
	 */
	UTIL_ASSERT( p_sch->p_delayq_normal != NULL );
	UTIL_ASSERT( p_sch->p_delayq_overflow != NULL );

	/* wakeup timestamp overflowed */
	if( wakeup < timestamp )
	{
		/* insert onto overflow queue */
		sch_qitem_enq_prio( p_item, p_sch->p_delayq_overflow );
	}
	else
	{
		/* insert onto normal queue */
		sch_qitem_enq_prio( p_item, p_sch->p_delayq_normal );
	}
}

/*
 * Initialize a thread control block (create a thread)
 */
UTIL_UNSAFE
void thd_init( thd_cblk_t *p_thd, uint_t prio, void *p_stack, uint_t stack_size,
		void (*p_job)(void), void (*p_return)(void) )
{
	/*
	 * If failed:
	 * Invalid parameters
	 */
	UTIL_ASSERT(p_thd != NULL);
	UTIL_ASSERT(p_stack != NULL);
	UTIL_ASSERT(stack_size != 0);
	UTIL_ASSERT(p_job != NULL);
	UTIL_ASSERT(p_return != NULL);
	UTIL_ASSERT(prio < OSPORT_NUM_PRIOS);

	p_thd->p_stack = p_stack;
	p_thd->p_sp = OSPORT_INIT_STACK(p_stack, stack_size, p_job, p_return );
	p_thd->state = THD_STATE_READY;
	p_thd->p_schinfo = NULL;

	sch_qitem_init( &p_thd->item_sch, p_thd, prio );
	sch_qitem_init( &p_thd->item_delay, p_thd, 0 );
	mlst_init( &p_thd->mlst );
}

/*
 * Ready a thread
 */
UTIL_UNSAFE
void thd_ready( thd_cblk_t *p_thd, sch_cblk_t *p_sch )
{
	/*
	 * If failed:
	 * NULL pointer passed to p_sch or p_thd
	 */
	UTIL_ASSERT( p_sch != NULL);
	UTIL_ASSERT( p_thd != NULL );

	if( p_thd->item_sch.p_q != NULL )
		sch_qitem_remove( &p_thd->item_sch );

	if( p_thd->item_delay.p_q != NULL )
		sch_qitem_remove( &p_thd->item_delay );

	p_thd->p_schinfo = NULL;

	/* change state to ready */
	p_thd->state = THD_STATE_READY;

	sch_insert_ready( p_sch, &p_thd->item_sch );
}

/*
 * Block current thread to optional resource list
 */
UTIL_UNSAFE
void thd_block_current( sch_qprio_t *p_to, void *p_schinfo, uint_t timeout,
		sch_cblk_t *p_sch )
{
	thd_cblk_t *p_thd;

	/*
	 * If failed
	 * Invalid parameters
	 */
	UTIL_ASSERT( p_sch != NULL );

	/*
	 * If failed:
	 * Invalid current thread
	 */
	UTIL_ASSERT( p_sch->p_current != NULL );
	p_thd = p_sch->p_current;

	/*
	 * If failed:
	 * Current thread not in a ready state
	 */
	UTIL_ASSERT( p_thd->state == THD_STATE_READY );
	UTIL_ASSERT( p_thd->item_sch.p_q != NULL );
	UTIL_ASSERT( p_thd->item_delay.p_q == NULL );

	p_thd->state = THD_STATE_BLOCKED;

	/* remove from ready list */
	sch_qitem_remove( &p_thd->item_sch );

	/* attach scheduling info */
	p_thd->p_schinfo = p_schinfo;

	/* insert into resource list, if any */
	if( p_to != NULL )
		sch_qitem_enq_prio( &p_thd->item_sch, p_to );

	/* put onto delay queue */
	if( timeout != 0)
		sch_insert_delay( p_sch, &p_thd->item_delay, timeout );

	sch_unload_current( p_sch );

	/*
	 * If failed:
	 * Current thread should be ready after resume
	 */
	UTIL_ASSERT( p_thd->state == THD_STATE_READY );
	UTIL_ASSERT( p_thd->item_sch.p_q != NULL );
	UTIL_ASSERT( p_thd->item_delay.p_q == NULL );
	UTIL_ASSERT( p_thd->p_schinfo == NULL);
}

/*
 * Create a thread using static memory
 */
UTIL_UNSAFE
void thd_create_static(thd_cblk_t *p_thd, uint_t prio, void *p_stack,
		uint_t stack_size, void (*p_job)(void), sch_cblk_t *p_sch)
{
	/*
	 * If failed:
	 * Invalid parameters
	 */
	UTIL_ASSERT(p_thd != NULL);
	UTIL_ASSERT(p_stack != NULL);
	UTIL_ASSERT(stack_size != 0);
	UTIL_ASSERT(p_job != NULL);
	UTIL_ASSERT(prio < OSPORT_NUM_PRIOS);

	thd_init(p_thd, prio, p_stack, stack_size, p_job, thd_return_hook_static);
	thd_ready( p_thd, p_sch );

	/* only request reschedule when current thread was loaded */
	if( p_sch->p_current != NULL )
	{
		sch_reschedule_req(p_sch);
	}
}

/*
 * Delete a static thread
 */
UTIL_UNSAFE
void thd_delete_static(thd_cblk_t *p_thd, sch_cblk_t *p_sch)
{
	mblk_t *p_mblk;

	/*
	 * If failed:
	 * Invalid parameters
	 */
	UTIL_ASSERT(p_thd != NULL );
	UTIL_ASSERT(p_sch != NULL );

	/*
	 * If failed:
	 * Thread killed twice
	 */
	UTIL_ASSERT( p_thd->state != THD_STATE_DELETED );

	p_thd->state = THD_STATE_DELETED;

	/* remove scheduling item */
	if( p_thd->item_sch.p_q != NULL )
		sch_qitem_remove( &p_thd->item_sch );

	/* remove delay item */
	if( p_thd->item_delay.p_q != NULL )
		sch_qitem_remove( &p_thd->item_delay );

	/* release memory */
	while( p_thd->mlst.p_head != NULL )
	{
		p_mblk = p_thd->mlst.p_head;
		mlst_remove( p_mblk );
		mpool_insert( p_mblk, &g_mpool );
	}

	p_thd->p_schinfo = NULL;

	if( p_thd == p_sch->p_current )
	{
		sch_unload_current(p_sch);
	}
}

/*
 * Static threads return here
 */
UTIL_SAFE
void thd_return_hook_static( void )
{
	thd_cblk_t *p_thd;

	UTIL_LOCK_EVERYTHING();

	p_thd = g_sch.p_current;

	/*
	 * If failed:
	 * Invalid current thread
	 */
	UTIL_ASSERT( p_thd != NULL)

	thd_delete_static( p_thd, &g_sch);

	/*
	 * reschedule request doesn't get serviced until this
	 * function exits
	 */
	UTIL_UNLOCK_EVERYTHING();

	/*
	 * If failed:
	 * Should never come back here
	 */
	UTIL_ASSERT(0);
}

#include "../include/api.h"

/**
 * @brief Create a thread, allocating necessary memory automatically
 * @param prio priority of the thread
 * @param stack_size stack size
 * @param p_job pointer to a job
 * @retval 0 thread creation failed because of low memory
 * @retval !0 handle to created thread
 * @details This function can be used to create a thread that is mortal
 * (only runs a period of time, after which resources are released).
 * The operating system does not implement parenting and thus the thread will
 * not be automatically killed if the parent thread has died. Will trigger
 * reschedule immediately if the created thread has a higher priority.
 * @note This function is thread safe and can be used in thread or interrupt
 * context.
 */
UTIL_SAFE
os_handle_t os_thread_create( os_uint_t prio, os_uint_t stack_size, void (*p_job)(void) )
{
	void *p_stack;
	thd_cblk_t *p_thd = NULL;

	/*
	 * If failed:
	 * Invalid parameters
	 */
	UTIL_ASSERT( stack_size > 0);
	UTIL_ASSERT( p_job != NULL );

	/*
	 * If failed:
	 * Trying to create using idle thread priority
	 */
	UTIL_ASSERT( prio < OSPORT_NUM_PRIOS - 1 );

	/* allocate memory */
	UTIL_LOCK_EVERYTHING();
	p_stack = mpool_alloc( stack_size, &g_mpool, &g_mlst );

	if( p_stack != NULL )
	{
		p_thd = (thd_cblk_t*)mpool_alloc( sizeof(thd_cblk_t), &g_mpool, &g_mlst );

		if( p_thd == NULL )
			mpool_free(p_stack, &g_mpool);
		else
		{
			thd_init( p_thd, prio, p_stack, stack_size, p_job, thd_return_hook );
			thd_ready( p_thd, &g_sch );

			/* only request reschedule when current thread was loaded */
			if( g_sch.p_current != NULL )
			{
				sch_reschedule_req(&g_sch);
			}
		}
	}

	UTIL_UNLOCK_EVERYTHING();
	return (os_handle_t)p_thd;
}

/**
 * @brief Delete a thread, free all memory
 * @param h_thread thread handle
 * @note This function is thread safe and can be used in thread
 * or interrupt context.
 */
UTIL_SAFE
void os_thread_delete( os_handle_t h_thread )
{
	thd_cblk_t *p_thd;
	mblk_t *p_mblk;

	UTIL_LOCK_EVERYTHING();

	if( h_thread == 0)
		p_thd = g_sch.p_current;
	else
		p_thd = (thd_cblk_t*)h_thread;

	/*
	 * If failed:
	 * Invalid current thread
	 */
	UTIL_ASSERT(p_thd != NULL);

	p_thd->state = THD_STATE_DELETED;

	/* remove scheduling item */
	if( p_thd->item_sch.p_q != NULL )
		sch_qitem_remove( &p_thd->item_sch );

	/* remove delay item */
	if( p_thd->item_delay.p_q != NULL )
		sch_qitem_remove( &p_thd->item_delay );

	/* release memory */
	while( p_thd->mlst.p_head != NULL )
	{
		p_mblk = p_thd->mlst.p_head;
		mlst_remove( p_mblk );
		mpool_insert( p_mblk, &g_mpool );
	}

	p_thd->p_schinfo = NULL;

	/*
	 * If failed:
	 * stack lost
	 */
	UTIL_ASSERT( p_thd->p_stack != NULL );

	/* free memory */
	mpool_free(p_thd->p_stack, &g_mpool);
	mpool_free(p_thd, &g_mpool);

	if( p_thd == g_sch.p_current )
	{
		sch_unload_current(&g_sch);
	}

	UTIL_UNLOCK_EVERYTHING();
}

/*
 * Thread created by os_thread_create will return here
 */
UTIL_SAFE
void thd_return_hook( void )
{
	/* delete myself */
	os_thread_delete(0);

	/*
	 * If failed:
	 * Should not come back here
	 */
	UTIL_ASSERT(0);
}

/**
 * @brief Request reschedule
 * @note This function is thread safe and can be used
 * in an interrupt or thread context.
 */
UTIL_SAFE
void os_thread_yield( void )
{
	UTIL_LOCK_EVERYTHING();
	sch_unload_current(&g_sch);
	/*
	 * If failed:
	 * Current thread should be ready after resume
	 */
	UTIL_ASSERT( g_sch.p_current->state == THD_STATE_READY );
	UTIL_ASSERT( g_sch.p_current->item_sch.p_q != NULL );
	UTIL_ASSERT( g_sch.p_current->item_delay.p_q == NULL );
	UTIL_ASSERT( g_sch.p_current->p_schinfo == NULL );

	UTIL_UNLOCK_EVERYTHING();
}

/**
 * @brief Sleep and then join the ready queue
 * @param timeout time in ticks to sleep
 * @note This function is thread safe and can only be used
 * in a thread context.
 */
UTIL_SAFE
void os_thread_delay( os_uint_t timeout )
{
	if( timeout != 0)
	{
		UTIL_LOCK_EVERYTHING();
		thd_block_current(NULL, NULL, timeout, &g_sch);
		UTIL_UNLOCK_EVERYTHING();
	}
}

/**
 * @brief Get thread priority
 * @param h_thread thread handle, pass 0 for current thread
 * @return priority of h_thread
 * @note This function is thread safe and can be used in an
 * interrupt or a thread context.
 */
UTIL_SAFE
os_uint_t os_thread_get_priority( os_handle_t h_thread )
{
	thd_cblk_t *p_thd;
	os_uint_t ret;

	UTIL_LOCK_EVERYTHING();

	if( h_thread == 0)
		p_thd = g_sch.p_current;
	else
		p_thd = (thd_cblk_t*)h_thread;

	/*
	 * If failed:
	 * Invalid priority
	 */
	UTIL_ASSERT( p_thd->item_sch.tag < OSPORT_NUM_PRIOS );

	ret = p_thd->item_sch.tag;

	UTIL_UNLOCK_EVERYTHING();

	return ret;
}

/**
 * @brief Get current thread handle
 * @return current thread handle
 * @note This function is thread safe and can be used
 * in an interrupt or a thread context.
 */
UTIL_SAFE
os_handle_t os_thread_get_current( void )
{
	thd_cblk_t *p_thd;

	UTIL_LOCK_EVERYTHING();

	/*
	 * If failed:
	 * Invalid current thread
	 */
	UTIL_ASSERT( g_sch.p_current != NULL );

	p_thd = g_sch.p_current;
	UTIL_UNLOCK_EVERYTHING();

	return (os_handle_t)p_thd;
}

/**
 * @brief Obtain state of thread
 * @param h_thread thread handle, pass 0 for current thread
 * @return state of the thread
 * @note This function is thread safe and can be used in
 * an interrupt or a thread context.
 */
UTIL_SAFE
os_thread_state_t os_thread_get_state( os_handle_t h_thread )
{
	thd_cblk_t *p_thd;
	os_thread_state_t ret;

	UTIL_LOCK_EVERYTHING();

	if( h_thread == 0)
		p_thd = g_sch.p_current;
	else
		p_thd = (thd_cblk_t*)h_thread;

	ret = (os_thread_state_t)p_thd->state;

	UTIL_UNLOCK_EVERYTHING();

	return ret;
}

/**
 * @brief Suspend thread
 * @param h_thread thread to suspend, pass 0 to current thread
 * @note This function is thread safe and can be used in an
 * interrupt or a thread context.
 */
UTIL_SAFE
void os_thread_suspend( os_handle_t h_thread )
{
	thd_cblk_t *p_thd;

	UTIL_LOCK_EVERYTHING();

	if( h_thread == 0)
		p_thd = g_sch.p_current;
	else
		p_thd = (thd_cblk_t*)h_thread;

	if( p_thd->state != THD_STATE_SUSPENDED )
	{
		p_thd->state = THD_STATE_SUSPENDED;

		/* remove scheduling item */
		if( p_thd->item_sch.p_q != NULL )
			sch_qitem_remove( &p_thd->item_sch );

		/* remove delay item */
		if( p_thd->item_delay.p_q != NULL )
			sch_qitem_remove( &p_thd->item_delay );

		/* remove scheduling info */
		p_thd->p_schinfo = NULL;

		if( p_thd == g_sch.p_current )
		{
			sch_unload_current(&g_sch);

			/*
			 * If failed:
			 * Current thread should be ready after resume
			 */
			UTIL_ASSERT( p_thd->state == THD_STATE_READY );
			UTIL_ASSERT( p_thd->item_sch.p_q != NULL );
			UTIL_ASSERT( p_thd->item_delay.p_q == NULL );
			UTIL_ASSERT( p_thd->p_schinfo == NULL );
		}
	}

	UTIL_UNLOCK_EVERYTHING();
}

/**
 * @brief Resume thread
 * @param h_thread thread to resume, must be non zero
 * @details Has no effect on a ready thread.
 * @note This function is thread safe and can be used in
 * an interrupt or a thread context.
 */
UTIL_SAFE
void os_thread_resume( os_handle_t h_thread )
{
	thd_cblk_t *p_thd;

	/*
	 * If failed:
	 * Thread cannot resume itself
	 */
	UTIL_ASSERT( h_thread != 0);

	p_thd = (thd_cblk_t*)h_thread;

	UTIL_LOCK_EVERYTHING();
	thd_ready( p_thd, &g_sch );
	UTIL_UNLOCK_EVERYTHING();
}


/**
 * @brief Set thread priority
 * @param h_thread handle to thread to set priority, pass 0 for current
 * thread
 * @param prio priority
 * @note This function is thread safe and can be used in an interrupt
 * or a thread context.
 */
void os_thread_set_priority( os_handle_t h_thread, os_uint_t prio )
{
	thd_cblk_t *p_thd;
	sch_qprio_t *p_qprio;

	/*
	 * If failed:
	 * Invalid prio
	 */
	UTIL_ASSERT( prio < OSPORT_NUM_PRIOS);

	UTIL_LOCK_EVERYTHING();

	if( h_thread == 0)
		p_thd = g_sch.p_current;
	else
		p_thd = (thd_cblk_t*)h_thread;

	switch( p_thd->state )
	{
	case THD_STATE_DELETED:
	case THD_STATE_SUSPENDED:
		/*
		 * If failed:
		 * Scheduling item queued when shouldn't be
		 */
		UTIL_ASSERT( p_thd->item_sch.p_q == NULL );

		p_thd->item_sch.tag = prio;

		break;

	case THD_STATE_READY:

		if( p_thd->item_sch.p_q != NULL )
		{
			sch_qitem_remove( &p_thd->item_sch );
			sch_qitem_enq_fifo( &p_thd->item_sch, &g_sch.q_ready[prio]);
		}

		p_thd->item_sch.tag = prio;

		break;

	case THD_STATE_BLOCKED:

		p_qprio = p_thd->item_sch.p_q;

		if( p_qprio != NULL )
			sch_qitem_remove( &p_thd->item_sch );

		p_thd->item_sch.tag = prio;

		if(p_qprio != NULL )
			sch_qitem_enq_prio( &p_thd->item_sch, p_qprio);

		break;

	default:
		/*
		 * If failed:
		 * Invalid state
		 */
		UTIL_ASSERT(0);
		break;
	}

	UTIL_UNLOCK_EVERYTHING();
}

