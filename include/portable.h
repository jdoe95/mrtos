/** ************************************************************************
 * @file portable.h
 * @brief Platform dependent function wrapper
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
#ifndef HE34C7678_2774_4CD4_90B8_DE1E8329ECDA
#define HE34C7678_2774_4CD4_90B8_DE1E8329ECDA

#include "../../mrtos-portable/portable.h"

#if !defined(OSPORT_BYTE_T) || \
	!defined(OSPORT_UINT_T) || \
	!defined(OSPORT_UINTPTR_T) || \
	!defined(OSPORT_BOOL_T)
#	error "One or more data types missing."
#endif

#if !defined(OSPORT_NUM_PRIOS) || \
	!defined(OSPORT_MEM_ALIGN) || \
	!defined(OSPORT_MEM_SMALLEST) || \
	!defined(OSPORT_IDLE_STACK_SIZE)
#	error "One or more configurations missing."
#endif

#if !defined(OSPORT_IDLE_FUNC) || \
	!defined(OSPORT_INIT_STACK) || \
	!defined(OSPORT_DISABLE_INT) || \
	!defined(OSPORT_ENABLE_INT) || \
	!defined(OSPORT_CONTEXTSW_REQ) || \
	!defined(OSPORT_START)
#	error "One or more functions missing."
#endif

#if !defined(OSPORT_ENABLE_DEBUG)
#	define OSPORT_ENABLE_DEBUG (0)
#else
#	if !defined(OSPORT_BREAKPOINT)
#		define OSPORT_BREAKPOINT() \
				for( ; ; )
#	endif
#endif

typedef OSPORT_BYTE_T os_byte_t;
typedef OSPORT_UINT_T os_uint_t;
typedef OSPORT_UINTPTR_T os_handle_t;
typedef OSPORT_BOOL_T os_bool_t;

#endif /* HE34C7678_2774_4CD4_90B8_DE1E8329ECDA */
