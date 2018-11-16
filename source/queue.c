/** ************************************************************************
 * @file queue.c
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
#include "../include/queue.h"
#include "../include/thread.h"
#include "../include/global.h"
#include "../include/api.h"

/*
 * Initialize queue
 */
UTIL_UNSAFE
void queue_init( queue_cblk_t *p_q, void *p_buffer, uint_t size)
{
	/*
	 * If failed:
	 * NULL pointer passed to p_q
	 */
	UTIL_ASSERT( p_q != NULL );

	p_q->p_buffer = (byte_t*)p_buffer;
	p_q->size = size;
	p_q->read = 0;
	p_q->write = 0;

	sch_q_init( &p_q->q_wait_read );
	sch_q_init( &p_q->q_wait_write );
}

/*
 * Initialize wait structure
 */
UTIL_UNSAFE
void queue_schinfo_read_init( queue_schinfo_read_t *p_schinfo, uint_t flag )
{
	/*
	 * If failed:
	 * NULL pointer passed to p_schinfo
	 */
	UTIL_ASSERT( p_schinfo != NULL );

	p_schinfo->result = false;
	p_schinfo->flag = flag;
}

/*
 * Initialize wait structure
 */
UTIL_UNSAFE
void queue_schnifo_write_init( queue_schinfo_write_t *p_schinfo, uint_t flag )
{
	/*
	 * If failed:
	 * NULL pointer passed to p_schinfo
	 */
	UTIL_ASSERT( p_schinfo != NULL );

	p_schinfo->result = false;
	p_schinfo->flag = flag;
}

/*
 * Delete a static queue
 */
UTIL_UNSAFE
void queue_delete_static( queue_cblk_t *p_q, sch_cblk_t *p_sch )
{
	sch_qitem_t *p_item;

	/*
	 * If failed:
	 * NULL pointer passed to p_q or p_sch
	 */
	UTIL_ASSERT( p_q != NULL );
	UTIL_ASSERT( p_sch != NULL );

	/* ready all reading threads */
	while( p_q->q_wait_read.p_head != NULL )
	{
		p_item = p_q->q_wait_read.p_head;

		/*
		 * If failed:
		 * cannot obtain thread from item
		 */
		UTIL_ASSERT( p_item->p_thd != NULL );

		thd_ready( p_item->p_thd, p_sch );
	}

	/* ready all writing threads */
	while( p_q->q_wait_write.p_head != NULL )
	{
		p_item = p_q->q_wait_write.p_head;

		/*
		 * If failed:
		 * cannot obtain thread from item
		 */
		UTIL_ASSERT( p_item->p_thd != NULL );

		thd_ready( p_item->p_thd, p_sch );
	}

	sch_reschedule_req(p_sch);
}

/*
 * Write to a queue
 */
UTIL_UNSAFE
void queue_write( queue_cblk_t *p_q, const byte_t *p_data, uint_t size )
{
	uint_t counter;
	uint_t write, qsize;
	byte_t *p_buffer;

	/*
	 * If failed:
	 * Invalid parameters
	 */
	UTIL_ASSERT( p_q != NULL );
	UTIL_ASSERT( p_data != NULL );

	/*
	 * If failed:
	 * Invalid buffer or index
	 */
	UTIL_ASSERT( p_q->p_buffer != NULL );
	UTIL_ASSERT( p_q->write < p_q->size );

	write = p_q->write;
	qsize = p_q->size;
	p_buffer = p_q->p_buffer;

	for( counter = 0; counter < size; counter++ )
	{
		p_buffer[write] = p_data[counter];

		if( write < qsize-1)
			write++;
		else
			write = 0;
	}

	p_q->write = write;
}

