/** ************************************************************************
 * @file memory.c
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
#include "../include/memory.h"
#include "../include/list.h"
#include "../include/global.h"

/*
 * Check the alignment of a size or address
 */
#define MPOOL_IS_ALIGNED(VAL)  \
	(((handle_t)(VAL) % OSPORT_MEM_ALIGN) == 0)

/*
 * Align a memory block size
 */
#define MPOOL_ALIGN(VAL) \
	(((VAL)%OSPORT_MEM_ALIGN)? \
		((VAL)+OSPORT_MEM_ALIGN-(VAL)%OSPORT_MEM_ALIGN):(VAL))

/*
 * Aligned memory block header size
 */
#define MBLK_HEADER_SIZE \
	MPOOL_ALIGN(sizeof(mblk_t))

/*
 * Aligned smallest memory block size
 */
#define MBLK_SMALLEST_SIZE \
	MPOOL_ALIGN(MBLK_HEADER_SIZE + OSPORT_MEM_SMALLEST)

/*
 * Convert memory block to base class lstitem_t
 */
#define TO_LSTITEM(P_MBLK) \
	((lstitem_t*)(P_MBLK))

/*
 * Initialize a memory block header
 */
UTIL_UNSAFE
void mblk_init( mblk_t *p_mblk, uint_t size)
{
	/*
	 * If failed:
	 * NULL pointer passed to p_mblk
	 */
	UTIL_ASSERT( p_mblk != NULL );

	/*
	 * If failed:
	 * Address not aligned
	 */
	UTIL_ASSERT( MPOOL_IS_ALIGNED(p_mblk) );

	/*
	 * If failed:
	 * size not aligned
	 */
	UTIL_ASSERT( MPOOL_IS_ALIGNED(size) );

	/*
	 * If failed:
	 * Memory block too small
	 */
	UTIL_ASSERT( size >= MBLK_SMALLEST_SIZE );

	lstitem_init(TO_LSTITEM(p_mblk));
	p_mblk->size = size;
	p_mblk->p_mlst = NULL;
}

/*
 * Initialize a memory list header
 */
UTIL_UNSAFE
void mlst_init( mlst_t *p_mlst )
{
	/*
	 * If failed:
	 * NULL pointer passed to p_mlst
	 */
	UTIL_ASSERT( p_mlst != NULL );

	p_mlst->p_head = NULL;
}

/*
 * Initialize a memory pool header
 */
UTIL_UNSAFE
void mpool_init( mpool_t *p_mpool )
{
	/*
	 * If failed:
	 * NULL pointer passed to p_mpool
	 */
	UTIL_ASSERT( p_mpool != NULL );

	p_mpool->p_head = NULL;
	p_mpool->p_alloc_head = NULL;
}


/*
 * Insert memory block into memory list
 */
UTIL_UNSAFE
void mlst_insert( mblk_t *p_mblk, mlst_t *p_mlst )
{
	/*
	 * If failed:
	 * NULL pointer passed to p_mblk or p_mlst
	 */
	UTIL_ASSERT( p_mblk != NULL );
	UTIL_ASSERT( p_mlst != NULL );

	/*
	 * If failed:
	 * Block corrupted, uninitialized, or already in list
	 */
	UTIL_ASSERT( p_mblk->p_mlst == NULL );

	p_mblk->p_mlst = p_mlst;

	/* inserting first block */
	if( p_mlst->p_head == NULL )
	{
		p_mlst->p_head = p_mblk;
	}
	else
	{
		/* insert as last block */
		lstitem_prepend(TO_LSTITEM(p_mblk), TO_LSTITEM(p_mlst->p_head));
	}
}

/*
 * Remove memory block from memory list
 */
