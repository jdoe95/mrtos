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

	p_sch->timestamp = 0;
	p_sch->p_current = NULL;
	p_sch->p_next = NULL;
	p_sch->p_delayq_normal = &p_sch->q_delay1;
	p_sch->p_delayq_overflow = &p_sch->q_delay2;
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
	UTIL_ASSERT(p_stack != NULL );
	UTIL_ASSERT( stack_size != 0);
	UTIL_ASSERT( p_job != NULL );
	UTIL_ASSERT( p_return != NULL );
	UTIL_ASSERT( prio < OSPORT_NUM_PRIOS );

	p_thd->p_stack = p_stack;
	p_thd->p_sp = OSPORT_INIT_STACK(p_stack, stack_size, p_job, p_return );
	p_thd->state = THD_STATE_READY;
	p_thd->p_schinfo = NULL;

	sch_qitem_init( &p_thd->item_sch, p_thd, prio );
	sch_qitem_init( &p_thd->item_delay, p_thd, 0 );
	mlst_init( &p_thd->mlst );
}

/*
 * Reschedule threads
 */
UTIL_UNSAFE
void sch_reschedule( sch_cblk_t *p_sch )
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

	if( counter < p_sch->p_current->item_sch.tag )
	{
		p_sch->p_next = p_sch->q_ready[counter].p_head->p_thd;

		/*
		 * If failed:
		 * Broken link
		 */
		UTIL_ASSERT( p_sch->q_ready[counter].p_head->p_next != NULL);

		p_sch->q_ready[counter].p_head =
				p_sch->q_ready[counter].p_head->p_next;

		OSPORT_CONTEXTSW_REQ();
	}
}

/*
 * Handle heart beat
 */
UTIL_UNSAFE
void sch_heartbeat( sch_cblk_t *p_sch )
{
	sch_qprio_t *p_q_temp;
	uint_t timestamp;
	sch_qitem_t *p_item;
	thd_cblk_t *p_thd;
	uint_t counter;

	/*
	 * If failed:
	 * NULL pointer passed to p_sch
	 */
	UTIL_ASSERT( p_sch != NULL );

	/* increase time stamp */
	timestamp = p_sch->timestamp + 1;
	p_sch->timestamp = timestamp;

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

	/*
	 * If failed:
	 * Invalid delay queue pointers
	 */
	UTIL_ASSERT( p_sch->p_delayq_normal != NULL );
	UTIL_ASSERT( p_sch->p_delayq_overflow != NULL );

	/* evict normal queue */
	while( p_sch->p_delayq_normal->p_head != NULL )
	{
		p_item = p_sch->p_delayq_normal->p_head;

		if( p_item->tag >= timestamp )
		{
			/* obtain thread */
			p_thd = p_item->p_thd;

			/*
			 * If failed:
			 * cannot obtain thread from delay item
			 */
			UTIL_ASSERT( p_thd != NULL );

			sch_ready( p_sch, p_thd);
		}
		else
			break;
	}

	/* reschedule new priority */
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

	if( counter <= p_sch->p_current->item_sch.tag )
	{
		p_sch->p_next = p_sch->q_ready[counter].p_head->p_thd;

		/*
		 * If failed:
		 * Corrupted queue
		 */
		UTIL_ASSERT( p_sch->q_ready[counter].p_head->p_next != NULL);

		p_sch->q_ready[counter].p_head =
				p_sch->q_ready[counter].p_head->p_next;

		OSPORT_CONTEXTSW_REQ();
	}
}

/*
 * Ready a thread
 */
UTIL_UNSAFE
void sch_ready( sch_cblk_t *p_sch, thd_cblk_t *p_thd )
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

	/*
	 * If failed:
	 * Invalid prio
	 */
	UTIL_ASSERT( p_thd->item_sch.tag < OSPORT_NUM_PRIOS );

	/* put back on ready queue */
	sch_qitem_enq_fifo( &p_thd->item_sch, &p_sch->q_ready[p_thd->item_sch.tag] );
}

/*
 * Block current thread
 */
