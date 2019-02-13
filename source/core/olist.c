/******************************************************************************
 * Ordered list module
 *
 * This file is part of mRTOS.
 * SPDX-License-Identifier: MIT
 * Copyright (C) 2018 John Buyi Yu
 *****************************************************************************/
#include <arch/assert.h>
#include <core/olist.h>



/********************************************************************
 * Macros
 ********************************************************************/
/*
 * Upcast a FIFO/priority list pointer to base list pointer
 */
#define OLIST_UPCAST(p_list)        ((struct os_olist*)(p_list))
#define OLIST_UPCAST_CONST(p_list)  ((const struct os_olist*)(p_list))



/********************************************************************
 * Internal functions
 ********************************************************************/
static void     olist_init(struct os_olist* p_list);

static struct os_olist_item*
                olist_pop(struct os_olist* p_list);



/********************************************************************
 * Function definitions
 ********************************************************************/
/*
 * Initializes ordered list
 */
static void
olist_init(struct os_olist* p_list)
{
	/* no parameter assertions in internal functions */

	p_list->p_head = NULL;
}

/*
 * Initializes a FIFO list
 */
void
os_olist_fifo_init(struct os_olist_fifo* p_fifo)
{
	/* parameter assertion */
	OS_ASSERT(p_fifo != NULL);

	/* call base constructor */
	olist_init( OLIST_UPCAST(p_fifo) );
}

/*
 * Initializes a priority list
 */
void
os_olist_prio_init(struct os_olist_prio* p_prio)
{
	/* parameter assertion */
	OS_ASSERT(p_prio != NULL);

	/* call base constructor */
	olist_init( OLIST_UPCAST(p_prio) );
}

/*
 * Initializes an ordered list item
 * The item is intended to be used inside a thread control block. The pointer
 * to the container thread needs to be provided.
 */
void
os_olist_item_init(struct os_olist_item* p_item)
{
	/* parameter assertions */
	OS_ASSERT( p_item != NULL);

	/* initialize members */
	p_item->p_next = p_item;
	p_item->p_prev = p_item;
	p_item->p_list = NULL;
	p_item->u_tag = 0U;
}

/*
 * Pops an item off a list (FIFO or priority).
 * The popped item will be removed from the head. The list must be non-empty.
 */
static struct os_olist_item*
olist_pop(struct os_olist* p_list)
{
	struct os_olist_item *p_item;

	/* no parameter assertions in internal functions */

	/* cannot pop from an empty list */
	OS_ASSERT(p_list->p_head != NULL);

	/* obtain item to pop */
	p_item = p_list->p_head;

	/* check the consistency of the item */
	OS_ASSERT(p_item->p_prev != NULL);
	OS_ASSERT(p_item->p_next != NULL);
	OS_ASSERT(p_item->p_prev->p_next == p_item);
	OS_ASSERT(p_item->p_next->p_prev == p_item);
	OS_ASSERT(p_item->p_list == p_list);

	/* the only item in list */
	if( p_item->p_next == p_item )
	{
		/* this condition must also be true */
		OS_ASSERT( p_item->p_prev == p_item );

		/* remove the only item by NULLing the head pointer */
		p_list->p_head = NULL;
	}

	/* not the only item in list */
	else
	{
		/* this condition must also be true */
		OS_ASSERT( p_item->p_prev != p_item );

		/* check the consistency of the next item */
		OS_ASSERT( p_item->p_next->p_prev != NULL );
		OS_ASSERT( p_item->p_next->p_next != NULL );
		OS_ASSERT( p_item->p_next->p_prev->p_next == p_item->p_next );
		OS_ASSERT( p_item->p_next->p_next->p_prev == p_item->p_next );
		OS_ASSERT( p_item->p_next->p_list == p_list );

		/* move the head pointer to the next item */
		p_list->p_head = p_item->p_next;

		/* remove the item */
		p_item->p_prev->p_next = p_item->p_next;
		p_item->p_next->p_prev = p_item->p_prev;
		p_item->p_next = p_item;
		p_item->p_prev = p_item;
	}

	/* update item list pointer */
	p_item->p_list = NULL;

	/* update item tag */
	p_item->u_tag = 0U;

	return p_item;
}

/*
 * Pops an item off a FIFO list.
 * The popped item will be removed from the head. The list must be non-empty.
 */
struct os_olist_item*
os_olist_fifo_pop(struct os_olist_fifo* p_fifo)
{
	/* parameter assertion */
	OS_ASSERT( p_fifo != NULL);

	/* call base method */
	return olist_pop( OLIST_UPCAST(p_fifo) );
}