UTIL_UNSAFE
void mlst_remove( mblk_t *p_mblk )
{
	mlst_t *p_mlst;

	/*
	 * If failed:
	 * NULL pointer passed to p_mblk or p_mlst
	 */
	UTIL_ASSERT( p_mblk != NULL );

	/*
	 * If failed:
	 * Block not in list
	 */
	UTIL_ASSERT( p_mblk->p_mlst != NULL );

	p_mlst = p_mblk->p_mlst;
	p_mblk->p_mlst = NULL;

	/* removing only block */
	if( p_mblk->p_next == p_mblk )
	{
		/*
		 * If failed:
		 * Memory block claims to be the only item in list,
		 * but memory list header indicates otherwise.
		 * Corrupted or uninitialized list.
		 */
		UTIL_ASSERT( p_mblk->p_prev == p_mblk );
		UTIL_ASSERT( p_mblk == p_mlst->p_head );

		p_mlst->p_head = NULL;
	}

	/* removing head block */
	else if( p_mblk == p_mlst->p_head )
	{
		/*
		 * If failed:
		 * Broken link
		 */
		UTIL_ASSERT( p_mlst->p_head->p_next != NULL );

		p_mlst->p_head = p_mlst->p_head->p_next;
		lstitem_remove(TO_LSTITEM(p_mblk));
	}
	else
	{
		lstitem_remove(TO_LSTITEM(p_mblk));
	}
}

/*
 * Insert memory block into memory pool
 */
UTIL_UNSAFE
void mpool_insert( mblk_t *p_mblk, mpool_t *p_mpool )
{
	mblk_t *p_i;

	/*
	 * If failed:
	 * NULL pointer passed to p_mblk or p_mpool
	 */
	UTIL_ASSERT( p_mblk != NULL );
	UTIL_ASSERT( p_mpool != NULL );

	/*
	 * If failed:
	 * Block not removed from list
	 */
	UTIL_ASSERT( p_mblk->p_mlst == NULL );

	/* inserting first block */
	if( p_mpool->p_head == NULL )
	{
		/*
		 * If failed:
		 * p_head indicates pool is empty but
		 * p_alloc_head indicates otherwise.
		 * Corrupted or uninitialized pool.
		 */
		UTIL_ASSERT( p_mpool->p_alloc_head == NULL );

		p_mpool->p_head = p_mblk;
		p_mpool->p_alloc_head = p_mblk;
	}
	else if( p_mblk < p_mpool->p_head )
	{
		/* insert as first block */
		lstitem_prepend(TO_LSTITEM(p_mblk), TO_LSTITEM(p_mpool->p_head));
		p_mpool->p_head = p_mblk;
	}
	else if( p_mblk > p_mpool->p_head->p_prev )
	{
		/* insert as last block */
		lstitem_prepend(TO_LSTITEM(p_mblk), TO_LSTITEM(p_mpool->p_head));
	}
	else
	{
		/* search starts from second item */
		p_i = p_mpool->p_head->p_next;

		do
		{
			/*
			 * If failed:
			 * Broken link
			 */
			UTIL_ASSERT( p_i != NULL );
			UTIL_ASSERT( p_i->p_next != NULL );
			UTIL_ASSERT( p_i->p_next->p_prev == p_i);

			/* find block with larger start address */
			if( p_mblk < p_i )
			{
				/* insert before found block */
				lstitem_prepend( TO_LSTITEM(p_mblk), TO_LSTITEM(p_i));
				break;
			}

			p_i = p_i->p_next;

		} while(true);
	}
}

/*
 * Remove memory block from memory pool
 */
UTIL_UNSAFE
void mpool_remove( mblk_t *p_mblk, mpool_t *p_mpool )
{
	/*
	 * If failed:
	 * NULL pointer passed to p_mblk or p_mpool
	 */
	UTIL_ASSERT( p_mblk != NULL );
	UTIL_ASSERT( p_mpool != NULL );

	/*
	 * If failed:
	 * Removing from pool, but block seems to be in list
	 */
	UTIL_ASSERT( p_mblk->p_mlst == NULL );

	/* removing only item */
	if( p_mblk == p_mblk->p_next )
	{
		/*
		 * If failed:
		 * Memory block claims to be the only item in pool,
		 * but memory pool header indicates otherwise.
		 * Corrupted or uninitialized pool.
		 */
		UTIL_ASSERT( p_mblk->p_prev == p_mblk );
		UTIL_ASSERT( p_mblk == p_mpool->p_head );
		UTIL_ASSERT( p_mblk == p_mpool->p_alloc_head );

		p_mpool->p_head = NULL;
		p_mpool->p_alloc_head = NULL;
	}
	else
	{
		/* removing first item */
		if (p_mblk == p_mpool->p_head )
		{
			/*
			 * If failed:
			 * Broken link
			 */
			UTIL_ASSERT( p_mpool->p_head->p_next != NULL );
			p_mpool->p_head = p_mpool->p_head->p_next;
		}

		/* removing current item */
		if (p_mblk == p_mpool->p_alloc_head )
		{
			/*
			 * If failed:
			 * Broken link
			 */
			UTIL_ASSERT( p_mpool->p_alloc_head->p_next != NULL );
			p_mpool->p_alloc_head = p_mpool->p_alloc_head->p_next;
		}

		lstitem_remove(TO_LSTITEM(p_mblk));
	}
}

