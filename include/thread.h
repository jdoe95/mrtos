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
#ifndef H22A9CDB9_7059_460F_BAEE_7953ABC94E21
#define H22A9CDB9_7059_460F_BAEE_7953ABC94E21

#include "util.h"
#include "memory.h"

struct sch_qitem_s;
struct sch_qfifo_s;
struct sch_qprio_s;
struct sch_q_s;

typedef struct sch_qitem_s sch_qitem_t;
typedef struct sch_qfifo_s sch_qfifo_t;
typedef struct sch_qprio_s sch_qprio_t;
typedef struct sch_q_s sch_q_t;

struct sch_cblk_s;
struct thd_cblk_s;

typedef struct sch_cblk_s sch_cblk_t;
typedef struct thd_cblk_s thd_cblk_t;

/*
 * Thread state
 */
typedef enum
{
	THD_STATE_READY = 0,
	THD_STATE_BLOCKED,
	THD_STATE_SUSPENDED,
	THD_STATE_DELETED
} thd_state_t;

/*
 * Scheduler queue item
 * Order of members makes a difference.
 */
struct sch_qitem_s
{
	struct sch_qitem_s *volatile p_prev; /* previous item          */
	struct sch_qitem_s *volatile p_next; /* next item              */
	struct thd_cblk_s *volatile p_thd;   /* thread                 */
	void *volatile p_q;                  /* parent queue           */
	volatile uint_t tag;                 /* tag value for ordering */
};

/*
 * Scheduler generic queue
 * Order of members makes a difference.
 */
struct sch_q_s
{
	struct sch_qitem_s *volatile p_head; /* queue head */
};

/*
 * Scheduler FIFO queue header
 * Order of members makes a difference.
 */
struct sch_qfifo_s
{
	struct sch_qitem_s *volatile p_head; /* queue head */
};

/*
 * Scheduler priority queue header
 * Order of members makes a difference.
 */
struct sch_qprio_s
{
	struct sch_qitem_s *volatile p_head; /* queue head */
};

/*
 * Initialization functions
 */
UTIL_UNSAFE void sch_qitem_init( sch_qitem_t *p_item, thd_cblk_t *p_thd, uint_t tag );
UTIL_UNSAFE void sch_q_init( void *p_q );

/*
 * Item manipulation
 */
UTIL_UNSAFE void sch_qitem_enq_fifo( sch_qitem_t *p_item, sch_qfifo_t *p_fifo );
UTIL_UNSAFE void sch_qitem_enq_prio( sch_qitem_t *p_item, sch_qprio_t *p_prio );
UTIL_UNSAFE sch_qitem_t* sch_qitem_deq( void *p_q );
UTIL_UNSAFE void sch_qitem_remove( sch_qitem_t *p_item );

/*
 * Scheduler control block
 * Order of members makes a difference.
 */
struct sch_cblk_s
{
	struct thd_cblk_s *volatile p_current;          /* currently loaded thread */
	struct thd_cblk_s *volatile p_next;             /* thread to be run next   */
	struct sch_qprio_s *volatile p_delayq_normal;   /* non-overflow queue      */
	struct sch_qprio_s *volatile p_delayq_overflow; /* overflow queue          */
	struct sch_qfifo_s q_ready[OSPORT_NUM_PRIOS];   /* run queues              */
	struct sch_qprio_s q_delay1;                    /* delay queue 1           */
	struct sch_qprio_s q_delay2;                    /* delay queue 2           */
	volatile uint_t timestamp;                      /* current time            */
	volatile uint_t lock_depth;						/* lock nesting counter    */
};

/*
 * Thread control block
 * Order of members makes a difference.
 */
struct thd_cblk_s
{
	void *volatile p_sp;              /* stack pointer snapshot */
	volatile thd_state_t state;       /* state                  */
	struct sch_qitem_s item_sch;      /* scheduling item        */
	struct sch_qitem_s item_delay;    /* delay item             */
	struct mlst_s mlst;				  /* memory list 			*/
	void *volatile p_stack;			  /* stack memory 		    */
	void *volatile p_schinfo;         /* scheduling info        */
};

/*
 * Scheduler functions
 */
UTIL_UNSAFE void sch_init( sch_cblk_t *p_sch );
UTIL_UNSAFE void sch_reschedule_req( sch_cblk_t *p_sch );
UTIL_UNSAFE void sch_unload_current( sch_cblk_t *p_sch );
UTIL_UNSAFE void sch_handle_heartbeat( sch_cblk_t *p_sch );
UTIL_UNSAFE void sch_insert_ready( sch_cblk_t *p_sch, sch_qitem_t *p_item );
UTIL_UNSAFE void sch_insert_delay( sch_cblk_t *p_sch, sch_qitem_t *p_item, uint_t timeout );

UTIL_SAFE void sch_lock_int( sch_cblk_t *p_sch );
UTIL_SAFE void sch_unlock_int( sch_cblk_t *p_sch );

/*
 * Thread functions
 */
UTIL_UNSAFE void thd_init( thd_cblk_t *p_thd, uint_t prio, void *p_stack,
		uint_t stack_size, void (*p_job)(void), void (*p_return)(void) );

UTIL_UNSAFE void thd_ready(thd_cblk_t *p_thd, sch_cblk_t *p_sch);
UTIL_UNSAFE void thd_block_current( sch_qprio_t *p_to, void *p_schinfo, uint_t timeout,
		sch_cblk_t *p_sch );

UTIL_UNSAFE void thd_suspend( thd_cblk_t *p_thd, sch_cblk_t *p_sch );
UTIL_UNSAFE void thd_change_prio( thd_cblk_t *p_thd, uint_t prio, sch_cblk_t *p_sch );

/*
 * Internal thread creation and deletion
 */
UTIL_UNSAFE void thd_create_static(thd_cblk_t *p_thd, uint_t prio, void *p_stack,
		uint_t stack_size, void (*p_job)(void), sch_cblk_t *p_sch);
UTIL_UNSAFE void thd_delete_static(thd_cblk_t *p_thd, sch_cblk_t *p_sch);

UTIL_SAFE void thd_return_hook_static( void );
UTIL_SAFE void thd_return_hook( void );

#endif /* H22A9CDB9_7059_460F_BAEE_7953ABC94E21 */
