/** ************************************************************************
 * @file semaphore.c
 * @brief Semaphore
 * @author John Yu buyi.yu@wne.edu
 *
 * This file is part of mRTOS.
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
#include "../include/semaphore.h"
#include "../include/global.h"

/*
 * Initialize a semaphore
 */
UTIL_UNSAFE
void sem_init( sem_cblk_t *p_sem, uint_t initial )
{
	/*
	 * If failed:
	 * NULL pointer passed to p_sem
	 */
	UTIL_ASSERT( p_sem != NULL );

	p_sem->counter = initial;
	sch_q_init( &p_sem->q_wait );
}

/*
 * Initialize semaphore scheduling info
 */
UTIL_UNSAFE
void sem_schinfo_init( sem_schinfo_t *p_schinfo )
{
	/*
	 * If failed:
	 * NULL pointer passed to p_schinfo
	 */
	UTIL_ASSERT( p_schinfo != NULL );
	p_schinfo->result = false;
}

/*
 * Delete a static semaphore
 */
UTIL_UNSAFE
void sem_delete_static( sem_cblk_t *p_sem, sch_cblk_t *p_sch )
{
	sch_qitem_t *p_item;

	/*
	 * If failed:
	 * NULL pointer passed to p_sem or p_sch
	 */
	UTIL_ASSERT( p_sem != NULL );
	UTIL_ASSERT(p_sch != NULL);

	/* ready all waiting threads */
	while( p_sem->q_wait.p_head != NULL )
	{
		p_item = p_sem->q_wait.p_head;

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
 * @brief Create a semaphore
 * @param initial the initial value of a semaphore
 * @retval 0 semaphore creation failed because of low memory
 * @retval !0 handle to the created semaphore
 */
UTIL_SAFE
os_handle_t os_semaphore_create( os_uint_t initial )
{
	sem_cblk_t *p_sem;

	UTIL_LOCK_EVERYTHING();
	p_sem = mpool_alloc( sizeof(sem_cblk_t), &g_mpool, &g_mlst );

	if( p_sem != NULL )
	{
		sem_init(p_sem, initial);
	}

	UTIL_UNLOCK_EVERYTHING();

	return (os_handle_t) p_sem;
}

/**
 * @brief Delete a semaphore
 *
 */
UTIL_SAFE
void os_semaphore_delete( os_handle_t h_sem )
{
	sem_cblk_t *p_sem;

	p_sem = (sem_cblk_t*)h_sem;

	/*
	 * If failed:
	 * NULL pointer passed to p_sem
	 */
	UTIL_ASSERT( p_sem != NULL );

	UTIL_LOCK_EVERYTHING();
	sem_delete_static(p_sem, &g_sch);

	/* free memory */
	mpool_free( p_sem, &g_mpool );
	UTIL_UNLOCK_EVERYTHING();
}

/**
 * @brief Reset a semaphore to its initial value
 */
UTIL_SAFE
void os_semaphore_reset( os_handle_t h_sem, os_uint_t initial )
{
	sem_cblk_t *p_sem;
	thd_cblk_t *p_thd;

	p_sem = (sem_cblk_t*)h_sem;

	/*
	 * If failed:
	 * NULL pointer passed to p_sem
	 */
	UTIL_ASSERT( p_sem != NULL );

	UTIL_LOCK_EVERYTHING();

	while( (p_sem->q_wait.p_head != NULL) && (initial--) )
	{
		/*
		 * If failed
		 * Cannot obtain thread from item
		 */
		UTIL_ASSERT(p_sem->q_wait.p_head->p_thd != NULL );

		p_thd = p_sem->q_wait.p_head->p_thd;

		/*
		 * If failed:
		 * scheduling info missing
		 */
		UTIL_ASSERT( p_thd->p_schinfo != NULL );

		((sem_schinfo_t*)(p_thd->p_schinfo))->result = true;
		thd_ready( p_thd, &g_sch );
	}

	/* the value left after unblocking all waiting threads */
	p_sem->counter = initial;

	sch_reschedule_req(&g_sch);
	UTIL_UNLOCK_EVERYTHING();
}

/**
 * @brief Get the counter value of a semaphore
 */
UTIL_SAFE
os_uint_t os_semaphore_get_counter( os_handle_t h_sem )
{
	sem_cblk_t *p_sem;
	uint_t ret;

	p_sem = (sem_cblk_t*) h_sem;

	/*
	 * If failed:
	 * NULL pointer passed to p_sem
	 */
	UTIL_ASSERT( p_sem != NULL );

	UTIL_LOCK_EVERYTHING();
	ret = p_sem->counter;
	UTIL_UNLOCK_EVERYTHING();

	return ret;
}

/**
 * @brief Increase the counter value of a semaphore
 */
UTIL_SAFE
void os_semaphore_post( os_handle_t h_sem )
{
	sem_cblk_t *p_sem;
	thd_cblk_t *p_thd;

	p_sem = (sem_cblk_t*) h_sem;

	/*
	 * If failed:
	 * NULL pointer passed to p_sem
	 */
	UTIL_ASSERT( p_sem != NULL );

	UTIL_LOCK_EVERYTHING();
	/* check if any thread is waiting, transfer this value to the first
	 * highest priority thread.
	 * first != NULL means the list is not empty */
	if( p_sem->q_wait.p_head != NULL )
	{
		/*
		 * If failed:
		 * Cannot obtain thread from item
		 */
		UTIL_ASSERT( p_sem->q_wait.p_head->p_thd != NULL);

		p_thd = (thd_cblk_t*) p_sem->q_wait.p_head->p_thd;

		((sem_schinfo_t*)(p_thd->p_schinfo))->result = true;

		thd_ready( p_thd, &g_sch );

		sch_reschedule_req(&g_sch);
	}
	else
	{
		/* no other threads are blocking. */
		p_sem->counter++;
	}

	UTIL_UNLOCK_EVERYTHING();
}

/**
 * @brief Test if the counter of a semaphore can decrease
 */
os_bool_t os_semaphore_peek_wait( os_handle_t h_sem )
{
	sem_cblk_t *p_sem;
	bool_t ret;

	p_sem = (sem_cblk_t*) h_sem;

	/*
	 * If failed:
	 * NULL pointer passed to p_sem
	 */
	UTIL_ASSERT( p_sem != NULL );

	UTIL_LOCK_EVERYTHING();
	ret = p_sem->counter > 0;
	UTIL_UNLOCK_EVERYTHING();

	return ret;
}

/**
 * @brief Decrease the semaphore without blocking
 */
os_bool_t os_semaphore_wait_nonblocking( os_handle_t h_sem )
{
	sem_cblk_t *p_sem;
	bool_t ret = false;

	p_sem = (sem_cblk_t*) h_sem;

	/*
	 * If failed:
	 * NULL pointer passed to p_sem
	 */
	UTIL_ASSERT( p_sem != NULL );

	UTIL_LOCK_EVERYTHING();

	if( p_sem->counter != 0)
	{
		p_sem->counter--;
		ret = true;
	}

	UTIL_UNLOCK_EVERYTHING();

	return ret;
}

/**
 * @brief Decrease the semaphore, block if necessary
 */
os_bool_t os_semaphore_wait(os_handle_t h_sem, os_uint_t timeout)
{
	sem_cblk_t *p_sem;
	sem_schinfo_t schinfo;
	bool_t ret = false;

	p_sem = (sem_cblk_t*) h_sem;

	/*
	 * If failed:
	 * NULL pointer passed to p_sem
	 */
	UTIL_ASSERT(p_sem != NULL);

	UTIL_LOCK_EVERYTHING();

	if( p_sem->counter != 0 )
	{
		p_sem->counter--;
		ret = true;
	}
	else
	{
		sem_schinfo_init(&schinfo);
		thd_block_current_req( &p_sem->q_wait, &schinfo, timeout, &g_sch);
		ret = schinfo.result;
	}

	UTIL_UNLOCK_EVERYTHING();

	return ret;
}
