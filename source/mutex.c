/** ************************************************************************
 * @file mutex.c
 * @brief Mutural exclusion
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
#include "../include/mutex.h"
#include "../include/thread.h"
#include "../include/global.h"
#include "../include/api.h"

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
void mutex_schinfo_init( mutex_schinfo_t *p_schinfo, uint_t wait_flag )
{
	/*
	 * If failed:
	 * NULL pointer passed to p_schinfo
	 */
	UTIL_ASSERT( p_schinfo != NULL );

	p_schinfo->result = false;
	p_schinfo->wait_flag = wait_flag;
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

	sch_reschedule_req(p_sch);
}

/**
 * @brief Creates a mutex
 * @retval !0 handle to the created mutex
 * @retval 0 creation failed because of low memory
 * @note This function is thread safe and can be used in an interrupt
 * context or a thread context.
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
 * @param h_mutex handle to a mutex
 * @details Sleeping threads will be readied and the wait will
 * fail.
 * @note This function is thread safe and can be used in an interrupt
 * or a thread context.
 */
UTIL_SAFE
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

/**
 * @brief Locks a mutex, sleep if necessary
 * @param h_mutex handle to a mutex
 * @param timeout sleep timeout
 * @retval true operation successful
 * @retval false timeout
 * @note This function is thread safe and can only be used in a
 * thread context
 */
UTIL_SAFE
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
		mutex_schinfo_init(&schinfo, 0);
		thd_block_current( &p_mutex->q_wait, &schinfo, timeout, &g_sch);
		ret = schinfo.result;
	}
	UTIL_UNLOCK_EVERYTHING();

	return ret;
}

/**
 * @brief Unlocks a mutex
 * @param h_mutex handle to a mutex
 * @details Only unlocks the mutex when current thread owns
 * the mutex
 * @note This function is thread safe and can only be used in
 * a thread context.
 */
UTIL_SAFE
void os_mutex_unlock(os_handle_t h_mutex)
{
	mutex_cblk_t *p_mutex;
	thd_cblk_t *p_thd;
	mutex_schinfo_t *p_schinfo;

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
			p_mutex->lock_depth--;

		else if( p_mutex->lock_depth == 1)
		{
			/* ready waiting threads */
			for( ; ; )
			{
				if( p_mutex->q_wait.p_head != NULL )
				{
					/*
					 * If failed:
					 * cannot obtain thread
					 */
					UTIL_ASSERT( p_mutex->q_wait.p_head->p_thd != NULL );
					p_thd = p_mutex->q_wait.p_head->p_thd;

					/*
					 * If failed:
					 * Cannot obtain schinfo
					 */
					UTIL_ASSERT( p_thd->p_schinfo != NULL );
					p_schinfo = p_thd->p_schinfo;

					p_schinfo->result = true;
					thd_ready(p_thd, &g_sch);
					sch_reschedule_req( &g_sch );

					if( p_schinfo->wait_flag & MUTEX_PEEK )
					{
						/* peeking thread, do nothing */
						continue;
					}
					else
					{
						p_mutex->p_owner = p_thd;
						break;
					}
				}
				else
				{
					/* unlock mutex */
					p_mutex->lock_depth = 0;
					p_mutex->p_owner = NULL;
					break;
				}
			}
		}
		else
		{
			/* already unlocked, do nothing */
		}
	}
	UTIL_UNLOCK_EVERYTHING();
}

/**
 * @brief Try locking a mutex, sleep if necessary
 * @param h_mutex handle to a mutex
 * @param timeout sleep timeout
 * @retval true operation successful
 * @retval false timeout
 * @brief Similar to @ref os_mutex_lock, but does not lock mutex.
 * @note This function is thread safe and can only be used in a
 * thread context
 */
UTIL_SAFE
os_bool_t os_mutex_peek(os_handle_t h_mutex, os_uint_t timeout)
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
		ret = true;
	}
	else
	{
		mutex_schinfo_init(&schinfo, MUTEX_PEEK);
		thd_block_current( &p_mutex->q_wait, &schinfo, timeout, &g_sch);
		ret = schinfo.result;
	}
	UTIL_UNLOCK_EVERYTHING();

	return ret;
}

/**
 * @brief Try locking a mutex, non-blocking
 * @param h_mutex handle to a mutex
 * @retval true operation successful
 * @retval false operation failed
 * @note This function is thread safe and can only be used in a thread
 * context.
 */
UTIL_SAFE
os_bool_t os_mutex_peek_nb(os_handle_t h_mutex)
{
	bool_t ret = false;
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
		ret = true;
	}
	UTIL_UNLOCK_EVERYTHING();

	return ret;
}

/**
 * @brief Checks if the mutex is locked
 * @param h_mutex handle to a mutex
 * @retval true mutex is locked
 * @retval false mutex is not locked
 * @note This function is thread safe and can be used in an
 * interrupt or a thread context.
 */
UTIL_SAFE
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

/**
 * @brief Locks a mutex, non-blocking
 * @param h_mutex handle to a mutex
 * @retval true operation successful
 * @retval false operation failed
 * @note This function is thread safe and can only
 * be used in a thread context.
 */
UTIL_SAFE
os_bool_t os_mutex_lock_nb(os_handle_t h_mutex)
{
	bool_t ret = false;
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
	UTIL_UNLOCK_EVERYTHING();

	return ret;
}

