/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#define __OFC_CORE_DLL__

#include "ofc/types.h"
#include "ofc/impl/backtraceimpl.h"

OFC_VOID ofc_backtrace(OFC_VOID **trace, OFC_SIZET len)
{
  ofc_backtrace_impl(trace, len);
}
