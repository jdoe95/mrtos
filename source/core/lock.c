/******************************************************************************
 * Nested locking/unlocking module.
 *
 * This file is part of mRTOS.
 * SPDX-License-Identifier: MIT
 * Copyright (C) 2018 John Buyi Yu
 *****************************************************************************/
#include <arch/assert.h>
#include <arch/config.h>
#include <arch/preempt.h>
#include <core/lock.h>



/********************************************************************
 * Static variables
 ********************************************************************/
static os_uint volatile g_lock_depth;



/********************************************************************
 * Hidden from application
 ********************************************************************/
void
os_lock_init(void)
{
	g_lock_depth = 0U;
}



/********************************************************************
 * Visible to application
 ********************************************************************/
/*
 * Obtain the lock depth
 */
os_uint
os_lock_get_depth(void)
{
	/*
	 * One might wonder if the read is safe, because multiple instructions
	 * might be needed to read the variable, if the underlying type is
	 * non-atomic. Here is an explanation:
	 *
	 * The read is unsafe when it is interrupted by a write to the global
	 * lock depth. If this happens, the read must have started outside a
	 * critical section, where the global lock depth is 0U. The read is only
	 * allowed to resume after the critical section is exited, when the global
	 * lock depth has returned to 0U. In this case the write does not modify
	 * the value of the variable. The interrupted read always yields the same
	 * result as if it has not been interrupted, and thus can be considered
	 * safe.
	 */
	return g_lock_depth;
}

/*
 * Lock preempt with nesting
 */
void
os_lock(void)
{
	OS_DISABLE_PREEMPT();

	/*
	 * Catch the program when the lock depth holds an unlikely value.
	 */
	OS_ASSERT( g_lock_depth < OS_MAX_LOCK_DEPTH );

	g_lock_depth++;
}

/*
 * Unlock preempt with nesting
 */
void
os_unlock(void)
{
	/*
	 * If there is an unmatched lock/unlock pair, the lock depth can underflow
	 * or accumulate to a very large number. This assertion hopefully catches
	 * it.
	 */
	OS_ASSERT( g_lock_depth > 0U );
	OS_ASSERT( g_lock_depth < OS_MAX_LOCK_DEPTH );

	g_lock_depth--;

	/* unlocked */
	if(g_lock_depth == 0U)
	{
		OS_ENABLE_PREEMPT();
	}
}

