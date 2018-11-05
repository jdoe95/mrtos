/** ************************************************************************
 * @file list.c
 * @brief Circular doubly linked list base class
 * @author John Yu buyi.yu@wne.edu
 *
 * This file is part of mRTOS.
 *
 * This list will be called by other components to form more complicated lists.
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
#include "../include/list.h"

/*
 * Initialize an item
 */
UTIL_UNSAFE
void lstitem_init(lstitem_t *p_item)
{
	/*
	 * If failed:
	 * NULL pointer passed to p_item
	 */
	UTIL_ASSERT(p_item != NULL);

	p_item->p_prev = p_item;
	p_item->p_next = p_item;
}

/*
 * Prepend, insert before an item
 */
UTIL_UNSAFE
void lstitem_prepend(lstitem_t* p_item, lstitem_t *p_pos)
{
	/*
	 * If failed:
	 * NULL pointer passed to p_item or p_pos
	 */
	UTIL_ASSERT( p_item != NULL );
	UTIL_ASSERT( p_pos != NULL );

	/*
	 * If failed:
	 * Item correpted, uninitialized, or already in list
	 */
	UTIL_ASSERT( p_item->p_prev == p_item );
	UTIL_ASSERT( p_item->p_next == p_item );

	/*
	 * If failed:
	 * List corrputed (left side of p_pos)
	 */
	UTIL_ASSERT( p_pos->p_prev->p_next == p_pos );

	p_item->p_prev = p_pos->p_prev;
	p_item->p_next = p_pos;
	p_pos->p_prev->p_next = p_item;
	p_pos->p_prev = p_item;
}

/*
 * Append, insert after an item
 */
UTIL_UNSAFE
void lstitem_append(lstitem_t* p_item, lstitem_t *p_pos)
{
	/*
	 * If failed:
	 * NULL pointer passed to p_item or p_pos
	 */
	UTIL_ASSERT( p_item != NULL );
	UTIL_ASSERT( p_pos != NULL );

	/*
	 * If failed:
	 * Item corrupted, uninitialized, or already in list
	 */
	UTIL_ASSERT( p_item->p_prev == p_item );
	UTIL_ASSERT( p_item->p_next == p_item );

	/*
	 * If failed:
	 * List corrupted (right side of p_pos)
	 */
	UTIL_ASSERT( p_pos->p_next->p_prev == p_pos );

	p_item->p_next = p_pos->p_next;
	p_item->p_prev = p_pos;
	p_pos->p_next->p_prev = p_item;
	p_pos->p_next = p_item;
}

/*
 * Remove an item
 */
UTIL_UNSAFE
void lstitem_remove(lstitem_t *p_item)
{
	/*
	 * If failed:
	 * NULL pointer passed to p_item
	 */
	UTIL_ASSERT( p_item != NULL );

	/*
	 * If failed:
	 * List corrupted
	 */
	UTIL_ASSERT( p_item->p_next != NULL );
	UTIL_ASSERT( p_item->p_prev != NULL );
	UTIL_ASSERT( p_item->p_prev->p_next == p_item );
	UTIL_ASSERT( p_item->p_next->p_prev == p_item );

	p_item->p_prev->p_next = p_item->p_next;
	p_item->p_next->p_prev = p_item->p_prev;
	p_item->p_next = p_item;
	p_item->p_prev = p_item;
}
