/*******************************************************************************
** Copyright (c) 2010 The Krell Institute. All Rights Reserved.
**
** This library is free software; you can redistribute it and/or modify it under
** the terms of the GNU Lesser General Public License as published by the Free
** Software Foundation; either version 2.1 of the License, or (at your option)
** any later version.
**
** This library is distributed in the hope that it will be useful, but WITHOUT
** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
** FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
** details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this library; if not, write to the Free Software Foundation, Inc.,
** 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*******************************************************************************/

/** @file
 *
 * Declaration of the FPE.
 *
 */

#ifndef _CBTF_Runtime_FPE_
#define _CBTF_Runtime_FPE_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "Assert.h"

#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif
#include <ucontext.h>


/**
 * Type representing different floating-point exception (FPE) types.
 *
 * @note    Some systems fail to properly fill in the "si_code" when calling
 *          the FPE handler. Thus we must have an Unknown FPE to handle these
 *          cases.
 */
typedef enum {
    DivideByZero,        /**< Divide by zero. */
    Overflow,            /**< Overflow. */
    Underflow,           /**< Underflow. */
    InexactResult,       /**< Inexact result. */
    InvalidOperation,    /**< Invalid operation. */
    SubscriptOutOfRange, /**< Subscript out of range. */
    Unknown,             /**< Unknown FPE (no "si_code" provided). */
    AllFPE               /**< Toggle on all FPE's. */
} CBTF_FPEType;


/** Type representing a function pointer to a fpe event handler. */
typedef void (*CBTF_FPEEventHandler)(const CBTF_FPEType, const ucontext_t*);

void CBTF_FPEHandler(CBTF_FPEType, const CBTF_FPEEventHandler);

#endif
