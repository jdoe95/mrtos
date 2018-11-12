/** ************************************************************************
 * @file queue.h
 * @brief Queue
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
#ifndef H37FD4A18_A407_4E67_B4AF_775AD0BA0B71
#define H37FD4A18_A407_4E67_B4AF_775AD0BA0B71

#include "util.h"
#include "thread.h"

/*
 * Type declarations
 */
struct queue_cblk_s;
struct queue_schinfo_read_s;
struct queue_schinfo_write_s;

typedef struct queue_cblk_s queue_cblk_t;
typedef struct queue_schinfo_read_s queue_schinfo_read_t;
typedef struct queue_schinfo_write_s queue_schinfo_write_t;

/*
 * Queue control block
 */
struct queue_cblk_s
{
	byte_t *volatile p_buffer;       /* queue memory buffer */
	struct sch_qprio_s q_wait_read;  /* reading wait queue  */
	struct sch_qprio_s q_wait_write; /* writing wait queue  */
	volatile uint_t size;            /* size of buffer      */
	volatile uint_t read;            /* read index          */
	volatile uint_t write;           /* write index         */
};

/*
 * Queue write wait flag
 */
typedef enum
{
	QUEUE_WRITE_AHEAD = (1<<0)
} queue_write_wait_flag_t;

/*
 * Queue read wait flag
 */
typedef enum
{
	QUEUE_READ_PEEK = (1<<0)
} queue_read_wait_flag_t;

/*
 * Queue scheduling info
 */
struct queue_schinfo_read_s
{
	volatile bool_t result;               /* wait result */
	volatile uint_t size;                 /* read size   */
	byte_t *volatile p_data;              /* data        */
	volatile queue_read_wait_flag_t flag; /* flag        */
};

/*
 * Queue schduling info
 */
struct queue_schinfo_write_s
{
	volatile bool_t result;                /* wait result */
	volatile uint_t size;                  /* read size   */
	const byte_t *volatile p_data;         /* data        */
	volatile queue_write_wait_flag_t flag; /* flag        */
};

/*
 * Initialization functions
 */
UTIL_UNSAFE void queue_init( queue_cblk_t *p_q, void *p_buffer, uint_t size );
UTIL_UNSAFE void queue_schinfo_read_init( queue_schinfo_read_t *p_schinfo, uint_t flag );
UTIL_UNSAFE void queue_schnifo_write_init( queue_schinfo_write_t *p_schinfo, uint_t flag );
UTIL_UNSAFE void queue_delete_static( queue_cblk_t *p_q, sch_cblk_t *p_sch );
UTIL_UNSAFE void queue_write( queue_cblk_t *p_q, const byte_t *p_data, uint_t size );
UTIL_UNSAFE void queue_write_ahead( queue_cblk_t *p_q, const byte_t *p_data, uint_t size );
UTIL_UNSAFE void queue_read( queue_cblk_t *p_q, byte_t *p_data, uint_t size );
UTIL_UNSAFE void queue_peek( const queue_cblk_t *p_q, byte_t *p_data, uint_t size );
UTIL_UNSAFE uint_t queue_get_used_size(const queue_cblk_t *p_q );
UTIL_UNSAFE uint_t queue_get_free_size(const queue_cblk_t *p_q );
UTIL_UNSAFE void queue_unlock_threads( queue_cblk_t *p_q, sch_cblk_t *p_sch );

#endif /* H37FD4A18_A407_4E67_B4AF_775AD0BA0B71 */
