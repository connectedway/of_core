/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#define __OFC_CORE_DLL__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/impl/consoleimpl.h"

OFC_CORE_LIB OFC_VOID
ofc_write_stdout(OFC_CCHAR *obuf, OFC_SIZET len) {
    ofc_write_stdout_impl(obuf, len);
}

OFC_CORE_LIB OFC_VOID
ofc_write_log(OFC_LOG_LEVEL level, OFC_CCHAR *obuf, OFC_SIZET len) {
  ofc_write_log_impl(level, obuf, len);
}

OFC_CORE_LIB OFC_VOID
ofc_write_console(OFC_CCHAR *obuf) {
    ofc_write_console_impl(obuf);
}

OFC_CORE_LIB OFC_VOID
ofc_read_line(OFC_CHAR *inbuf, OFC_SIZET len) {
    ofc_read_stdin_impl(inbuf, len);
}

OFC_CORE_LIB OFC_VOID
ofc_read_password(OFC_CHAR *inbuf, OFC_SIZET len) {
    ofc_read_password_impl(inbuf, len);
}

OFC_CORE_LIB OFC_VOID
ofc_console_set_log_file(OFC_CHAR *log_file,
			 OFC_LARGE_INTEGER rollover_size,
			 OFC_UINT max_instance) {
  ofc_console_set_log_file_impl(log_file, rollover_size, max_instance);
}

OFC_CORE_LIB OFC_VOID ofc_console_init(OFC_VOID)
{
  ofc_console_init_impl();
}

OFC_CORE_LIB OFC_VOID ofc_console_destroy(OFC_VOID)
{
  ofc_console_destroy_impl();
}

