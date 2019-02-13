/******************************************************************************
 * Ordered list module
 *
 * This file is part of mRTOS.
 * SPDX-License-Identifier: MIT
 * Copyright (C) 2018 John Buyi Yu
 *****************************************************************************/
#ifndef H31343094_388A_4601_9088_EEB8B7D01B88
#define H31343094_388A_4601_9088_EEB8B7D01B88

#include <arch/pretype.h>



/********************************************************************
 * The struct members are read-only if they are accessed directly.
 * Only modify the structs through functions.
 ********************************************************************/



/********************************************************************
 * Visible to application
 ********************************************************************/



/********************************************************************
 * Hidden from application
 ********************************************************************/
#ifdef MRTOS_INTERNAL

/*
 * Opaque structs
 */
struct os_olist;       /* generic ordered list  */
struct os_olist_fifo;  /* FIFO ordered list     */
struct os_olist_prio;  /* priority ordered list */
struct os_olist_item;  /* ordered list item     */



/*
 * The ordered list item. The item keeps track of its parent list to update the
 * latter's head pointer during removal without requiring a separate 'remove
 * from' parameter.
 */
struct os_olist_item
{
	struct os_olist_item *volatile p_prev;
	struct os_olist_item *volatile p_next;
	struct os_olist      *volatile p_list;   /* parent list          */
	os_uint               volatile u_tag;    /* tag value (priority) */
};



/*
 * The FIFO and priority lists are wrappers around the actual list.
 * They are defined separately so that the compiler can ensure the
 * priority and FIFO enqueues don't intermix.
 *
 * The head and tail of the lists are linked together forming a circle.
 * This allows the list to keep track of both its head and tail using
 * only one pointer.
 *
 * One might doubt why not directly define the two lists instead of
 * wrapping around a common one, and use a void* parent pointer in
 * the item to keep track of the parent list. The reason for this is
 * purely aesthetic. During removal, to update the head pointer, the
 * removal function needs to cast the parent pointer to one of the list
 * types, but which one? They are indeed the same and casting to either
 * will work, but it can make the code confusing and misunderstanding.
 * Casting void* to pointer-to-object also violates MISRA C:2012 Rule 11.5.
 */
struct os_olist
{
	struct os_olist_item  *volatile p_head;
};

struct os_olist_fifo
{
	const struct os_olist _inherited;
};

struct os_olist_prio
{
	const struct os_olist _inherited;
};



/********************************************************************
 * FIFO lists are ordered by order of insertion. The oldest item will
 * be the head and new items are appended at the end. Popping a FIFO
 * will remove the oldest item.
 *
 * Priority lists are ordered by the tag value. The smallest value
 * will be the head, and larger tags will be at the end. If there are
 * multiple items with the same tag, the new item will be appended at
 * the last item with the tag. Popping a priority list will remove the
 * smallest item.
 *
 * FIFO lists also provide a 'rotate' method. It rotates the
 * circularly arranged FIFO list forward, so that the first item becomes
 * the last. It is identical to pop-and-enqueue.
 ********************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

	/* this must be called first on new lists */
	void      os_olist_fifo_init(struct os_olist_fifo* p_fifo);

	void      os_olist_fifo_enq(struct os_olist_fifo* p_fifo,
			                    struct os_olist_item* p_item);

	struct os_olist_item*
	          os_olist_fifo_pop(struct os_olist_fifo* p_fifo);

	void      os_olist_fifo_rotate(struct os_olist_fifo* p_fifo);



	/* this must be called first on new lists */
	void      os_olist_prio_init(struct os_olist_prio* p_prio);

	void      os_olist_prio_enq(struct os_olist_prio* p_prio,
			                    struct os_olist_item* p_item,
							    os_uint u_tag);

	struct os_olist_item*
	          os_olist_prio_pop(struct os_olist_prio* p_prio);



	/* this must be called first on new items */
	void      os_olist_item_init(struct os_olist_item* p_item);

	void      os_olist_item_remove(struct os_olist_item* p_item);

#ifdef __cplusplus
}
#endif

#endif /* MRTOS_INTERNAL */



#endif /* H31343094_388A_4601_9088_EEB8B7D01B88 */