/*
 * Pops an item off a priority list.
 * The popped item will be removed from the head. The list must be non-empty.
 */
struct os_olist_item*
os_olist_prio_pop(struct os_olist_prio* p_prio)
{
	/* parameter assertion */
	OS_ASSERT( p_prio != NULL);

	/* call base method */
	return olist_pop( OLIST_UPCAST(p_prio) );
}

/*
 * Enqueues an item on to a FIFO
 * The item will be appended at the end of the list.
 */
void
os_olist_fifo_enq(struct os_olist_fifo* p_fifo,
		struct os_olist_item* p_item)
{
	struct os_olist* p_q;
	struct os_olist_item* p_pos;

	/* parameter assertions */
	OS_ASSERT(p_fifo != NULL );
	OS_ASSERT(p_item != NULL);

	/* checks the consistency of the item */
	OS_ASSERT(p_item->p_prev == p_item);
	OS_ASSERT(p_item->p_next == p_item);
	OS_ASSERT(p_item->p_list == NULL);
	OS_ASSERT(p_item->u_tag == 0U);

	/* obtain base pointer */
	p_q = OLIST_UPCAST(p_fifo);

	/* the list is empty */
	if(p_q->p_head == NULL)
	{
		/* insert the item by updating the head pointer */
		p_q->p_head = p_item;
	}

	/* the list is not empty */
	else
	{
		/* obtain insert position */
		p_pos = p_q->p_head;

		/* check the consistency of the position item */
		OS_ASSERT(p_pos->p_prev != NULL);
		OS_ASSERT(p_pos->p_next != NULL);
		OS_ASSERT(p_pos->p_prev->p_next == p_pos);
		OS_ASSERT(p_pos->p_next->p_prev == p_pos);
		OS_ASSERT(p_pos->p_list == p_q);
		OS_ASSERT(p_pos->u_tag == 0U);

		/* insert the item by prepending to the head item */
		p_item->p_prev = p_pos->p_prev;
		p_item->p_next = p_pos;
		p_pos->p_prev->p_next = p_item;
		p_pos->p_prev = p_item;
	}

	/* update item list pointer */
	p_item->p_list = p_q;
}

/*
 * Rotates a non-empty FIFO list
 * The first item (head) will become the last item.
 */
void
os_olist_fifo_rotate(struct os_olist_fifo *p_fifo)
{
	struct os_olist* p_q;

	/* parameter assertion */
	OS_ASSERT(p_fifo != NULL);

	/* obtain base pointer */
	p_q = OLIST_UPCAST(p_fifo);

	/* the list must be non-empty */
	OS_ASSERT(p_q->p_head != NULL);

	/* check the consistency of the head item */
	OS_ASSERT(p_q->p_head->p_prev != NULL);
	OS_ASSERT(p_q->p_head->p_next != NULL);
	OS_ASSERT(p_q->p_head->p_prev->p_next == p_q->p_head);
	OS_ASSERT(p_q->p_head->p_next->p_prev == p_q->p_head);
	OS_ASSERT(p_q->p_head->p_list == p_q);
	OS_ASSERT(p_q->p_head->u_tag == 0U);

	/* check the consistency of the next item */
	OS_ASSERT(p_q->p_head->p_next->p_prev != NULL);
	OS_ASSERT(p_q->p_head->p_next->p_next != NULL);
	OS_ASSERT(p_q->p_head->p_next->p_prev->p_next == p_q->p_head->p_next);
	OS_ASSERT(p_q->p_head->p_next->p_next->p_prev == p_q->p_head->p_next);
	OS_ASSERT(p_q->p_head->p_next->p_list == p_q);
	OS_ASSERT(p_q->p_head->p_next->u_tag == 0U);

	/* move the head pointer to the next item */
	p_q->p_head = p_q->p_head->p_next;
}

/*
 * Enqueues an item on to a priority list.
 * The item will be inserted after the last item with a tag value smaller
 * than or equal to the tag value passed to the function. If the new item
 * is smaller than all items, the item will become the new head.
 */
void
os_olist_prio_enq(struct os_olist_prio* p_prio,
		struct os_olist_item* p_item, os_uint u_tag)
{
	struct os_olist* p_q;
	struct os_olist_item* p_pos;
	struct os_olist_item* p_iter;

	/* parameter assertions */
	OS_ASSERT(p_prio != NULL);
	OS_ASSERT(p_item != NULL);

	/* checks the consistency of the item */
	OS_ASSERT(p_item->p_prev == p_item);
	OS_ASSERT(p_item->p_next == p_item);
	OS_ASSERT(p_item->p_list == NULL);
	OS_ASSERT(p_item->u_tag == 0U);

