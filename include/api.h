/** ************************************************************************
 * @file api.h
 * @brief Application programming interface
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
#ifndef H1CB9096F_13C2_4118_B608_F147C53BE57D
#define H1CB9096F_13C2_4118_B608_F147C53BE57D

#include "portable.h"

typedef struct {
	void *p_pool_mem;    /* pointer to pool memory, must be aligned */
	os_uint_t pool_size; /* pool size, must be aligned              */
} os_config_t;

void      os_init                 ( const os_config_t *p_config );
void      os_start                ( void );
os_uint_t os_get_heartbeat_counter( void );
void      os_enter_critical       ( void );
void      os_exit_critical        ( void );

/* Information about a memory block */
typedef struct {
	os_uint_t block_size; /* size of memory block */
} os_memory_block_info_t;

/* Information about the system memory pool */
typedef struct {
	os_uint_t pool_size;  /* size of memory pool             */
	os_uint_t num_blocks; /* number of blocks in memory pool */
} os_memory_pool_info_t;

/* Information about thread memory allocation */
typedef struct {
	os_uint_t thread_size; /* size of memory allocated to thread          */
	os_uint_t num_blocks;  /* number of memory blocks allocated to thread */
} os_memory_thread_info_t;

void * os_memory_allocate       ( os_uint_t size );
void   os_memory_free           ( void *p );
void   os_memory_get_block_info ( void *p, os_memory_block_info_t *p_info );
void   os_memory_get_thread_info( os_handle_t h_thread, os_memory_thread_info_t *p_info );
void   os_memory_get_pool_info  ( os_memory_pool_info_t *p_info );

typedef enum
{
	OS_THREAD_READY = 0,
	OS_THREAD_BLOCKED,
	OS_THREAD_SUSPENDED,
	OS_THREAD_DELETED
} os_thread_state_t;

os_handle_t       os_thread_create      ( os_uint_t prio, os_uint_t stack_size, void(*p_job) (void) );
void              os_thread_delete      ( os_handle_t h_thread );
void              os_thread_suspend     ( os_handle_t h_thread );
void              os_thread_resume      ( os_handle_t h_thread );
os_thread_state_t os_thread_get_state   ( os_handle_t h_thread );
os_handle_t       os_thread_get_current ( void );
void              os_thread_set_priority( os_handle_t h_thread, os_uint_t prio );
os_uint_t         os_thread_get_priority( os_handle_t h_thread );
void              os_thread_yield       ( void );
void              os_thread_delay       ( os_uint_t timeout );

os_handle_t os_semaphore_create          ( os_uint_t initial);
void        os_semaphore_delete          ( os_handle_t h_sem );
void        os_semaphore_reset           ( os_handle_t h_sem, os_uint_t initial );
os_uint_t   os_semaphore_get_counter     ( os_handle_t h_sem );
void        os_semaphore_post            ( os_handle_t h_sem );
os_bool_t   os_semaphore_wait            ( os_handle_t h_sem, os_uint_t timeout );
os_bool_t   os_semaphore_wait_nonblocking( os_handle_t h_sem );
os_bool_t   os_semaphore_peek_wait       ( os_handle_t h_sem );

os_handle_t os_mutex_create                    ( void );
void        os_mutex_delete                    ( os_handle_t h_mutex );
os_bool_t   os_mutex_peek_lock                 ( os_handle_t h_mutex );
os_bool_t   os_mutex_is_locked                 ( os_handle_t h_mutex );
os_bool_t   os_mutex_lock_nonblocking          ( os_handle_t h_mutex );
os_bool_t   os_mutex_lock                      ( os_handle_t h_mutex, os_uint_t timeout );
void        os_mutex_unlock                    ( os_handle_t h_mutex );


#endif /* H1CB9096F_13C2_4118_B608_F147C53BE57D */