UTIL_UNSAFE
void queue_write_ahead( queue_cblk_t *p_q, const byte_t *p_data, uint_t size )
{
	uint_t counter;
	uint_t read, qsize;
	byte_t *p_buffer;

	/*
	 * If failed:
	 * Invalid parameters
	 */
	UTIL_ASSERT( p_q != NULL );
	UTIL_ASSERT( p_data != NULL );

	/*
	 * If failed:
	 * Invalid buffer or index
	 */
	UTIL_ASSERT( p_q->p_buffer != NULL );
	UTIL_ASSERT( p_q->read < p_q->size );

	read = p_q->read;
	p_buffer = p_q->p_buffer;
	qsize = p_q->size;

	for( counter = size; counter > 0; counter--)
	{
		if(read > 0)
			read--;
		else
			read = qsize-1;

		p_buffer[read] = p_data[counter-1];
	}

	p_q->read = read;

}

UTIL_UNSAFE
void queue_peek( const queue_cblk_t *p_q, byte_t *p_data, uint_t size )
{
	uint_t counter;
	uint_t read, qsize;
	byte_t *p_buffer;

	/*
	 * If failed:
	 * Invalid parameters
	 */
	UTIL_ASSERT( p_q != NULL );
	UTIL_ASSERT( p_data != NULL );

	/*
	 * If failed:
	 * Invalid buffer or index
	 */
	UTIL_ASSERT( p_q->p_buffer != NULL );
	UTIL_ASSERT( p_q->read < p_q->size );

	read = p_q->read;
	qsize = p_q->size;
	p_buffer = p_q->p_buffer;

	for( counter = 0; counter < size; counter++ )
	{
		p_data[counter] = p_buffer[read];

		if( read < qsize-1)
			read++;
		else
			read = 0;
	}
}

UTIL_UNSAFE
void queue_read( queue_cblk_t *p_q, byte_t *p_data, uint_t size )
{
	uint_t counter;
	uint_t read, qsize;
	byte_t *p_buffer;

	/*
	 * If failed:
	 * Invalid parameters
	 */
	UTIL_ASSERT( p_q != NULL );
	UTIL_ASSERT( p_data != NULL );

	/*
	 * If failed:
	 * Invalid buffer or index
	 */
	UTIL_ASSERT( p_q->p_buffer != NULL );
	UTIL_ASSERT( p_q->read < p_q->size );

	read = p_q->read;
	qsize = p_q->size;
	p_buffer = p_q->p_buffer;

	for( counter = 0; counter < size; counter++ )
	{
		p_data[counter] = p_buffer[read];

		if( read < qsize-1)
			read++;
		else
			read = 0;
	}

	p_q->read = read;
}

UTIL_UNSAFE
uint_t queue_get_used_size(const queue_cblk_t *p_q )
{
	uint_t write, read, size;

	/*
	 * If failed:
	 * NULL pointer passed to p_q
	 */
	UTIL_ASSERT( p_q != NULL );

	write = p_q->write;
	read = p_q->read;
	size = p_q->size;

	if( write >= read )
		return write - read;
	else
		/* (size - 1) - ( read - write  - 1) */
		return size - read + write;
}

UTIL_UNSAFE
uint_t queue_get_free_size(const queue_cblk_t *p_q )
{
	uint_t write, read, size;

	/*
	 * If failed:
	 * NULL pointer passed to p_q
	 */
	UTIL_ASSERT( p_q != NULL );

	write = p_q->write;
	read = p_q->read;
	size = p_q->size;

	if( read > write )
		return read - write - 1;
	else
		/* (size - 1) - ( write - read) */
		return size - 1 - write + read;
}

