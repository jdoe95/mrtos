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
 * Interrupt disable nesting counter
 */
volatile uint_t g_int_depth;

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
 * Idle thread control block
 */
thd_cblk_t g_thd_idle;
byte_t thd_idle_stack[OSPORT_IDLE_STACK_SIZE];

/*
 * handle heartbeat
 */
UTIL_SAFE
void g_handle_heartbeat( void )
{
	UTIL_LOCK_EVERYTHING();
	sch_handle_heartbeat(&g_sch);
	UTIL_UNLOCK_EVERYTHING();
}

/**
 * @brief Initializes the operating system
 * @note This function must be called with preemption disabled
 */
UTIL_UNSAFE
void os_init( const os_config_t *p_config )
{
	mblk_t *p_mblk;
	/*
	 * If failed:
	 * Invalid parameter
	 */
	UTIL_ASSERT(p_config != NULL);

	/* initialize global variables */
	g_int_depth = 0;
	mpool_init(&g_mpool);
	mlst_init(&g_mlst);
	sch_init(&g_sch);

	/* create pool memory */
	p_mblk = (mblk_t*)(p_config->p_pool_mem);
	mblk_init( p_mblk, p_config->pool_size );
	mpool_insert( p_mblk, &g_mpool);

	/* initialize idle thread */
	thd_init( &g_thd_idle, (OSPORT_NUM_PRIOS-1), thd_idle_stack,
			OSPORT_IDLE_STACK_SIZE, OSPORT_IDLE_FUNC, OSPORT_IDLE_FUNC);

	/* install idle thread */
	thd_ready(&g_thd_idle, &g_sch);
}