UTIL_UNSAFE
void sch_block_current( sch_cblk_t *p_sch, sch_qprio_t *p_q, void *p_schinfo,
		uint_t timeout )
{
	thd_cblk_t *p_thd;
	uint_t counter, wakeup;

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
	UTIL_ASSERT( p_thd->state != THD_STATE_READY );
	UTIL_ASSERT( p_thd->item_sch.p_q != NULL );
	UTIL_ASSERT( p_thd->item_delay.p_q == NULL );

	p_thd->state = THD_STATE_BLOCKED;

	/* remove from ready list */
	sch_qitem_remove( &p_thd->item_sch );
	p_thd->p_schinfo = p_schinfo;

	/* insert into resource list, if any */
	if( p_q != NULL )
		sch_qitem_enq_prio( &p_thd->item_sch, p_q );

	/* put onto delay queue */
	if( timeout != 0)
	{
		/* calculate wakeup */
		wakeup = p_sch->timestamp + timeout;
		p_thd->item_delay.tag = wakeup;

		/* overflowed */
		if( wakeup < p_sch->timestamp )
			sch_qitem_enq_prio( &p_thd->item_delay, p_sch->p_delayq_overflow );
		else
			sch_qitem_enq_prio( &p_thd->item_delay, p_sch->p_delayq_normal );
	}

	/* reschedule immediately */
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

	p_sch->p_next = p_sch->q_ready[counter].p_head->p_thd;

	/*
	 * If failed:
	 * Broken link
	 */
	UTIL_ASSERT( p_sch->q_ready[counter].p_head->p_next != NULL );

	p_sch->q_ready[counter].p_head =
			p_sch->q_ready[counter].p_head->p_next;

	OSPORT_CONTEXTSW_REQ();
}

/*
 * Remove thread from scheduler
 */
UTIL_UNSAFE void sch_remove( sch_cblk_t *p_sch, thd_cblk_t *p_thd )
{
	uint_t counter;

	/*
	 * If failed:
	 * Invalid parameters
	 */
	UTIL_ASSERT( p_sch != NULL );
	UTIL_ASSERT( p_thd != NULL );

	if( p_thd->item_sch.p_q != NULL )
		sch_qitem_remove( &p_thd->item_sch );

	if( p_thd->item_delay.p_q != NULL )
		sch_qitem_remove( &p_thd->item_delay );

	p_thd->p_schinfo = NULL;

	if( p_thd == p_sch->p_current )
	{
		/* reschedule immediately */
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

		p_sch->p_next = p_sch->q_ready[counter].p_head->p_thd;

		/*
		 * If failed:
		 * Broken link
		 */
		UTIL_ASSERT( p_sch->q_ready[counter].p_head->p_next != NULL );

		p_sch->q_ready[counter].p_head =
				p_sch->q_ready[counter].p_head->p_next;

		OSPORT_CONTEXTSW_REQ();
	}
}

/*
 * Change scheduler prio
 */
UTIL_UNSAFE
void sch_change_prio( sch_cblk_t *p_sch, thd_cblk_t *p_thd, uint_t prio )
{
	sch_qprio_t *p_qprio;
	/*
	 * If failed:
	 * Invalid parameter
	 */
	UTIL_ASSERT( p_sch != NULL );
	UTIL_ASSERT( p_thd != NULL );
	UTIL_ASSERT( prio < OSPORT_NUM_PRIOS);

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
		/*
		 * If failed:
		 * Thread not queued
		 */
		UTIL_ASSERT( p_thd->item_sch.p_q != NULL );
		sch_qitem_remove( &p_thd->item_sch );
		p_thd->item_sch.tag = prio;
		sch_qitem_enq_fifo( &p_thd->item_sch, &p_sch->q_ready[prio]);

		break;

	case THD_STATE_BLOCKED:

		/*
		 * If failed:
		 * Thread not queued
		 */
		UTIL_ASSERT( p_thd->item_sch.p_q != NULL );

		p_qprio = p_thd->item_sch.p_q;
		sch_qitem_remove( &p_thd->item_sch );
		p_thd->item_sch.tag = prio;
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
}

/**************************************************************************************
 * Internal thread functions
 **************************************************************************************/
UTIL_SAFE void thd_return( void );

/*
 * Create thread internally
 */
UTIL_SAFE
void thd_create( thd_cblk_t *p_thd, uint_t prio, void *p_stack, uint_t stack_size,
		void (*p_job)(void) )
{
	UTIL_LOCK_EVERYTHING();
	thd_init( p_thd, prio, p_stack, stack_size, p_job, thd_return );
	sch_ready( &g_sch, p_thd );
	UTIL_UNLOCK_EVERYTHING();
}

/*
 * Start or resume a thread (reschedule)
 */
