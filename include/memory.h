/** ************************************************************************
 * @file memory.h
 * @brief Dynamic memory pool
 * @author John Yu buyi.yu@wne.edu
 *
 * This file is part of mRTOS.
 *
 * This implementation uses the next fit algorithm.
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
#ifndef H2D331914_483F_4D1B_9049_BFF1D74B0B20
#define H2D331914_483F_4D1B_9049_BFF1D74B0B20

#include "util.h"

/*
 * Type declarations
 */
struct mblk_s;
struct mlst_s;
struct mpool_s;

typedef struct mblk_s mblk_t;
typedef struct mlst_s mlst_t;
typedef struct mpool_s mpool_t;

/*
 * Memory block header
 * Order of members makes a difference.
 */
struct mblk_s
{
	struct mblk_s *volatile p_prev; /* previous block */
	struct mblk_s *volatile p_next; /* next block     */
	volatile uint_t size;           /* block size     */
	struct mlst_s *volatile p_mlst; /* parent list    */
};

/*
 * Memory list header
 */
struct mlst_s
{
	struct mblk_s *volatile p_head; /* list head */
};

/*
 * Memory pool header
 */
struct mpool_s
{
	struct mblk_s *volatile p_head;       /* pool head       */
	struct mblk_s *volatile p_alloc_head; /* allocation head */
};

#ifdef __cplusplus
extern "C" {
#endif
/*
 * Initialization functions
 */
UTIL_UNSAFE void mblk_init(mblk_t *p_mblk, uint_t size);
UTIL_UNSAFE void mlst_init(mlst_t *p_mlst );
UTIL_UNSAFE void mpool_init(mpool_t *p_mpool);

/*
 * Memory list functions
 */
UTIL_UNSAFE void mlst_insert(mblk_t *p_mblk, mlst_t *p_mlst);
UTIL_UNSAFE void mlst_remove(mblk_t *p_mblk);

/*
 * Memory pool functions
 */
UTIL_UNSAFE void mpool_insert(mblk_t *p_mblk, mpool_t *p_mpool);
UTIL_UNSAFE void mpool_remove(mblk_t *p_mblk, mpool_t *p_mpool);
UTIL_UNSAFE void mpool_split(mblk_t *p_mblk, uint_t size, mpool_t *p_mpool);
UTIL_UNSAFE void mpool_merge(mblk_t *p_mblk, mpool_t *p_mpool);

/*
 * Memory allocation functions
 */
UTIL_UNSAFE void *mpool_alloc(uint_t size, mpool_t *p_mpool, mlst_t *p_mlst );
UTIL_UNSAFE void mpool_free(void *p, mpool_t *p_mpool);

#ifdef __cplusplus
}
#endif

/*
 * Diagnostics
 */
struct mblk_info_s;
struct mlst_info_s;
struct mpool_info_s;

typedef struct mblk_info_s mblk_info_t;
typedef struct mlst_info_s mlst_info_t;
typedef struct mpool_info_s mpool_info_t;

/*
 * Memory block information
 */
struct mblk_info_s
{
	uint_t size; /* size */
};

/*
 * Memory list information
 */
struct mlst_info_s
{
	uint_t size;  /* size in list             */
	uint_t count; /* number of blocks in list */
};

/*
 * Memory pool information
 */
struct mpool_info_s
{
	uint_t size;  /* size in pool             */
	uint_t count; /* number of blocks in pool */
};

#ifdef __cplusplus
extern "C" {
#endif

UTIL_UNSAFE void mblk_gather_info( const mblk_t *p_mblk, mblk_info_t *p_info);
UTIL_UNSAFE void mlst_gather_info( const mlst_t *p_mlst, mlst_info_t *p_info);
UTIL_UNSAFE void mpool_gather_info( const mpool_t *p_mpool, mpool_info_t *p_info);

#ifdef __cplusplus
}
#endif

#endif /* H2D331914_483F_4D1B_9049_BFF1D74B0B20 */

