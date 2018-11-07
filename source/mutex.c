/** ************************************************************************
 * @file mutex.c
 * @brief Mutural exclusion
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
#include "../include/mutex.h"
#include "../include/thread.h"
#include "../include/global.h"

/*
 * Initialize a mutex
 */
UTIL_UNSAFE
void mutex_init( mutex_cblk_t *p_mutex )
{
	/*
	 * If failed:
	 * NULL pointer passed to p_mutex
	 */
	UTIL_ASSERT( p_mutex != NULL );

	p_mutex->lock_depth = 0;
	p_mutex->p_owner = NULL;
	sch_q_init( &p_mutex->q_wait );
}

/*
 * Initialize mutex scheduling info
 */
UTIL_UNSAFE
void mutex_schinfo_init( mutex_schinfo_t *p_schinfo )
{
	/*
	 * If failed:
	 * NULL pointer passed to p_schinfo
	 */
	UTIL_ASSERT( p_schinfo != NULL );

	p_schinfo->result = false;
}

/*
 * Delete a static mutex
 */
UTIL_UNSAFE
void mutex_delete_static( mutex_cblk_t *p_mutex, sch_cblk_t *p_sch )
{
	sch_qitem_t *p_item;

	/*
	 * If failed
	 * NULL pointer passed to p_mutex or p_sch
	 */
	UTIL_ASSERT( p_mutex != NULL );
	UTIL_ASSERT( p_sch != NULL );

	/* ready all waiting threads */
	while( p_mutex->q_wait.p_head != NULL )
	{
		p_item = p_mutex->q_wait.p_head;

		/*
		 * If failed:
		 * cannot obtain thread from item
		 */
		UTIL_ASSERT( p_item->p_thd != NULL );

		thd_ready( p_item->p_thd, p_sch );
	}

	sch_reschedule_req(&g_sch);
}

/**
 * @brief Creates a mutex
 */
UTIL_SAFE
os_handle_t os_mutex_create( void )
{
	mutex_cblk_t *p_mutex;

	UTIL_LOCK_EVERYTHING();
	p_mutex = mpool_alloc( sizeof(mutex_cblk_t), &g_mpool, &g_mlst );

	if( p_mutex != NULL )
	{
		mutex_init(p_mutex);
	}

	UTIL_UNLOCK_EVERYTHING();

	return (os_handle_t)p_mutex;
}

/**
 * @brief Deletes a mutex
 */
void os_mutex_delete( os_handle_t h_mutex )
{
	mutex_cblk_t *p_mutex;

	p_mutex = (mutex_cblk_t*)h_mutex;

	/*
	 * If failed:
	 * NULL pointer passed to p_mutex
	 */
	UTIL_ASSERT( p_mutex != NULL );

	UTIL_LOCK_EVERYTHING();
	mutex_delete_static( p_mutex, &g_sch );

	/* free memory */
	mpool_free( p_mutex, &g_mpool);
	UTIL_UNLOCK_EVERYTHING();
}

os_bool_t os_mutex_peek_lock(os_handle_t h_mutex)
{
	mutex_cblk_t *p_mutex;
	bool_t ret;

	p_mutex = (mutex_cblk_t*)h_mutex;

	/*
	 * If failed:
	 * NULL pointer passed to p_mutex
	 */
	UTIL_ASSERT( p_mutex != NULL );

	UTIL_LOCK_EVERYTHING();
	ret = (p_mutex->p_owner == g_sch.p_current) || (p_mutex->lock_depth == 0);
	UTIL_UNLOCK_EVERYTHING();

	return ret;
}

os_bool_t os_mutex_is_locked(os_handle_t h_mutex)
{
	mutex_cblk_t *p_mutex;
	bool_t ret;

	p_mutex = (mutex_cblk_t*)h_mutex;

	/*
	 * If failed:
	 * NULL pointer passed to p_mutex
	 */
	UTIL_ASSERT( p_mutex != NULL );
	UTIL_LOCK_EVERYTHING();
	ret = p_mutex->lock_depth > 0;
	UTIL_UNLOCK_EVERYTHING();

	return ret;
}

os_bool_t os_mutex_lock_nonblocking(os_handle_t h_mutex)
{
	mutex_cblk_t *p_mutex;
	bool_t ret = false;

	p_mutex = (mutex_cblk_t*)h_mutex;

	/*
	 * If failed:
	 * NULL pointer passed to p_mutex
	 */
	UTIL_ASSERT( p_mutex != NULL );
	UTIL_LOCK_EVERYTHING();

	if( (p_mutex->lock_depth == 0) ||
			(p_mutex->p_owner == g_sch.p_current ) )
	{
		p_mutex->p_owner = g_sch.p_current;
		p_mutex->lock_depth++;

		ret = true;
	}

	UTIL_UNLOCK_EVERYTHING();

	return ret;
}

os_bool_t os_mutex_lock(os_handle_t h_mutex, os_uint_t timeout)
{
	bool_t ret;
	mutex_schinfo_t schinfo;
	mutex_cblk_t *p_mutex;

	p_mutex = (mutex_cblk_t*)h_mutex;

	/*
	 * If failed:
	 * NULL pointer passed to p_mutex
	 */
	UTIL_ASSERT( p_mutex != NULL );
	UTIL_LOCK_EVERYTHING();

	if( (p_mutex->lock_depth == 0) ||
			(p_mutex->p_owner == g_sch.p_current ) )
	{
		p_mutex->p_owner = g_sch.p_current;
		p_mutex->lock_depth++;

		ret = true;
	}
	else
	{
		mutex_schinfo_init(&schinfo);
		thd_block_current( &p_mutex->q_wait, &schinfo, timeout, &g_sch);
		ret = schinfo.result;
	}

	return ret;
}

void os_mutex_unlock(os_handle_t h_mutex)
{
	mutex_cblk_t *p_mutex;
	thd_cblk_t *p_thd;
	p_mutex = (mutex_cblk_t*)h_mutex;

	/*
	 * If failed:
	 * NULL pointer passed to p_mutex
	 */
	UTIL_ASSERT( p_mutex != NULL );

	UTIL_LOCK_EVERYTHING();

	/* only unlocks if current thread owns the mutex */
	if( p_mutex->p_owner == g_sch.p_current )
	{
		if( p_mutex->lock_depth > 1 )
		{
			p_mutex->lock_depth--;
		}
		else if( p_mutex->lock_depth == 1)
		{
			/* ready waiting threads */
			if( p_mutex->q_wait.p_head != NULL )
			{
				/*
				 * If failed:
				 * cannot obtain thread
				 */
				UTIL_ASSERT( p_mutex->q_wait.p_head->p_thd != NULL );

				p_thd = p_mutex->q_wait.p_head->p_thd;

				p_mutex->p_owner = p_thd;
				((mutex_schinfo_t*)(p_thd->p_schinfo))->result = true;

				thd_ready(p_thd, &g_sch);
				sch_reschedule_req( &g_sch );

			}
			else
			{
				/* unlock mutex */
				p_mutex->lock_depth = 0;
				p_mutex->p_owner = NULL;
			}
		}
		else
		{
			/* do nothing */
		}
	}
	UTIL_UNLOCK_EVERYTHING();
}