UTIL_SAFE
void thd_resume( thd_cblk_t *p_thd )
{
	/*
	 * If failed:
	 * Invalid parameter
	 */
	UTIL_ASSERT( p_thd != NULL );

	UTIL_LOCK_EVERYTHING();

	if( p_thd->state != THD_STATE_READY )
	{
		sch_ready(&g_sch, p_thd);
		sch_reschedule( &g_sch );
	}

	UTIL_UNLOCK_EVERYTHING();
}

/*
 * Stop a thread
 */
UTIL_UNSAFE
void thd_suspend( thd_cblk_t *p_thd )
{
	/*
	 * If failed:
	 * Invalid parameter
	 */
	UTIL_ASSERT( p_thd != NULL );

	UTIL_LOCK_EVERYTHING();

	if( p_thd->state != THD_STATE_SUSPENDED )
	{
		sch_remove( &g_sch, p_thd );
		p_thd->state = THD_STATE_SUSPENDED;
	}

	UTIL_UNLOCK_EVERYTHING();
}

/*
 * Delete a thread
 */
UTIL_UNSAFE
void thd_delete( thd_cblk_t *p_thd )
{
	mblk_t *p_mblk;

	/*
	 * If failed:
	 * Invalid parameter
	 */
	UTIL_ASSERT( p_thd != NULL );

	UTIL_LOCK_EVERYTHING();

	/*
	 * If failed
	 * Thread already deleted
	 */
	UTIL_ASSERT( p_thd->state != THD_STATE_DELETED );

	/* remove from scheduler */
	sch_remove( &g_sch, p_thd );
	p_thd->state = THD_STATE_DELETED;

	/* free memory */
	while( p_thd->mlst.p_head != NULL )
	{
		p_mblk = p_thd->mlst.p_head;
		mlst_remove( p_mblk );
		mpool_insert( p_mblk, &g_mpool );
	}

	UTIL_UNLOCK_EVERYTHING();
}

/*
 * Internal thread return
 */
UTIL_SAFE
void thd_return( void )
{
	UTIL_LOCK_EVERYTHING();
	thd_delete(g_sch.p_current);
	UTIL_UNLOCK_EVERYTHING();
}

/*
 * Internal change priority
 */
UTIL_SAFE
void thd_change_prio( thd_cblk_t *p_thd, uint_t prio )
{
	/*
	 * If failed:
	 * Invalid parameter
	 */
	UTIL_ASSERT( p_thd != NULL );

	UTIL_LOCK_EVERYTHING();
	sch_change_prio( &g_sch, p_thd, prio );
	UTIL_UNLOCK_EVERYTHING();
}

/*
 * Internal yield thread
 */
UTIL_SAFE void thd_yield( void )
{
	UTIL_LOCK_EVERYTHING();
	sch_reschedule( &g_sch );
	UTIL_UNLOCK_EVERYTHING();
}

UTIL_SAFE void thd_delay( uint_t timeout )
{
	UTIL_LOCK_EVERYTHING();

	if( timeout != 0)
		sch_block_current( &g_sch, NULL, NULL, timeout );

	UTIL_UNLOCK_EVERYTHING();
}

/**************************************************************************************
 * External thread functions
 **************************************************************************************/
#include "../include/api.h"

void os_thread_return( void );

/*
 * Create a thread, allocating memory automatically
 */
os_handle_t os_thread_create( os_uint_t prio, os_uint_t stack_size, void (*p_job)(void) )
{
	void *p_stack;
	thd_cblk_t *p_thd = NULL;

	/*
	 * If failed:
	 * Invalid parameters
	 */
	UTIL_ASSERT( prio < OSPORT_NUM_PRIOS );
	UTIL_ASSERT( stack_size > 0);
	UTIL_ASSERT( p_job != NULL );

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
			thd_init( p_thd, prio, p_stack, stack_size, p_job, os_thread_return );
			sch_ready( &g_sch, p_thd );
		}
	}

	UTIL_UNLOCK_EVERYTHING();

	return (os_handle_t)p_thd;
}

/*
 * Delete a thread, free all memory
 */
void os_thread_delete( os_handle_t h_thread )
{
	thd_cblk_t *p_thd;

	UTIL_LOCK_EVERYTHING();

	if( h_thread == 0)
		p_thd = g_sch.p_current;
	else
		p_thd = (thd_cblk_t*)h_thread;

	thd_delete(p_thd);

	/* free stack and thread memory */
	mpool_free( p_thd->p_stack, &g_mpool );
	mpool_free(p_thd, &g_mpool );

	UTIL_UNLOCK_EVERYTHING();
}

/*
 * Suspend thread
 */