	/* obtain base pointer */
	p_q = OLIST_UPCAST(p_prio);

	/* update item tag */
	p_item->u_tag = u_tag;

	/* initialize prepend position */
	p_pos = NULL;

	/* the list is empty, inserting first item */
	if(p_q->p_head == NULL)
	{
		/* update the head pointer */
		p_q->p_head = p_item;
	}

	/* inserting largest item */
	else if( p_item->u_tag > p_q->p_head->p_prev->u_tag)
	{
		/* prepend before head */
		p_pos = p_q->p_head;
	}

	/* inserting smallest item */
	else if(p_item->u_tag < p_q->p_head->u_tag )
	{
		/* prepend before head */
		p_pos = p_q->p_head;

		/* update head pointer */
		p_q->p_head = p_item;
	}

	/* search for insert location */
	else
	{
		/* search starts from second item */
		p_iter = p_q->p_head->p_next;

		do
		{
			/* check the consistency of the iterator item */
			OS_ASSERT(p_iter != NULL);
			OS_ASSERT(p_iter->p_prev != NULL);
			OS_ASSERT(p_iter->p_next != NULL);
			OS_ASSERT(p_iter->p_prev->p_next == p_iter);
			OS_ASSERT(p_iter->p_next->p_prev == p_iter);
			OS_ASSERT(p_iter->p_list == p_q);

			/* found prepend position */
			if( p_item->u_tag < p_iter->u_tag )
			{
				p_pos = p_iter;
				break;
			}

			p_iter = p_iter->p_next;

			/* loop should not reach head */
			OS_ASSERT(p_iter != p_q->p_head);

		} while(true);
	}

	/* prepend position is set */
	if (p_pos != NULL )
	{
		/* check the consistency of the position item */
		OS_ASSERT(p_pos->p_prev != NULL);
		OS_ASSERT(p_pos->p_next != NULL);
		OS_ASSERT(p_pos->p_prev->p_next == p_pos);
		OS_ASSERT(p_pos->p_next->p_prev == p_pos);
		OS_ASSERT(p_pos->p_list == OLIST_UPCAST(p_prio));

		p_item->p_prev = p_pos->p_prev;
		p_item->p_next = p_pos;
		p_pos->p_prev->p_next = p_item;
		p_pos->p_prev = p_item;
	}

	/* update item list pointer */
	p_item->p_list = p_q;
}

/*
 * Removes an item from its list
 * The item must be in a list.
 */
void
os_olist_item_remove(struct os_olist_item* p_item)
{
	struct os_olist* p_q;

	/* parameter assertion */
	OS_ASSERT( p_item != NULL );

	/* check the consistency of the item */
	OS_ASSERT( p_item->p_prev != NULL );
	OS_ASSERT( p_item->p_next != NULL );
	OS_ASSERT( p_item->p_next->p_prev == p_item );
	OS_ASSERT( p_item->p_prev->p_next == p_item );
	OS_ASSERT( p_item->p_list != NULL );

	/* obtain parent list */
	p_q = p_item->p_list;

	/* removing only item */
	if( p_item->p_next == p_item )
	{
		/* this condition must also be true */
		OS_ASSERT(p_item->p_prev == p_item);

		/* remove by updating head */
		p_q->p_head = NULL;
	}

	/* removing first item */
	else if( p_item == p_q->p_head )
	{
		/* this condition must also be true */
		OS_ASSERT( p_item->p_prev != p_item );

		/* check the consistency of the next item */
		OS_ASSERT( p_item->p_next->p_prev != NULL );
		OS_ASSERT( p_item->p_next->p_next != NULL );
		OS_ASSERT( p_item->p_next->p_prev->p_next == p_item->p_next );
		OS_ASSERT( p_item->p_next->p_next->p_prev == p_item->p_next );
		OS_ASSERT( p_item->p_next->p_list == p_q );

		/* move the head pointer to the next item */
		p_q->p_head = p_item->p_next;

		/* remove the item */
		p_item->p_prev->p_next = p_item->p_next;
		p_item->p_next->p_prev = p_item->p_prev;
		p_item->p_next = p_item;
		p_item->p_prev = p_item;
	}

	else
	{
		/* this condition must also be true */
		OS_ASSERT( p_item->p_prev != p_item );

		/* remove the item */
		p_item->p_prev->p_next = p_item->p_next;
		p_item->p_next->p_prev = p_item->p_prev;
		p_item->p_next = p_item;
		p_item->p_prev = p_item;
	}

	/* update item list pointer */
	p_item->p_list = NULL;

	/* update item tag value */
	p_item->u_tag = 0U;
}