UTIL_UNSAFE
void queue_unlock_threads( queue_cblk_t *p_q, sch_cblk_t *p_sch )
{
	bool_t can_read = true, can_write = true;
	thd_cblk_t *p_thd;
	queue_schinfo_read_t *p_readinfo;
	queue_schinfo_write_t *p_writeinfo;

	/*
	 * If failed:
	 * NULL pointer passed to p_q
	 */
	UTIL_ASSERT( p_q != NULL );

	while( can_read || can_write )
	{
		if( can_write)
		{
			/* has writing threads */
			if( p_q->q_wait_write.p_head != NULL )
			{
				/*
				 * If failed:
				 * cannot obtain thread
				 */
				UTIL_ASSERT( p_q->q_wait_write.p_head->p_thd != NULL );
				p_thd = p_q->q_wait_write.p_head->p_thd;

				/*
				 * If failed:
				 * write info missing
				 */
				UTIL_ASSERT(p_thd->p_schinfo != NULL );
				p_writeinfo = p_thd->p_schinfo;

				/* has free space */
				if( p_writeinfo->size <= queue_get_free_size(p_q) )
				{
					/* check flags */
					if( p_writeinfo->flag & QUEUE_WRITE_AHEAD )
						queue_write_ahead( p_q, p_writeinfo->p_data, p_writeinfo->size );
					else
						queue_write( p_q, p_writeinfo->p_data, p_writeinfo->size );

					can_read = true;
					p_writeinfo->result = true;
					thd_ready( p_thd, p_sch );
				}

				/* not enough free space */
				else
					can_write = false;
			}
			/* no writing threads */
			else
				can_write = false;

		} /* if (can_write) */

		if( can_read )
		{
			/* has reading threads */
			if( p_q->q_wait_read.p_head != NULL )
			{
				/*
				 * If failed:
				 * cannot obtain thread
				 */
				UTIL_ASSERT(p_q->q_wait_read.p_head->p_thd != NULL);
				p_thd = p_q->q_wait_read.p_head->p_thd;

				/*
				 * If failed:
				 * read info missing
				 */
				UTIL_ASSERT( p_thd->p_schinfo != NULL );
				p_readinfo = p_thd->p_schinfo;

				/* has data */
				if( p_readinfo->size <= queue_get_used_size(p_q) )
				{
					/* check flags */
					if( p_readinfo->flag & QUEUE_READ_PEEK )
						queue_peek(p_q, p_readinfo->p_data, p_readinfo->size );
					else
						queue_read(p_q, p_readinfo->p_data, p_readinfo->size );

					can_write = true;
					p_readinfo->result = true;
					thd_ready( p_thd, p_sch );
				}
				else
					/* no data */
					can_read = false;
			}

			/* no reading threads */
			else
				can_read = false;
		} /* if (can_read) */
	}

	sch_reschedule_req( p_sch);
}

UTIL_SAFE
os_handle_t os_queue_create(os_uint_t size)
{
	queue_cblk_t *p_q;
	byte_t *p_buffer;

	UTIL_LOCK_EVERYTHING();
	p_q = mpool_alloc( sizeof(p_q), &g_mpool, &g_mlst );

	if( p_q != NULL )
	{
		p_buffer = mpool_alloc( size, &g_mpool, &g_mlst );

		if( p_buffer == NULL )
			mpool_free(p_q, &g_mpool);
		else
			queue_init( p_q, p_buffer, size);
	}
	UTIL_UNLOCK_EVERYTHING();

	return (os_handle_t)p_q;
}

UTIL_SAFE
void os_queue_delete(os_handle_t h_q)
{
	queue_cblk_t *p_q;
	p_q = (queue_cblk_t*)h_q;

	/*
	 * If failed:
	 * NULL pointer passed to p_q
	 */
	UTIL_ASSERT(p_q != NULL);

	UTIL_LOCK_EVERYTHING();

	queue_delete_static(p_q, &g_sch);

	/* free memory */
	mpool_free( p_q, &g_mpool );
	UTIL_UNLOCK_EVERYTHING();
}

UTIL_SAFE
void os_queue_reset(os_handle_t h_q)
{
	queue_cblk_t *p_q;

	p_q = (queue_cblk_t*)h_q;

	/*
	 * If failed:
	 * NULL pointer passed to p_q
	 */
	UTIL_ASSERT(p_q != NULL);

	UTIL_LOCK_EVERYTHING();
	p_q->read = 0;
	p_q->write = 0;
	queue_unlock_threads(p_q, &g_sch);
	UTIL_UNLOCK_EVERYTHING();
}

UTIL_SAFE
os_uint_t os_queue_get_size(os_handle_t h_q)
{
	queue_cblk_t *p_q;
	uint_t ret;
	p_q = (queue_cblk_t*)h_q;

	/*
	 * If failed:
	 * NULL pointer passed to p_q
	 */
	UTIL_ASSERT(p_q != NULL);

	UTIL_LOCK_EVERYTHING();
	ret = p_q->size;
	UTIL_UNLOCK_EVERYTHING();

	return ret;
}

