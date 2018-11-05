/** ************************************************************************
 * @file util.c
 * @brief Utilities
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
#include "../include/util.h"
#include "../include/global.h"

/*
 * Disable interrupts nested
 */
void util_dint_nested( void )
{
	uint_t int_depth;

	/*
	 * If failed:
	 * Interrupt nested over 100 times, completely impossible.
	 * Underflow?
	 */
	UTIL_ASSERT( g_int_depth < 100 );

	int_depth = g_int_depth + 1;

	if( int_depth == 1 )
	{
		OSPORT_DISABLE_INT();
		g_int_depth = int_depth;
	}
}

/*
 * Enable interrupt nested
 */
void util_eint_nested( void )
{
	/*
	 * If failed:
	 * Trying to release a lock you do not own
	 * lock/unlock must be used in pairs
	 */
	UTIL_ASSERT( g_int_depth > 0 );

	/*
	 * If failed:
	 * Interrupt nested over 100 times, completely impossible.
	 * Underflow?
	 */
	UTIL_ASSERT( g_int_depth < 100 );

	g_int_depth--;

	if( g_int_depth == 0)
	{
		OSPORT_ENABLE_INT();
	}
}

