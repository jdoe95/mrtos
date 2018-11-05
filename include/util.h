/** ************************************************************************
 * @file util.h
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
#ifndef H7D9C202C_17FC_4683_B032_AC3425ABC5EC
#define H7D9C202C_17FC_4683_B032_AC3425ABC5EC

#include "portable.h"

#if OSPORT_ENABLE_DEBUG
#	define UTIL_ASSERT(cond) \
		if(!(cond)) \
			OSPORT_BREAKPOINT();
#else
#	define UTIL_ASSERT(cond)
#endif

#define UTIL_LOCK_EVERYTHING() \
	util_dint_nested()

#define UTIL_UNLOCK_EVERYTHING() \
	util_eint_nested()

#define UTIL_UNSAFE	/* unsafe in a preemptive context */
#define UTIL_SAFE	/* safe in a preemptive context */

typedef os_byte_t byte_t;
typedef os_uint_t uint_t;
typedef os_handle_t handle_t;
typedef os_bool_t bool_t;

void util_dint_nested( void );
void util_eint_nested( void );

#endif /* H7D9C202C_17FC_4683_B032_AC3425ABC5EC */
