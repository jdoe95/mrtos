/******************************************************************************
 * Nested locking/unlocking module.
 *
 * This file is part of mRTOS.
 * SPDX-License-Identifier: MIT
 * Copyright (C) 2018 John Buyi Yu
 *****************************************************************************/
#ifndef H011EB498_EE1D_436C_9FD3_035BA8FABE1B
#define H011EB498_EE1D_436C_9FD3_035BA8FABE1B

#include <arch/pretype.h>



/********************************************************************
 * Visible to application
 ********************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

		void 		os_lock(void);
		void 		os_unlock(void);
		os_uint 	os_lock_get_depth(void);

#ifdef __cplusplus
}
#endif



/********************************************************************
 * Hidden from application
 ********************************************************************/
#ifdef MRTOS_INTERNAL
#ifdef __cplusplus
extern "C" {
#endif

		/* this must be called first by os init */
		void 		os_lock_init(void);

#ifdef __cplusplus
}
#endif
#endif /* MRTOS_INTERNAL */

#endif /* H011EB498_EE1D_436C_9FD3_035BA8FABE1B */
