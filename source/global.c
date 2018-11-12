/** ************************************************************************
 * @file global.c
 * @brief Global variables
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
#include "../include/global.h"
#include "../include/api.h"

/*
 * Global memory pool
 */
mpool_t g_mpool;

/*
 * Global memory list
 */
mlst_t g_mlst;

/*
 * Scheduler control block
 */
sch_cblk_t g_sch;

/*
 * Idle thread
 */
static thd_cblk_t thd_idle;
static byte_t thd_idle_stack[OSPORT_IDLE_STACK_SIZE];

/**
 * @brief Handle heartbeat
 * @details This function should be called everytime the
 * heartbeat counter triggers.
 * @note This function is thread safe, and can be used in
 * thread or interrupt context.
 */
UTIL_SAFE
void os_handle_heartbeat( void )
{
	UTIL_LOCK_EVERYTHING();
	sch_handle_heartbeat( &g_sch );
	UTIL_UNLOCK_EVERYTHING();
}

/**
 * @brief Initializes the operating system
 * @param p_config pointer to configuration
 * @note This function is not thread safe. It should
 * be used before calling any other OS functions.
 */
UTIL_UNSAFE
void os_init( const os_config_t *p_config )
{
	/*
	 * If failed:
	 * Invalid parameter
	 */
	UTIL_ASSERT(p_config != NULL);

	/* initialize global variables */
	mpool_init(&g_mpool);
	mlst_init(&g_mlst);
	sch_init(&g_sch);

	/* create pool memory */
	mblk_init( (mblk_t*)p_config->p_pool_mem, p_config->pool_size );
	mpool_insert( (mblk_t*)p_config->p_pool_mem, &g_mpool);

	/* initialize idle thread */
	thd_init( &thd_idle, (OSPORT_NUM_PRIOS-1), thd_idle_stack,
			OSPORT_IDLE_STACK_SIZE, OSPORT_IDLE_FUNC, OSPORT_IDLE_FUNC);

	/* install idle thread */
	thd_ready(&thd_idle, &g_sch);
}

/**
 * @brief Returns current OS time in ticks
 * @return current OS time in ticks, might overflow
 * @note This function is thread safe, and can be used
 * in thread or interrupt context.
 */
UTIL_SAFE
os_uint_t os_get_time( void )
{
	os_uint_t ret;

	UTIL_LOCK_EVERYTHING();
	ret = g_sch.timestamp;
	UTIL_UNLOCK_EVERYTHING();

	return ret;
}

/**
 * @brief Enters a critical section
 * @details Use this function when a thread wants
 * to prevent the context being preempted by an interrupt
 * or other threads. Must be used in pairs with @ref
 * os_exit_critical. The critical section won't prevent
 * a thread from falling asleep. When a thread sleeps
 * in a critical section, the critical context will be
 * temporarily broken and restored when the thread wakes
 * up.
 * @note This function is thread safe, and can be
 * used in a thread or interrupt context.
 */
UTIL_SAFE
void os_enter_critical( void )
{
	UTIL_LOCK_EVERYTHING();
}

/**
 * @brief Exits a critical section
 * @details Must be used in pairs with @ref os_enter_critical.
 * @note This function is thread safe, and can be used
 * in a thread or interrupt context.
 */
UTIL_SAFE
void os_exit_critical( void )
{
	UTIL_UNLOCK_EVERYTHING();
}

/**
 * @brief Start the kernel
 * @details Call this function to load the first
 * thread onto the CPU. This function will never
 * return.
 */
UTIL_SAFE
void os_start( void )
{
	UTIL_LOCK_EVERYTHING();
	sch_set_next_thread(&g_sch);
	g_sch.p_current = g_sch.p_next;
	UTIL_UNLOCK_EVERYTHING();

	/* call portable start function to start kernel */
	OSPORT_START();
}