/*
 * Split memory block in memory pool
 */
UTIL_UNSAFE
void mpool_split( mblk_t *p_mblk, uint_t size, mpool_t *p_mpool )
{
	mblk_t *p_mblk_new;

	/*
	 * If failed:
	 * NULL pointer passed to p_mblk or p_mpool
	 */
	UTIL_ASSERT( p_mblk != NULL );
	UTIL_ASSERT( p_mpool != NULL );

	/*
	 * If failed:
	 * Requested size not aligned
	 */
	UTIL_ASSERT( MPOOL_IS_ALIGNED(size) );

	/*
	 * If failed:
	 * Requested size too small
	 */
	UTIL_ASSERT( size >= MBLK_SMALLEST_SIZE );

	/*
	 * If failed:
	 * Memory block too small
	 */
	UTIL_ASSERT( p_mblk->size >= size + MBLK_SMALLEST_SIZE );

	p_mblk_new = (mblk_t*)( (os_byte_t*)p_mblk + size );

	mblk_init( p_mblk_new, p_mblk->size - size );
	p_mblk->size = size;

	mpool_insert( p_mblk_new, p_mpool );
}

/*
 * Merge memory blocks in memory pool
 */
UTIL_UNSAFE
void mpool_merge( mblk_t *p_mblk, mpool_t *p_mpool )
{
	/*
	 * If failed:
	 * NULL pointer passed to p_mblk or p_mpool
	 */
	UTIL_ASSERT( p_mblk != NULL );
	UTIL_ASSERT( p_mpool != NULL );

	/*
	 * If failed:
	 * Broken link
	 */
	UTIL_ASSERT( p_mblk->p_next->p_prev == p_mblk );
	UTIL_ASSERT( p_mblk->p_prev->p_next == p_mblk );

	/* merge with next block */
	if( (os_byte_t*)p_mblk + p_mblk->size == (os_byte_t*)p_mblk->p_next  )
	{
		p_mblk->size += p_mblk->p_next->size;
		mpool_remove( p_mblk->p_next, p_mpool );
	}

	/* merge with previous block */
	if( (os_byte_t*)p_mblk == (os_byte_t*) p_mblk->p_prev + p_mblk->p_prev->size )
	{
		p_mblk->p_prev->size += p_mblk->size;
		mpool_remove( p_mblk, p_mpool );
	}
}

/*
 * Allocate memory from memory pool
 */
UTIL_UNSAFE
void *mpool_alloc( uint_t size, mpool_t *p_mpool, mlst_t *p_mlst )
{
	mblk_t *p_i;
	void *p_ret;

	/*
	 * If failed:
	 * NULL pointer passed to p_mpool or p_mlst
	 */
	UTIL_ASSERT( p_mpool != NULL );
	UTIL_ASSERT( p_mlst != NULL );

	p_ret = NULL;

	/* skip if memory pool is empty */
	if( p_mpool->p_head != NULL )
	{
		/*
		 * If failed:
		 * p_head indicates the pool is not empty, but
		 * p_alloc_head indicates otherwise.
		 * Corrupted or uninitialized pool.
		 */
		UTIL_ASSERT( p_mpool->p_alloc_head != NULL );

		/* calculate and align the block size */
		size = MPOOL_ALIGN(size + MBLK_HEADER_SIZE);
		if( size < MBLK_SMALLEST_SIZE )
			size = MBLK_SMALLEST_SIZE;

		/* start searching from current block */
		p_i = p_mpool->p_alloc_head;

		do
		{
			/*
			 * If failed:
			 * Broken link
			 */
			UTIL_ASSERT( p_i != NULL );
			UTIL_ASSERT( p_i->p_next != NULL );
			UTIL_ASSERT( p_i->p_next->p_prev == p_i );

			/* encounter a large block */
			if( size <= p_i->size )
			{
				/* update allocation head pointer */
				p_mpool->p_alloc_head = p_i->p_next;

				/* split the block when possible */
				if( size + MBLK_SMALLEST_SIZE <= p_i->size )
				{
					mpool_split( p_i, size, p_mpool );
				}

				mpool_remove(p_i, p_mpool);
				mlst_insert(p_i, p_mlst );
				p_ret = (os_byte_t*)p_i + MBLK_HEADER_SIZE;

				break;
			}

			p_i = p_i->p_next;

		} while( p_i != p_mpool->p_alloc_head );
	}

	return p_ret;
}

