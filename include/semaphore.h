/** ************************************************************************
 * @file semaphore.h
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
#ifndef H822CB423_CB23_4742_8D53_7A478AB85C15
#define H822CB423_CB23_4742_8D53_7A478AB85C15

#include "util.h"
#include "thread.h"

/*
 * Type declarations
 */
struct sem_cblk_s;
struct sem_schinfo_s;

typedef struct sem_cblk_s sem_cblk_t;
typedef struct sem_schinfo_s sem_schinfo_t;

/*
 * Semaphore control block
 */
struct sem_cblk_s
{
	volatile uint_t counter; /* semaphore counter */
	struct sch_qprio_s q_wait; /* waiting threads */
};

/*
 * Semaphore scheduling info
 */
struct sem_schinfo_s
{
	bool_t result; /* wait result */
};

/*
 * Initialization functions
 */
UTIL_UNSAFE void sem_init( sem_cblk_t *p_sem, uint_t initial );
UTIL_UNSAFE void sem_schinfo_init( sem_schinfo_t *p_schinfo );

UTIL_UNSAFE void sem_delete_static( sem_cblk_t *p_sem, sch_cblk_t *p_sch );

#endif /* H822CB423_CB23_4742_8D53_7A478AB85C15 */