void os_thread_suspend( os_handle_t h_thread )
{
	thd_cblk_t *p_thd;

	UTIL_LOCK_EVERYTHING();

	if( h_thread == 0)
		p_thd = g_sch.p_current;
	else
		p_thd = (thd_cblk_t*)h_thread;

	thd_suspend(p_thd);

	UTIL_UNLOCK_EVERYTHING();
}

/*
 * Resume thread
 */
void os_thread_resume( os_handle_t h_thread )
{
	thd_cblk_t *p_thd;

	/*
	 * If failed:
	 * Thread cannot resume itself
	 */
	UTIL_ASSERT( h_thread != 0);

	p_thd = (thd_cblk_t*)h_thread;

	thd_resume(p_thd);
}

/*
 * Obtain state of thread
 */
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

/*
 * Get current thread handle
 */
os_handle_t os_thread_get_current( void )
{
	thd_cblk_t *p_thd;

	UTIL_LOCK_EVERYTHING();
	p_thd = g_sch.p_current;
	UTIL_UNLOCK_EVERYTHING();

	return (os_handle_t)p_thd;
}

/*
 * Set thread priority
 */
void os_thread_set_priority( os_handle_t h_thread, os_uint_t prio )
{
	thd_cblk_t *p_thd;

	UTIL_LOCK_EVERYTHING();

	if( h_thread == 0)
		p_thd = g_sch.p_current;
	else
		p_thd = (thd_cblk_t*)h_thread;

	thd_change_prio(p_thd, prio);

	UTIL_UNLOCK_EVERYTHING();
}

/*
 * Get thread priority
 */
os_uint_t os_thread_get_priority( os_handle_t h_thread )
{
	thd_cblk_t *p_thd;
	os_uint_t ret;

	UTIL_LOCK_EVERYTHING();

	if( h_thread == 0)
		p_thd = g_sch.p_current;
	else
		p_thd = (thd_cblk_t*)h_thread;

	ret = p_thd->item_sch.tag;

	UTIL_UNLOCK_EVERYTHING();

	return ret;
}

/*
 * Request reschedule
 */
void os_thread_yield( void )
{
	thd_yield();
}

/*
 * Sleep for sometime
 */
void os_thread_delay( os_uint_t timeout )
{
	thd_delay(timeout);
}

/*
 * Thread return hook
 */
void os_thread_return( void )
{
	os_thread_delete(0);
}

/**
 * @brief Start the kernel
 */
UTIL_SAFE
void os_start( void )
{
	uint_t counter;

	UTIL_LOCK_EVERYTHING();

	for( counter = 0; counter < OSPORT_NUM_PRIOS; counter++ )
	{
		if( g_sch.q_ready[counter].p_head != NULL )
			break;
	}

	/*
	 * If failed:
	 * Idle thread missing
	 */
	UTIL_ASSERT( counter < OSPORT_NUM_PRIOS );

	g_sch.p_current = g_sch.q_ready[counter].p_head->p_thd;

	/*
	 * If failed:
	 * Broken link
	 */
	UTIL_ASSERT( g_sch.q_ready[counter].p_head->p_next != NULL );

	g_sch.q_ready[counter].p_head =
			g_sch.q_ready[counter].p_head->p_next;


	UTIL_UNLOCK_EVERYTHING();

	/* call portable init function to start kernel */
	OSPORT_INIT();
}

/**
 * @brief Handle heart beat
 * @note This function can be used in a thread
 * or interrupt context
 */
UTIL_SAFE
void os_handle_heartbeat( void )
{
	UTIL_LOCK_EVERYTHING();
	sch_heartbeat(&g_sch);
	UTIL_UNLOCK_EVERYTHING();
}

/**
 * @brief Returns current OS time in ticks
 * @note This function can be used in a thread or
 * interrupt context.
 */
UTIL_SAFE
os_uint_t os_get_heartbeat_counter( void )
{
	os_uint_t ret;

	UTIL_LOCK_EVERYTHING();
	ret = g_sch.timestamp;
	UTIL_UNLOCK_EVERYTHING();

	return ret;
}

/**
 * @brief Enters an exclusive critical section
 * @note This function can be used in a thread or
 * interrupt context
 */
UTIL_SAFE
void os_enter_critical( void )
{
	UTIL_LOCK_EVERYTHING();
}

/**
 * @brief Exits an exclusive critical section
 * @note This function can be used in a thread or
 * interrupt context.
 */
UTIL_SAFE
void os_exit_critical( void )
{
	UTIL_UNLOCK_EVERYTHING();
}