/*
 * Free memory and return to pool
 */
UTIL_UNSAFE
void mpool_free( void *p, mpool_t *p_mpool )
{
	mblk_t *p_mblk;

	/*
	 * If failed:
	 * NULL pointer passed to p, p_mlst or p_mpool
	 */
	UTIL_ASSERT( p != NULL );
	UTIL_ASSERT( p_mpool != NULL );

	/*
	 * If failed:
	 * p unaligned
	 */
	UTIL_ASSERT( MPOOL_IS_ALIGNED(p) );

	p_mblk = (mblk_t*)( (os_byte_t*)p - MBLK_HEADER_SIZE );

	/*
	 * If failed:
	 * Unaligned size or broken link
	 * Corrupted header
	 */
	UTIL_ASSERT( MPOOL_IS_ALIGNED(p_mblk->size) );
	UTIL_ASSERT( p_mblk->p_prev != NULL );
	UTIL_ASSERT( p_mblk->p_next != NULL );
	UTIL_ASSERT( p_mblk->p_mlst != NULL );

	mlst_remove( p_mblk );
	mpool_insert( p_mblk, p_mpool );
	mpool_merge( p_mblk, p_mpool );
}

/*
 * Gather memory block information
 */
UTIL_UNSAFE
void mblk_gather_info( const mblk_t *p_mblk, mblk_info_t *p_info)
{
	/*
	 * If failed:
	 * Invalid parameters
	 */
	UTIL_ASSERT( p_mblk != NULL );
	UTIL_ASSERT( p_info != NULL );

	p_info->size = p_mblk->size;
}

/*
 * Gather memory list information
 */
UTIL_UNSAFE
void mlst_gather_info( const mlst_t *p_mlst, mlst_info_t *p_info)
{
	mblk_t *p_i;
	uint_t count = 0, size = 0;

	/*
	 * If failed:
	 * Invalid parameters
	 */
	UTIL_ASSERT( p_mlst != NULL );
	UTIL_ASSERT( p_info != NULL );

	if( p_mlst->p_head != NULL )
	{
		p_i = p_mlst->p_head;

		do {
			size += p_i->size;
			count++;

			p_i = p_i->p_next;

		} while( p_i != p_mlst->p_head );
	}

	p_info->count = count;
	p_info->size = size;
}

/*
 * Gather memory pool information
 */
UTIL_UNSAFE
void mpool_gather_info( const mpool_t *p_mpool, mpool_info_t *p_info)
{
	mblk_t *p_i;
	uint_t count = 0, size = 0;

	/*
	 * If failed:
	 * Invalid parameters
	 */
	UTIL_ASSERT( p_mpool != NULL );
	UTIL_ASSERT( p_info != NULL );

	if( p_mpool->p_head != NULL )
	{
		/*
		 * If failed:
		 * p_head indicates pool not empty, but p_alloc_head otherwise
		 */
		UTIL_ASSERT(p_mpool->p_alloc_head != NULL );

		p_i = p_mpool->p_head;

		do {
			size += p_i->size;
			count++;

			p_i = p_i->p_next;
		} while( p_i != p_mpool->p_head );
	}

	p_info->count = count;
	p_info->size = size;
}

#include "../include/api.h"

