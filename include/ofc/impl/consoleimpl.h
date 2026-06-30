/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__OFC_CONSOLE_IMPL_H__)
#define __OFC_CONSOLE_IMPL_H__

#include "ofc/types.h"
#include "ofc/core.h"

/**
 * \defgroup console_impl Console I/O Implementation
 * \ingroup port
 *
 * This facility implements the platform specific console I/O routines
 */

/** \{ */

#if defined(__cplusplus)
extern "C"
{
#endif

/**
 * Output a buffer to the console
 *
 * This routine should implement a write to the console.  Whether the
 * console is STDOUT or a physical console or a log file is platform
 * dependent
 *
 * \param obuf
 * Pointer to output buffer
 *
 * \param len
 * Number of characters to output
 */
OFC_VOID
ofc_write_stdout_impl(OFC_CCHAR *obuf, OFC_SIZET len);
OFC_VOID
ofc_write_log_impl(OFC_LOG_LEVEL level, OFC_CCHAR *obuf, OFC_SIZET len);

OFC_CORE_LIB OFC_VOID
ofc_write_console_impl(OFC_CCHAR *obuf);

OFC_VOID
ofc_read_stdin_impl(OFC_CHAR *inbuf, OFC_SIZET len);

OFC_VOID
ofc_read_password_impl(OFC_CHAR *inbuf, OFC_SIZET len);

OFC_CORE_LIB OFC_VOID
ofc_console_set_log_file_impl(OFC_CHAR *log_file,
			      OFC_LARGE_INTEGER rollover_size,
			      OFC_UINT max_instance);
OFC_CORE_LIB OFC_VOID ofc_console_init_impl(OFC_VOID);
OFC_CORE_LIB OFC_VOID ofc_console_destroy_impl(OFC_VOID);
#if defined(__cplusplus)
}
#endif
/** \} */
#endif

