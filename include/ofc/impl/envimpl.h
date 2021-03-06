/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__OFC_ENV_IMPL_H__)
#define __OFC_ENV_IMPL_H__

#include "ofc/types.h"
#include "ofc/core.h"
#include "ofc/env.h"

/**
 * \defgroup env_impl Environment Variable Implementation
 * \ingroup port
 *
 * This facility implements the platform specific environment handling
 *
 * \{
 */

#if defined(__cplusplus)
extern "C"
{
#endif

OFC_BOOL
ofc_env_get_impl(OFC_ENV_VALUE value, OFC_TCHAR *ptr, OFC_SIZET len);

#if defined(__cplusplus)
}
#endif

/* \} */
#endif