UTIL_SAFE
os_uint_t os_queue_get_used_size(os_handle_t h_q)
{
	queue_cblk_t *p_q;
	uint_t ret;
	p_q = (queue_cblk_t*)h_q;

	/*
	 * If failed:
	 * NULL pointer passed to p_q
	 */
	UTIL_ASSERT(p_q != NULL);

	UTIL_LOCK_EVERYTHING();
	ret = queue_get_used_size(p_q);
	UTIL_UNLOCK_EVERYTHING();

	return ret;
}

UTIL_SAFE
os_uint_t os_queue_get_free_size(os_handle_t h_q)
{
	queue_cblk_t *p_q;
	uint_t ret;

	p_q = (queue_cblk_t*)h_q;

	/*
	 * If failed:
	 * NULL pointer passed to p_q
	 */
	UTIL_ASSERT(p_q != NULL);

	UTIL_LOCK_EVERYTHING();
	ret = queue_get_free_size(p_q);
	UTIL_UNLOCK_EVERYTHING();

	return ret;
}

UTIL_SAFE
os_bool_t os_queue_peek(os_handle_t h_q, void *p_data, os_uint_t size,
		os_uint_t timeout)
{
	queue_cblk_t *p_q;
	os_bool_t ret;
	queue_schinfo_read_t schinfo;
	p_q = (queue_cblk_t*)h_q;

	/*
	 * If failed:
	 * NULL pointer passed to p_q
	 */
	UTIL_ASSERT(p_q != NULL);

	UTIL_LOCK_EVERYTHING();
	if( queue_get_used_size(p_q) >= size )
	{
		queue_peek(p_q, p_data, size );
		ret = true;
	}
	else
	{
		queue_schinfo_read_init( &schinfo, QUEUE_READ_PEEK );
		schinfo.p_data = p_data;
		schinfo.size = size;
		thd_block_current(&p_q->q_wait_read, &schinfo, timeout, &g_sch );
		ret = schinfo.result;
	}

	UTIL_UNLOCK_EVERYTHING();
	return ret;
}

UTIL_SAFE
os_bool_t os_queue_peek_nb(os_handle_t h_q, void *p_data, os_uint_t size)
{
	queue_cblk_t *p_q;
	os_bool_t ret = false;
	p_q = (queue_cblk_t*)h_q;

	/*
	 * If failed:
	 * NULL pointer passed to p_q
	 */
	UTIL_ASSERT(p_q != NULL);

	UTIL_LOCK_EVERYTHING();
	if( queue_get_used_size(p_q) >= size )
	{
		queue_peek(p_q, p_data, size );
		ret = true;
	}

	UTIL_UNLOCK_EVERYTHING();
	return ret;
}

UTIL_SAFE
os_bool_t os_queue_send(os_handle_t h_q, const void *p_data, os_uint_t size,
		os_uint_t timeout)
{
	queue_cblk_t *p_q;
	os_bool_t ret;
	queue_schinfo_write_t schinfo;
	p_q = (queue_cblk_t*)h_q;

	/*
	 * If failed:
	 * NULL pointer passed to p_q
	 */
	UTIL_ASSERT(p_q != NULL);

	UTIL_LOCK_EVERYTHING();
	if( queue_get_free_size(p_q) >= size )
	{
		queue_write(p_q, p_data, size );
		queue_unlock_threads( p_q, &g_sch );
		ret = true;
	}
	else
	{
		queue_schnifo_write_init( &schinfo, 0 );
		schinfo.p_data = p_data;
		schinfo.size = size;
		thd_block_current(&p_q->q_wait_read, &schinfo, timeout, &g_sch );
		ret = schinfo.result;
	}

	UTIL_UNLOCK_EVERYTHING();
	return ret;
}