/**
 * @brief Allocates a continuous memory block to the calling thread
 * @param size the requested size of the continuous memory block
 * @retval !NULL allocation successful
 * @retval NULL allocation failed because of low memory
 * @note This function can only be called in a thread context.
 * @brief This function is used to dynamically allocate large pieces of
 * continuous memory region to the calling thread, or used for data
 * sharing between different threads and thread to interrupts.
 *
 * When the allocation is successful, the function returns a pointer to
 * an aligned read-write address, unless allocation unsuccessful due to
 * low memory, where NULL is returned and allocation is aborted.
 * The application must check the return value against NULL to prevent
 * accessing a NULL pointer.
 *
 * The actual allocated size might be larger than the requested size
 * because the system will sometimes allocate extra bytes to keep the
 * system memory pool aligned. Depending on the platform, accessing data
 * placed in an unaligned address might result in faults or performance
 * issues. The system will ensure an aligned start address is returned.
 * The actual allocated size can be obtained on @ref os_memory_get_block_info.
 * When the requested size is 0, an allocation will still be attempted
 * as normal.
 *
 * When a non-NULL value is returned, and the allocated memory is no
 * longer useful, call @ref os_memory_free as soon as possible to free
 * the memory to the system memory pool to be recycled. A freed memory
 * cannot be accessed again and cannot be freed again. The memory allocated
 * by one thread can be freed by another thread or even interrupts. Upon
 * thread deletion, the unfreed memory will be automatically recycled.
 */
UTIL_SAFE
void *os_memory_allocate( os_uint_t size )
{
	void *p_ret;

	UTIL_LOCK_EVERYTHING();
	p_ret = mpool_alloc( size, &g_mpool, &g_sch.p_current->mlst);
	UTIL_UNLOCK_EVERYTHING();

	return p_ret;
}

/*
 * @brief Frees a piece of memory
 * @param p a non-NULL pointer previously returned by @ref os_memory_allocate
 * @details This function recycles dynamic memory when it is no longer needed
 * @note This function can be used in thread or interrupt context.
 */
UTIL_SAFE
void os_memory_free( void *p )
{
	UTIL_ASSERT( p != NULL );

	UTIL_LOCK_EVERYTHING();
	mpool_free(p, &g_mpool);
	UTIL_UNLOCK_EVERYTHING();
}

/**
 * @brief Obtain information about a memory block
 * @param p a non-NULL pointer previously returned by @ref os_memory_allocate
 * @param p_info a pointer to a struct where obtained info should be stored
 */
UTIL_SAFE
void os_memory_get_block_info( void *p, os_memory_block_info_t *p_info )
{
	mblk_t *p_mblk;
	mblk_info_t info;

	/*
	 * If failed:
	 * Invalid parameter(s)
	 */
	UTIL_ASSERT( p != NULL );
	UTIL_ASSERT( p_info != NULL );

	p_mblk = (mblk_t*)( (os_byte_t*)p - MBLK_HEADER_SIZE );

	UTIL_LOCK_EVERYTHING();
	mblk_gather_info( p_mblk, &info );
	UTIL_UNLOCK_EVERYTHING();

	p_info->block_size = info.size;
}

/**
 * @brief Obtain memory allocation details of system pool
 * @param p_info a pointer to a struct where obtained info should be stored
 */
UTIL_SAFE
void os_memory_get_pool_info( os_memory_pool_info_t *p_info )
{
	mpool_info_t info;

	/*
	 * If failed:
	 * Invalid parameter
	 */
	UTIL_ASSERT( p_info != NULL );

	UTIL_LOCK_EVERYTHING();
	mpool_gather_info( &g_mpool, &info );
	UTIL_UNLOCK_EVERYTHING();

	p_info->num_blocks = info.count;
	p_info->pool_size = info.size;
}

/**
 * @brief Obtain memory allocation details of a thread
 * @param h_thread handle to a thread. Pass 0 for current thread
 * @param p_info a pointer to a struct where obtained info should be stored
 */
UTIL_SAFE
void os_memory_get_thread_info( os_handle_t h_thread, os_memory_thread_info_t *p_info )
{
	thd_cblk_t *p_thd;
	mlst_info_t info;

	/*
	 * If failed:
	 * Invalid parameter
	 */
	UTIL_ASSERT( p_info != NULL );

	if( h_thread == 0)
		p_thd = g_sch.p_current;
	else
		p_thd = (thd_cblk_t*)h_thread;

	UTIL_LOCK_EVERYTHING();
	mlst_gather_info( &p_thd->mlst, &info );
	UTIL_UNLOCK_EVERYTHING();

	p_info->num_blocks = info.count;
	p_info->thread_size = info.size;
}

