/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__OFC_CONSOLE_H__)
#define __OFC_CONSOLE_H__

#include "ofc/core.h"
#include "ofc/types.h"

/**
 * \defgroup console APIs to perform primitive Console I/O
 *
 * These APIs allow an application to read and write to the console or
 * controlling terminal in a platform independent way.  The C-lib like 
 * ofc_printf calls these functions.
 */

/** \{ */

#if defined(__cplusplus)
extern "C"
{
#endif
/**
 * Write a buffer to standard output
 *
 * This routine is called by the ofc_printf function.
 *
 * \param obuf
 * Pointer to output buffer.  For compatibility with some platfom specific
 * code, this buffer must be NULL terminated.  The ofc_printf function
 * does NULL terminate the buffer.
 *
 * \param len
 * Number of characters to output
 */
OFC_CORE_LIB OFC_VOID
ofc_write_stdout(OFC_CCHAR *obuf, OFC_SIZET len);
OFC_CORE_LIB OFC_VOID
ofc_write_log(OFC_LOG_LEVEL level, OFC_CCHAR *obuf, OFC_SIZET len);

/**
 * Writes a null terminated string to the console
 *
 * Almost identical to ofc_write_stdout accept the length argument
 * is not required.
 *
 * \param obuf
 * The NULL terminated string to output
 */
OFC_CORE_LIB OFC_VOID
ofc_write_console(OFC_CCHAR *obuf);
  
/**
 * Read from the console
 *
 * \param inbuf
 * Pointer to a buffer to read characters into
 *
 * \param len
 * Size of the buffer.  The line read will be NULL terminated
 * unless the buffer is full.
 */
OFC_CORE_LIB OFC_VOID
ofc_read_line(OFC_CHAR *inbuf, OFC_SIZET len);

/**
 * Read a password from the console
 *
 * This is similar to ofc_read_line except the input will not be echoed.
 *
 * \param inbuf
 * Pointer to a buffer to read characters into
 *
 * \param len
 * Size of the buffer.  The line read will be NULL terminated
 * unless the buffer is full.
 */
OFC_CORE_LIB OFC_VOID
ofc_read_password(OFC_CHAR *inbuf, OFC_SIZET len);

OFC_CORE_LIB OFC_VOID
ofc_console_set_log_file(OFC_CHAR *log_file,
			 OFC_LARGE_INTEGER rollover_size,
			 OFC_UINT max_instance);
  
OFC_CORE_LIB OFC_VOID ofc_console_init(OFC_VOID);
OFC_CORE_LIB OFC_VOID ofc_console_destroy(OFC_VOID);

#if defined(__cplusplus)
}
#endif
/** \} */
#endif