UTIL_SAFE
os_bool_t os_queue_send_nb(os_handle_t h_q, const void *p_data,
		os_uint_t size)
{
	queue_cblk_t *p_q;
	os_bool_t ret = false;
	p_q = (queue_cblk_t*)h_q;

	/*
	 * If failed:
	 * NULL pointer passed to p_q
	 */
	UTIL_ASSERT(p_q != NULL);

	UTIL_LOCK_EVERYTHING();
	if( queue_get_free_size(p_q) >= size )
	{
		queue_write(p_q, p_data, size );
		queue_unlock_threads( p_q, &g_sch );
		ret = true;
	}
	UTIL_UNLOCK_EVERYTHING();
	return ret;
}

UTIL_SAFE
os_bool_t os_queue_send_ahead(os_handle_t h_q, const void *p_data, os_uint_t size,
		os_uint_t timeout)
{
	queue_cblk_t *p_q;
	os_bool_t ret;
	queue_schinfo_write_t schinfo;
	p_q = (queue_cblk_t*)h_q;

	/*
	 * If failed:
	 * NULL pointer passed to p_q
	 */
	UTIL_ASSERT(p_q != NULL);

	UTIL_LOCK_EVERYTHING();
	if( queue_get_free_size(p_q) >= size )
	{
		queue_write_ahead(p_q, p_data, size );
		queue_unlock_threads( p_q, &g_sch );
		ret = true;
	}
	else
	{
		queue_schnifo_write_init( &schinfo, QUEUE_WRITE_AHEAD );
		schinfo.p_data = p_data;
		schinfo.size = size;
		thd_block_current(&p_q->q_wait_read, &schinfo, timeout, &g_sch );
		ret = schinfo.result;
	}

	UTIL_UNLOCK_EVERYTHING();
	return ret;
}

UTIL_SAFE
os_bool_t os_queue_send_ahead_nb(os_handle_t h_q, const void *p_data,
		os_uint_t size)
{
	queue_cblk_t *p_q;
	os_bool_t ret = false;
	p_q = (queue_cblk_t*)h_q;

	/*
	 * If failed:
	 * NULL pointer passed to p_q
	 */
	UTIL_ASSERT(p_q != NULL);

	UTIL_LOCK_EVERYTHING();
	if( queue_get_free_size(p_q) >= size )
	{
		queue_write_ahead(p_q, p_data, size );
		queue_unlock_threads( p_q, &g_sch );
		ret = true;
	}
	UTIL_UNLOCK_EVERYTHING();
	return ret;
}

UTIL_SAFE
os_bool_t os_queue_receive(os_handle_t h_q, void *p_data, os_uint_t size,
		os_uint_t timeout)
{
	queue_cblk_t *p_q;
	os_bool_t ret;
	queue_schinfo_read_t schinfo;
	p_q = (queue_cblk_t*)h_q;

	/*
	 * If failed:
	 * NULL pointer passed to p_q
	 */
	UTIL_ASSERT(p_q != NULL);

	UTIL_LOCK_EVERYTHING();
	if( queue_get_used_size(p_q) >= size )
	{
		queue_read(p_q, p_data, size );
		queue_unlock_threads( p_q, &g_sch );
		ret = true;
	}
	else
	{
		queue_schinfo_read_init( &schinfo, 0 );
		schinfo.p_data = p_data;
		schinfo.size = size;
		thd_block_current(&p_q->q_wait_read, &schinfo, timeout, &g_sch );
		ret = schinfo.result;
	}

	UTIL_UNLOCK_EVERYTHING();
	return ret;
}

UTIL_SAFE
os_bool_t os_queue_receive_nb(os_handle_t h_q, void *p_data, os_uint_t size)
{
	queue_cblk_t *p_q;
	os_bool_t ret = false;
	p_q = (queue_cblk_t*)h_q;

	/*
	 * If failed:
	 * NULL pointer passed to p_q
	 */
	UTIL_ASSERT(p_q != NULL);

	UTIL_LOCK_EVERYTHING();
	if( queue_get_used_size(p_q) >= size )
	{
		queue_read(p_q, p_data, size );
		queue_unlock_threads( p_q, &g_sch );
		ret = true;
	}

	UTIL_UNLOCK_EVERYTHING();
	return ret;
}



