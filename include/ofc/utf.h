/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__OFC_UTF_H__)
#define __OFC_UTF_H__

#include "ofc/core.h"
#include "ofc/types.h"

/**
 * \defgroup utf UTF-8 Conversion Routines
 *
 * These APIs provide conversion between UTF-16 (TSTR) and UTF-8 strings.
 * Unlike the naive cstr conversion routines, these preserve Unicode characters
 * by properly encoding/decoding UTF-8.
 */

/** \{ */

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * Convert a UTF-16 string to UTF-8
 *
 * \param utf16_str
 * Pointer to null-terminated UTF-16 string (TSTR)
 *
 * \returns
 * Pointer to newly allocated UTF-8 string, or OFC_NULL if input is OFC_NULL.
 * Caller must free the returned string with ofc_free().
 *
 * This function properly encodes UTF-16 characters into UTF-8:
 * - ASCII (0x0000-0x007F) -> 1 byte
 * - BMP (0x0080-0x07FF) -> 2 bytes
 * - BMP (0x0800-0xFFFF) -> 3 bytes
 *
 * Note: Surrogate pairs (4-byte UTF-8) are not currently supported.
 */
OFC_CORE_LIB OFC_CHAR *
ofc_tstr2utf8(OFC_LPCTSTR utf16_str);

/**
 * Convert a UTF-8 string to UTF-16
 *
 * \param utf8_str
 * Pointer to null-terminated UTF-8 string
 *
 * \returns
 * Pointer to newly allocated UTF-16 string (TSTR), or OFC_NULL if input is OFC_NULL.
 * Caller must free the returned string with ofc_free().
 *
 * This function properly decodes UTF-8 sequences into UTF-16:
 * - 1 byte (0x00-0x7F) -> ASCII
 * - 2 bytes (0xC0-0xDF) -> BMP characters
 * - 3 bytes (0xE0-0xEF) -> BMP characters
 *
 * Invalid UTF-8 sequences are skipped.
 */
OFC_CORE_LIB OFC_LPTSTR
ofc_utf82tstr(OFC_CCHAR *utf8_str);

#if defined(__cplusplus)
}
#endif

/** \} */

#endif