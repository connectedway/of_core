/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#define __OFC_CORE_DLL__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/libc.h"
#include "ofc/utf.h"
#include "ofc/heap.h"

OFC_CORE_LIB OFC_CHAR *
ofc_tstr2utf8(OFC_LPCTSTR utf16_str)
{
    OFC_SIZET utf16_len;
    OFC_SIZET utf8_len;
    OFC_SIZET i;
    OFC_CHAR *utf8_str;
    OFC_CHAR *utf8_ptr;
    OFC_UINT16 ch;

    if (utf16_str == OFC_NULL)
        return OFC_NULL;

    utf16_len = ofc_tstrlen(utf16_str);

    /* Calculate UTF-8 length needed */
    utf8_len = 0;
    for (i = 0; i < utf16_len; i++)
    {
        ch = (OFC_UINT16) utf16_str[i];
        if (ch < 0x80)
            utf8_len += 1;          /* ASCII */
        else if (ch < 0x800)
            utf8_len += 2;          /* 2-byte UTF-8 */
        else
            utf8_len += 3;          /* 3-byte UTF-8 (includes Hindi) */
        /* Note: Surrogate pairs (4-byte) not handled for simplicity */
    }

    utf8_str = ofc_malloc(utf8_len + 1);
    if (utf8_str == OFC_NULL)
        return OFC_NULL;

    utf8_ptr = utf8_str;

    for (i = 0; i < utf16_len; i++)
    {
        ch = (OFC_UINT16) utf16_str[i];

        if (ch < 0x80)
        {
            /* ASCII: 0xxxxxxx */
            *utf8_ptr++ = (OFC_CHAR) ch;
        }
        else if (ch < 0x800)
        {
            /* 2-byte: 110xxxxx 10xxxxxx */
            *utf8_ptr++ = (OFC_CHAR) (0xC0 | (ch >> 6));
            *utf8_ptr++ = (OFC_CHAR) (0x80 | (ch & 0x3F));
        }
        else
        {
            /* 3-byte: 1110xxxx 10xxxxxx 10xxxxxx */
            *utf8_ptr++ = (OFC_CHAR) (0xE0 | (ch >> 12));
            *utf8_ptr++ = (OFC_CHAR) (0x80 | ((ch >> 6) & 0x3F));
            *utf8_ptr++ = (OFC_CHAR) (0x80 | (ch & 0x3F));
        }
    }
    *utf8_ptr = '\0';
    return utf8_str;
}

OFC_CORE_LIB OFC_LPTSTR
ofc_utf82tstr(OFC_CCHAR *utf8_str)
{
    OFC_SIZET utf8_len;
    OFC_SIZET utf16_len;
    OFC_SIZET i;
    OFC_LPTSTR utf16_str;
    OFC_TCHAR *utf16_ptr;
    OFC_UINT8 b1, b2, b3;

    if (utf8_str == OFC_NULL)
        return OFC_NULL;

    utf8_len = ofc_strlen(utf8_str);

    /* Calculate UTF-16 length needed */
    utf16_len = 0;
    i = 0;
    while (i < utf8_len)
    {
        if ((utf8_str[i] & 0x80) == 0)
        {
            /* ASCII */
            i += 1;
        }
        else if ((utf8_str[i] & 0xE0) == 0xC0)
        {
            /* 2-byte sequence */
            i += 2;
        }
        else if ((utf8_str[i] & 0xF0) == 0xE0)
        {
            /* 3-byte sequence */
            i += 3;
        }
        else
        {
            /* Invalid sequence, skip */
            i += 1;
        }
        utf16_len++;
    }

    utf16_str = ofc_malloc((utf16_len + 1) * sizeof(OFC_TCHAR));
    if (utf16_str == OFC_NULL)
        return OFC_NULL;

    utf16_ptr = utf16_str;

    i = 0;
    while (i < utf8_len)
    {
        if ((utf8_str[i] & 0x80) == 0)
        {
            /* ASCII: 0xxxxxxx */
            *utf16_ptr++ = (OFC_TCHAR) utf8_str[i++];
        }
        else if ((utf8_str[i] & 0xE0) == 0xC0 && i + 1 < utf8_len)
        {
            /* 2-byte: 110xxxxx 10xxxxxx */
            b1 = (OFC_UINT8) utf8_str[i++];
            b2 = (OFC_UINT8) utf8_str[i++];
            if ((b2 & 0xC0) == 0x80)  /* Valid continuation byte */
            {
                *utf16_ptr++ = (OFC_TCHAR) (((b1 & 0x1F) << 6) | (b2 & 0x3F));
            }
            else
            {
                /* Invalid sequence, skip */
                utf16_ptr++;
            }
        }
        else if ((utf8_str[i] & 0xF0) == 0xE0 && i + 2 < utf8_len)
        {
            /* 3-byte: 1110xxxx 10xxxxxx 10xxxxxx */
            b1 = (OFC_UINT8) utf8_str[i++];
            b2 = (OFC_UINT8) utf8_str[i++];
            b3 = (OFC_UINT8) utf8_str[i++];
            if ((b2 & 0xC0) == 0x80 && (b3 & 0xC0) == 0x80)  /* Valid continuation bytes */
            {
                *utf16_ptr++ = (OFC_TCHAR) (((b1 & 0x0F) << 12) |
                                           ((b2 & 0x3F) << 6) |
                                           (b3 & 0x3F));
            }
            else
            {
                /* Invalid sequence, skip */
                utf16_ptr++;
            }
        }
        else
        {
            /* Invalid or unsupported sequence, skip */
            i++;
            utf16_ptr++;
        }
    }
    *utf16_ptr = (OFC_TCHAR) '\0';
    return utf16_str;
}