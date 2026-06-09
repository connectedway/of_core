/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#define __OFC_CORE_DLL__

#define AUTHENTICATE

#include "ofc/core.h"
#include "ofc/config.h"
#include "ofc/types.h"
#include "ofc/libc.h"
#include "ofc/path.h"
#include "ofc/net.h"
#include "ofc/lock.h"
#include "ofc/queue.h"

#include "ofc/file.h"
#include "ofc/heap.h"
#include "ofc/persist.h"

typedef struct {
    OFC_FST_TYPE type;        /**< The Mapped File System Type   */
    OFC_LPTSTR device;        /**< The device name */
    OFC_LPTSTR username;    /**< The user name  */
    OFC_LPTSTR password;    /**< The password  */
    OFC_LPTSTR domain;        /**< The workgroup/domain  */
    OFC_INT port;
    OFC_UINT num_dirs;        /**< Number of directory fields  */
    OFC_LPTSTR *dir;        /**< Array of directory entries.
				   First is the share */
    OFC_BOOL absolute;        /**< If the path is absolute */
    OFC_BOOL remote;        /**< If the path is remote */
} _OFC_PATH;

typedef struct {
    OFC_LPTSTR lpDevice;
    OFC_LPTSTR lpDesc;
    _OFC_PATH *map;
    OFC_BOOL thumbnail;
} PATH_MAP_ENTRY;

static PATH_MAP_ENTRY OfcPathMaps[OFC_MAX_MAPS];
static OFC_LOCK lockPath;

OFC_BOOL ofc_path_is_wild(OFC_LPCTSTR dir) {
    OFC_LPCTSTR p;
    OFC_BOOL wild;

    wild = OFC_FALSE;
    p = ofc_tstrtok(dir, TSTR("*?"));
    if (p != OFC_NULL && *p != TCHAR_EOS)
        wild = OFC_TRUE;
    return (wild);
}

OFC_CORE_LIB OFC_VOID
ofc_path_update(OFC_PATH *_path, OFC_PATH *_map) {
    OFC_UINT i;

    _OFC_PATH *path = (_OFC_PATH *) _path;
    _OFC_PATH *map = (_OFC_PATH *) _map;

    path->type = map->type;
    ofc_free(path->device);
    path->device = ofc_tstrdup(map->device);

    if (map->username != OFC_NULL) {
        ofc_free(path->username);
        path->username = ofc_tstrdup(map->username);
    }

    if (map->password != OFC_NULL) {
        ofc_free(path->password);
        path->password = ofc_tstrdup(map->password);
    }

    if (map->domain != OFC_NULL) {
        ofc_free(path->domain);
        path->domain = ofc_tstrdup(map->domain);
    }

    /*
     * If there is a server in the path, we want to remove it.
     */
    if (path->remote && path->num_dirs > 0) {
        ofc_free(path->dir[0]);
        for (i = 1; i < path->num_dirs; i++) {
            path->dir[i - 1] = path->dir[i];
        }
        path->num_dirs--;
    }

    path->absolute = map->absolute;
    path->remote = map->remote;
    if (path->remote)
        path->port = map->port;

    path->dir = ofc_realloc(path->dir,
                            (map->num_dirs + path->num_dirs) *
                            sizeof(OFC_TCHAR *));
    /*
     * Now insert the mapped paths in front of the existing path dirs
     * Move paths dirs
     */
    for (i = map->num_dirs + path->num_dirs; i > map->num_dirs; i--) {
        path->dir[i - 1] = path->dir[i - map->num_dirs - 1];
    }

    for (i = 0; i < map->num_dirs; i++) {
        path->dir[i] = ofc_tstrdup(map->dir[i]);
    }
    path->num_dirs += map->num_dirs;
    /*
     * This is ugly.  Wish workgroups were really part of the URL
     * If the number of directories is > 1, the first directory is a workgroup
     * and the second directory is not wild, then we want to squelch the
     * workgroup
     */
    if (path->num_dirs > 1 && !ofc_path_is_wild(path->dir[1]) &&
        lookup_workgroup(path->dir[0])) {
        ofc_free(path->dir[0]);
        for (i = 1; i < path->num_dirs; i++)
            path->dir[i - 1] = path->dir[i];
        path->num_dirs--;
        path->dir = ofc_realloc(path->dir,
                                path->num_dirs * sizeof(OFC_TCHAR *));
    }
}

static OFC_BOOL special(OFC_CTCHAR ct) {
    OFC_BOOL ret;

    switch (ct) {
        case TCHAR(' '):
        case TCHAR('<'):
        case TCHAR('>'):
        case TCHAR('#'):
        case TCHAR('%'):
        case TCHAR('{'):
        case TCHAR('}'):
        case TCHAR('|'):
        case TCHAR('\\'):
        case TCHAR('^'):
        case TCHAR('~'):
        case TCHAR('['):
        case TCHAR(']'):
        case TCHAR('`'):
        case TCHAR(';'):
        case TCHAR('/'):
        case TCHAR('?'):
        case TCHAR(':'):
        case TCHAR('@'):
        case TCHAR('='):
        case TCHAR('&'):
        case TCHAR('$'):
            ret = OFC_TRUE;
            break;

        default:
            ret = OFC_FALSE;
            break;
    }
    return (ret);
}

static OFC_BOOL terminator(OFC_CTCHAR c, OFC_CTCHAR *terms) {
    OFC_BOOL hit;
    OFC_INT i;

    if (c == '\0')
        hit = OFC_TRUE;
    else {
        for (i = 0, hit = OFC_FALSE; i < ofc_tstrlen(terms) && !hit; i++) {
            if (c == terms[i])
                hit = OFC_TRUE;
        }
    }
    return (hit);
}

#define THEX(val) \
  ((val >= TCHAR('0') && val <= TCHAR('9')) ? (val - TCHAR('0')) :    \
   (val >= TCHAR('a') && val <= TCHAR('f')) ? (val - TCHAR('a') + 10) :    \
   (val >= TCHAR('A') && val <= TCHAR('F')) ? (val - TCHAR('A') + 10) : 0)

#define HEX2T(val) \
  ((val >= 0 && val <= 9) ? val + TCHAR('0') : (val - 10 + 'A'))

static OFC_TCHAR *ParseEscaped(OFC_CTCHAR *cursor, OFC_CTCHAR **outcursor,
                               OFC_CTCHAR *terms) {
    OFC_LPCTSTR peek;
    OFC_BOOL hit;
    OFC_LPTSTR ret;
    OFC_INT len;
    OFC_TCHAR *p;
    OFC_TCHAR *outstr;

    outstr = OFC_NULL;
    *outcursor = cursor;

    if (cursor == OFC_NULL || terms == OFC_NULL)
        ret = OFC_NULL;
    else {
        /*
         * First, find the length
         */
        for (peek = cursor, len = 0, hit = OFC_FALSE; !hit;) {
            OFC_INT i;
            OFC_CHAR c;

            if (*peek == TCHAR ('%')) {
                peek++;
                for (c = 0x00, i = 0; *peek != TCHAR_EOS && i < 2; i++, peek++)
                    c = (c << 4) | THEX(*peek);
                len++;
            } else {
                if (terminator(*peek, terms)) {
                    hit = OFC_TRUE;
                } else {
                    peek++;
                    len++;
                }
            }
        }

        if (len > 0) {
            outstr = ofc_malloc(sizeof(OFC_TCHAR) * (len + 1));
            p = outstr;

            for (peek = cursor, hit = OFC_FALSE; !hit;) {
                OFC_INT i;
                OFC_CHAR c;

                if (*peek == TCHAR ('%')) {
                    peek++;
                    for (c = 0x00, i = 0; *peek != '\0' && i < 2; i++, peek++)
                        c = (c << 4) | THEX(*peek);
                    *p++ = c;
                } else {
                    if (terminator(*peek, terms)) {
                        hit = OFC_TRUE;
                    } else {
                        *p++ = *peek++;
                    }
                }
            }
            *p = TCHAR_EOS;
            *outcursor = peek;
        }
        ret = outstr;
    }
    return (ret);
}

static OFC_TCHAR *ParseUnescaped(OFC_CTCHAR *cursor, OFC_CTCHAR **outcursor,
				 OFC_CTCHAR *terms) {
    OFC_LPCTSTR peek;
    OFC_BOOL hit;
    OFC_LPTSTR ret;
    OFC_INT len;
    OFC_TCHAR *p;
    OFC_TCHAR *outstr;

    outstr = OFC_NULL;
    *outcursor = cursor;

    if (cursor == OFC_NULL || terms == OFC_NULL)
        ret = OFC_NULL;
    else {
        /*
         * First, find the length
         */
        for (peek = cursor, len = 0, hit = OFC_FALSE; !hit;) {
	    if (terminator(*peek, terms)) {
	      hit = OFC_TRUE;
	    } else {
	      peek++;
	      len++;
            }
        }

        if (len > 0) {
            outstr = ofc_malloc(sizeof(OFC_TCHAR) * (len + 1));
            p = outstr;

            for (peek = cursor, hit = OFC_FALSE; !hit;) {
		if (terminator(*peek, terms)) {
		  hit = OFC_TRUE;
		} else {
		  *p++ = *peek++;
		}
            }
            *p = TCHAR_EOS;
            *outcursor = peek;
        }
        ret = outstr;
    }
    return (ret);
}

OFC_CORE_LIB OFC_PATH *
ofc_path_init_path(OFC_VOID) {
    _OFC_PATH *path;

    path = ofc_malloc(sizeof(_OFC_PATH));
    path->type = OFC_FST_FILE;
    path->device = OFC_NULL;
    path->username = OFC_NULL;
    path->password = OFC_NULL;
    path->domain = OFC_NULL;
    path->port = 445;
    path->num_dirs = 0;
    path->dir = OFC_NULL;
    path->absolute = OFC_FALSE;
    path->remote = OFC_FALSE;

    return (path);
}

static OFC_VOID ofc_path_update_type(OFC_PATH *path) {
#if defined(OFC_FS_SMB)
    if ((ofc_path_type(path) == OFC_FST_UNKNOWN ||
         ofc_path_type(path) == OFC_FST_FILE) && ofc_path_remote(path))
      ofc_path_set_type (path, OFC_FST_SMB) ;
#endif

    if (ofc_path_type(path) == OFC_FST_UNKNOWN ||
        ofc_path_type(path) == OFC_FST_FILE) {
#if defined(OFC_FS_WIN32)
        ofc_path_set_type(path, OFC_FST_WIN32);
#elif defined(OFC_FS_WINCE)
        ofc_path_set_type(path, OFC_FST_WINCE);
#elif defined(OFC_FS_DARWIN)
        ofc_path_set_type(path, OFC_FST_DARWIN);
#elif defined(OFC_FS_LINUX)
        ofc_path_set_type(path, OFC_FST_LINUX);
#elif defined(OFC_FS_ANDROID)
        ofc_path_set_type(path, OFC_FST_ANDROID);
#elif defined(OFC_FS_FILEX)
        ofc_path_set_type(path, OFC_FST_FILEX);
#endif
    }
}

OFC_CORE_LIB OFC_PATH *
ofc_path_createW(OFC_LPCTSTR lpFileName)
{
  _OFC_PATH *path;
  OFC_LPCTSTR cursor;
  OFC_LPCTSTR p;
  OFC_TCHAR *portw;
  OFC_CHAR *porta;
  OFC_LPTSTR dir;
  OFC_TCHAR *credentials;
  OFC_BOOL wild;

  path = ofc_path_init_path();
  cursor = lpFileName;
  /*
   * path_parse - Parse a path into it's requisite parts.
   *
   * A path is in the form of:
   *   [uri:][\\[user[:pass[:domain]]@]server[:port]][\share][\][path\]file
   *   where uri can be smb, or drive letter
   */
  /*
   * parse a device.  The path either begins with a device or a
   * network credential.  If it starts with a network credential, there
   * is no device.
   *
   * See if it's a network path that begins with a network terminator
   * (\\ or //)
   */
  p = ofc_tstrtok(cursor, TSTR("\\/:"));
  if (*p == TCHAR_COLON)
    {
      path->device = ParseUnescaped(cursor, &cursor, TSTR(":"));
      cursor++;
      if ((ofc_tstrcmp(path->device, TSTR("cifs")) == 0) ||
          (ofc_tstrcmp(path->device, TSTR("smb")) == 0) ||
          (ofc_tstrcmp(path->device, TSTR("proxy")) == 0))
        path->remote = OFC_TRUE;
      /*
       * We expect a device, colon and two separators for a uri
       * We expect a device, colon and one separator for an absolute DOS path
       * We expect a device, colon and no separators for a relative DOS path
       */
      if ((cursor[0] == TCHAR_SLASH || cursor[0] == TCHAR_BACKSLASH) &&
          (cursor[1] == TCHAR_SLASH || cursor[1] == TCHAR_BACKSLASH))
        {
          cursor += 2;
          path->absolute = OFC_TRUE;
        }
      else if (cursor[0] == TCHAR_SLASH || cursor[0] == TCHAR_BACKSLASH)
        {
          cursor++;
          path->absolute = OFC_TRUE;
        }
    }
  else if ((cursor[0] == TCHAR_SLASH || cursor[0] == TCHAR_BACKSLASH) &&
           (cursor[1] == TCHAR_SLASH || cursor[1] == TCHAR_BACKSLASH))
    {
      /* UNC path */
      cursor += 2;
      path->remote = OFC_TRUE;
      path->absolute = OFC_TRUE;
    }
  else if (cursor[0] == TCHAR_SLASH || cursor[0] == TCHAR_BACKSLASH)
    {
      cursor++;
      path->absolute = OFC_TRUE;
    }

  path->num_dirs = 0;
  if (path->remote)
    {
      /*
       * There's an authority (remote access)
       * Find out if there's a credential.  There will be an unescapedd "@"
       */
      credentials = ParseEscaped(cursor, &p, TSTR("/\\@"));
      ofc_free(credentials);

      if (*p == TCHAR_AMP)
        {
          /*
           * we have user:pass:domain
           */
          path->username = ParseEscaped(cursor, &cursor, TSTR(":@"));
          if (*cursor == TCHAR_COLON)
            {
              cursor++;
              /*
               * we have a password:domain
               */
              path->password = ParseEscaped(cursor, &cursor, TSTR(":@"));
            }

          if (*cursor == TCHAR_COLON)
            {
              cursor++;
              /*
               * we have a domain
               */
              path->domain = ParseEscaped(cursor, &cursor, TSTR("@"));
            }
          cursor++;
        }
    }

  while (*cursor != TCHAR_EOS)
    {
      if (path->remote && path->num_dirs == 0)
	dir = ParseEscaped(cursor, &cursor, TSTR("\\/:"));
      else
	dir = ParseEscaped(cursor, &cursor, TSTR("\\/"));

      if (dir != OFC_NULL)
        {
          wild = ofc_path_is_wild(dir);

          if (ofc_tstrcmp(dir, TSTR("..")) == 0)
            {
              if (path->num_dirs > 0)
                {
                  path->num_dirs--;
                  ofc_free(path->dir[path->num_dirs]);
                  path->dir = ofc_realloc(path->dir,
                                          sizeof(OFC_LPCTSTR *) *
                                          (path->num_dirs));
                }
              ofc_free(dir);
            }
          else if (ofc_tstrcmp(dir, TSTR(".")) == 0)
            {
              /* Skip it */
              ofc_free(dir);
            }
          /*
           * if we're remote and if dir index is 1 we're looking at a share or
           * potentially a server if dir index 0 is a workgroup.  In the latter
           * case, if the current item is not a wildcard, we want to strip the
           * workgroup.
           */
          else if (path->remote && path->num_dirs == 1 && !wild &&
                   lookup_workgroup(path->dir[0]))
            {
              /*
               * Strip the workgroup.
               * Free the old one, and replace it with this one.
               */
              ofc_free(path->dir[0]);
              path->dir[0] = dir;
            }
          else
            {
              path->dir = ofc_realloc(path->dir,
                                      sizeof(OFC_LPCTSTR *) *
                                      (path->num_dirs + 1));
              path->dir[path->num_dirs] = dir;
              path->num_dirs++;
            }

          if ((*cursor == TCHAR(':')) &&
              path->remote &&
              (path->num_dirs == 1))
            {
              cursor++;
              portw = ParseUnescaped(cursor, &cursor, TSTR("\\/"));
              porta = ofc_tstr2cstr(portw);
              path->port = (OFC_INT) ofc_strtoul(porta, OFC_NULL, 10);
              ofc_free(portw);
              ofc_free(porta);
            }
        }
      if (*cursor != TCHAR_EOS)
        cursor++;
    }

  ofc_path_update_type((OFC_PATH *) path);

  return ((OFC_PATH *) path);
}

OFC_CORE_LIB OFC_PATH *
ofc_path_createA(OFC_LPCSTR lpFileName) {
    _OFC_PATH *ret;
    OFC_TCHAR *lptFileName;

    lptFileName = ofc_cstr2tstr(lpFileName);
    ret = ofc_path_createW(lptFileName);
    ofc_free(lptFileName);
    return ((OFC_PATH *) ret);
}

OFC_CORE_LIB OFC_VOID
ofc_path_delete(OFC_PATH *_path) {
    OFC_UINT i;
    _OFC_PATH *path = (_OFC_PATH *) _path;

    ofc_free(path->device);
    ofc_free(path->username);
    ofc_free(path->password);
    ofc_free(path->domain);
    for (i = 0; i < path->num_dirs; i++) {
        ofc_free(path->dir[i]);
    }
    ofc_free(path->dir);
    ofc_free(path);
}

static OFC_SIZET
ofc_path_out_char(OFC_TCHAR c, OFC_LPTSTR *dest, OFC_SIZET *rem) {
    if (*rem > 0) {
        (*rem)--;
        *(*dest)++ = c;
    }
    return (1);
}


static OFC_SIZET
ofc_path_out_escaped(OFC_LPCTSTR str, OFC_LPTSTR *filename, OFC_SIZET *rem) {
    OFC_LPCTSTR p;
    OFC_SIZET len;

    len = 0;
    for (p = str; *p != TCHAR_EOS; p++) {
        if (special(*p)) {
            OFC_TCHAR c;

            len += ofc_path_out_char(TCHAR('%'), filename, rem);
            c = *p >> 4;
            len += ofc_path_out_char(HEX2T(c), filename, rem);
            c = *p & 0x0F;
            len += ofc_path_out_char(HEX2T(c), filename, rem);
        } else
            len += ofc_path_out_char(*p, filename, rem);
    }

    return (len);
}

static OFC_SIZET
ofc_path_out_escaped_lc(OFC_LPCTSTR str, OFC_LPTSTR *filename, OFC_SIZET *rem) {
    OFC_LPCTSTR p;
    OFC_SIZET len;

    len = 0;
    for (p = str; *p != TCHAR_EOS; p++) {
        if (special(*p)) {
            OFC_TCHAR c;

            len += ofc_path_out_char(TCHAR('%'), filename, rem);
            c = *p >> 4;
            len += ofc_path_out_char(HEX2T(c), filename, rem);
            c = *p & 0x0F;
            len += ofc_path_out_char(HEX2T(c), filename, rem);
        } else
            len += ofc_path_out_char(OFC_TOLOWER(*p), filename, rem);
    }

    return (len);
}

static OFC_SIZET
ofc_path_out_str(OFC_LPCTSTR str, OFC_LPTSTR *filename, OFC_SIZET *rem) {
    OFC_LPCTSTR p;
    OFC_SIZET len;

    len = 0;
    for (p = str; *p != TCHAR_EOS; p++) {
        len += ofc_path_out_char(*p, filename, rem);
    }

    return (len);
}

OFC_CORE_LIB OFC_SIZET
ofc_path_printW(OFC_PATH *_path, OFC_LPTSTR *filename, OFC_SIZET *rem) {
    _OFC_PATH *path = (_OFC_PATH *) _path;
    OFC_TCHAR delimeter;
    OFC_SIZET len;
    OFC_UINT i;

    switch (path->type) {
        case OFC_FST_WIN32:
        case OFC_FST_FILEX:
            delimeter = TCHAR_BACKSLASH;
            break;

        case OFC_FST_SMB:
#if defined(__ANDROID__)
            delimeter = TCHAR_SLASH ;
#else
            delimeter = TCHAR_BACKSLASH;
#endif

        case OFC_FST_DARWIN:
        case OFC_FST_LINUX:
        case OFC_FST_ANDROID:
            delimeter = TCHAR_SLASH;
            break;

        case OFC_FST_UNKNOWN:
        case OFC_FST_FILE:
        default:
#if defined(OFC_FS_WIN32)
            delimeter = TCHAR_BACKSLASH ;
#else
            delimeter = TCHAR_SLASH;
#endif
            break;
    }
    /*
     * Need to do an update remainder kind of thing
     */
    len = 0;

    if (path->device != OFC_NULL) {
        len += ofc_path_out_escaped_lc(path->device, filename, rem);
        len += ofc_path_out_char(TCHAR_COLON, filename, rem);
        if (path->absolute) {
            len += ofc_path_out_char(delimeter, filename, rem);
            len += ofc_path_out_char(delimeter, filename, rem);
        }
    } else if (path->remote) {
        len += ofc_path_out_char(delimeter, filename, rem);
        len += ofc_path_out_char(delimeter, filename, rem);
    } else if (path->absolute) {
#if 0
        if (path->num_dirs > 0)
#endif
            len += ofc_path_out_char(delimeter, filename, rem);
    }

    if (path->remote)
      {
	/*
	 * We want to output credentials if either username or domain is
	 * not null.  It is possible for username to be null and domain not
	 * to be null in the case of active directory authentication where
	 * we are using the domain as the name of the cache
	 */
        if (path->username != OFC_NULL || path->domain != OFC_NULL)
	  {
	    if (path->username != OFC_NULL)
	      len += ofc_path_out_escaped(path->username, filename, rem);
	    /*
	     * We may have output a username, or we may not have.  we
	     * want to put a ':' out if we have either a password or domain
	     * yet to be output
	     */
	    if (path->password != OFC_NULL || path->domain != OFC_NULL)
	      {
                len += ofc_path_out_char(TCHAR_COLON, filename, rem);
	      }

	    if (path->password != OFC_NULL && path->password[0] != TCHAR_EOS)
	      {
		len += ofc_path_out_escaped(path->password, filename, rem);
	      }
	    if (path->domain != OFC_NULL)
	      {
		len += ofc_path_out_char(TCHAR_COLON, filename, rem);
		len += ofc_path_out_escaped(path->domain, filename, rem);
	      }
            len += ofc_path_out_char(TCHAR_AMP, filename, rem);
	  }
      }

    for (i = 0; i < path->num_dirs; i++) {
        if (path->dir[i] != OFC_NULL &&
            ofc_tstrcmp(path->dir[i], TSTR("")) != 0) {
            if (i == 0 && path->remote )
              len += ofc_path_out_escaped(path->dir[i], filename, rem);
            else
              len += ofc_path_out_str(path->dir[i], filename, rem);

            if (path->remote && (i == 0) && (path->port != 0)) {
                OFC_SIZET count;
                OFC_CHAR *porta;
                OFC_TCHAR *portw;

                count = 0;
                count = ofc_snprintf(OFC_NULL, count, "%d", path->port);
                porta = ofc_malloc((count + 1) * sizeof(OFC_CHAR));
                ofc_snprintf(porta, count + 1, "%d", path->port);
                portw = ofc_cstr2tstr(porta);
                len += ofc_path_out_char(TCHAR(':'), filename, rem);
                len += ofc_path_out_str(portw, filename, rem);
                ofc_free(porta);
                ofc_free(portw);
            }

            if ((i + 1) < path->num_dirs)
                len += ofc_path_out_char(delimeter, filename, rem);
        }
    }

    ofc_path_out_char(TCHAR_EOS, filename, rem);
    return (len);
}

OFC_CORE_LIB OFC_SIZET
ofc_path_printA(OFC_PATH *_path, OFC_LPSTR *filename, OFC_SIZET *rem) {
    _OFC_PATH *path = (_OFC_PATH *) _path;
    OFC_SIZET ret;
    OFC_SIZET orig_rem;
    OFC_TCHAR *tfilename;
    OFC_TCHAR *ptfilename;
    OFC_CHAR *pfilename;
    OFC_INT i;

    orig_rem = *rem;
    tfilename = ofc_malloc(*rem * sizeof(OFC_TCHAR));
    ptfilename = tfilename;
    ret = ofc_path_printW(path, &ptfilename, rem);
    if (filename != OFC_NULL) {
        pfilename = *filename;
        for (i = 0; i < OFC_MIN (orig_rem, ret + 1); i++)
            *pfilename++ = (OFC_CHAR) tfilename[i];
    }
    ofc_free(tfilename);
    return (ret);
}

OFC_CORE_LIB OFC_TCHAR *ofc_path_print_alloc(OFC_PATH *path)
{
  OFC_TCHAR *cursor;
  OFC_SIZET size;
  OFC_TCHAR *filename;

  cursor = OFC_NULL ;
  size = 0 ;

  size = ofc_path_print(path, &cursor, &size) ;
  filename = ofc_malloc ((size+1) * sizeof (OFC_TCHAR)) ;
  cursor = filename ;
  size++ ;
  ofc_path_print(path, &cursor, &size) ;
  return (filename);
}

OFC_CORE_LIB OFC_FST_TYPE ofc_path_type(OFC_PATH *_path) {
    _OFC_PATH *path = (_OFC_PATH *) _path;

    return (path->type);
}

/*
 * Inner workings of ofc_path_mapW.  Returns the resulting path
 * A map is a device (gen:/temp) or a server name (//gen/temp)
 */
OFC_CORE_LIB OFC_PATH *ofc_map_path(OFC_LPCTSTR lpFileName,
                                    OFC_LPTSTR *lppMappedName) {
    OFC_PATH *path;
    OFC_PATH *map;
    OFC_SIZET len;
    OFC_LPTSTR cursor;
#if defined(AUTHENTICATE)
    OFC_LPCTSTR server;
    OFC_BOOL updated;
    OFC_LPCTSTR previous_server;
#endif

    path = ofc_path_createW(lpFileName);

    /*
     * We will only consider the current directory if the path is
     * not remote and not absolute
     */
    if (!ofc_path_absolute(path) && !ofc_path_remote(path))
      {
	/*
	 * Do we have a current directory
	 */
	OFC_DWORD current_size;
	current_size = OfcGetCurrentDirectory(0, OFC_NULL);
	if (current_size > 0)
	  {
	    /* 
	     * There is a current directory
	     */
	    OFC_LPTSTR current_directory;
	    current_directory = ofc_malloc(sizeof(OFC_TCHAR) * current_size);
	    if (OfcGetCurrentDirectory(current_size, current_directory) <= current_size)
	      {
		/*
		 * We have obtained the current directory,
		 * prepend the original lpFileName with the current directory
		 * size of new file name is the ize of the current directory
		 * plus a delimeter, plus the size of the original filename
		 * plus a terminator.
		 */
		OFC_SIZET current_dir_len = ofc_tstrlen(current_directory);
		OFC_SIZET file_name_len = ofc_tstrlen(lpFileName);
		OFC_SIZET newlen = current_dir_len + 1 + file_name_len + 1;
		OFC_LPTSTR lpNewFileName = ofc_malloc(sizeof(OFC_TCHAR) * newlen);
		OFC_TCHAR *p = lpNewFileName;
		ofc_tstrcpy (p, current_directory);
		p += current_dir_len;
		*p++ = TCHAR_SLASH;
		ofc_tstrcpy(p, lpFileName);
		p += file_name_len;
		*p++ = TCHAR_EOS;
		ofc_path_delete(path);
		path = ofc_path_createW(lpNewFileName);
		ofc_free(lpNewFileName);
	      }
	    ofc_free(current_directory);
	  }
      }

#if defined(AUTHENTICATE)
    previous_server = OFC_NULL;
    do {
        updated = OFC_FALSE;
        server = ofc_path_device(path);

        if (server == OFC_NULL)
            server = ofc_path_server(path);

        if (server != OFC_NULL) {
            /*
             * We don't want to get stuck in a loop
             * protect against:
             * //aserver -> //user:domain:password@aserver
             *
             * although something like:
             * //aserver -> //bserver
             * //bserver -> //aserver
             * can still happen
             */
            if (previous_server == OFC_NULL ||
                ofc_tstrcmp(server, previous_server) != 0) {
                previous_server = server;
                map = ofc_path_map_deviceW(server);

                if (map != OFC_NULL) {
                    ofc_path_update(path, map);
                    updated = OFC_TRUE;
                }
            }
        }
    } while (updated);
#else
    for (map = ofc_path_map_deviceW (ofc_path_device(path)) ;
         map != OFC_NULL ;
         map = ofc_path_map_deviceW (ofc_path_device(map)))
      {
        ofc_path_update (path, map) ;
      } ;
#endif

    ofc_path_update_type(path);

    if (lppMappedName != OFC_NULL) {
        len = 0;
        len = ofc_path_printW(path, OFC_NULL, &len) + 1;

        *lppMappedName = ofc_malloc(len * sizeof(OFC_TCHAR));

        cursor = *lppMappedName;
        ofc_path_printW(path, &cursor, &len);
    }

    return (path);
}

/*
 * A map is a device (i.e. gen:/temp) or a server name: (i.e. //gen/temp
 */
OFC_CORE_LIB OFC_VOID
ofc_path_mapW(OFC_LPCTSTR lpFileName, OFC_LPTSTR *lppMappedName,
              OFC_FST_TYPE *filesystem) {
    OFC_PATH *path;

    path = ofc_map_path(lpFileName, lppMappedName);

    if (filesystem != OFC_NULL)
        *filesystem = ofc_path_type(path);
    ofc_path_delete(path);
}

OFC_CORE_LIB OFC_VOID
ofc_path_mapA(OFC_LPCSTR lpFileName, OFC_LPSTR *lppMappedName,
              OFC_FST_TYPE *filesystem) {
    OFC_TCHAR *lptFileName;
    OFC_TCHAR *lptMappedName;

    lptFileName = ofc_cstr2tstr(lpFileName);
    lptMappedName = OFC_NULL;
    ofc_path_mapW(lptFileName, &lptMappedName, filesystem);
    if (lppMappedName != OFC_NULL)
        *lppMappedName = ofc_tstr2cstr(lptMappedName);
    ofc_free(lptFileName);
    ofc_free(lptMappedName);
}

OFC_CORE_LIB OFC_VOID
ofc_path_init(OFC_VOID) {
    OFC_INT i;

    lockPath = ofc_lock_init();

    for (i = 0; i < OFC_MAX_MAPS; i++) {
        OfcPathMaps[i].lpDevice = OFC_NULL;
        OfcPathMaps[i].lpDesc = OFC_NULL;
        OfcPathMaps[i].map = OFC_NULL;
        OfcPathMaps[i].thumbnail = OFC_FALSE;
    }
}

OFC_CORE_LIB OFC_VOID
ofc_path_destroy(OFC_VOID) {
    OFC_INT i;

    for (i = 0; i < OFC_MAX_MAPS; i++) {
        if (OfcPathMaps[i].lpDevice != OFC_NULL)
            ofc_path_delete_mapW(OfcPathMaps[i].lpDevice);
    }

    ofc_lock_destroy(lockPath);
}

/**
 * Add a map for a virtual path
 *
 * \param lpVirtual
 * Virtual Patho
 *
 * \param map
 * path to use for translating virtual path
 */
OFC_CORE_LIB OFC_BOOL
ofc_path_add_mapW(OFC_LPCTSTR lpDevice, OFC_LPCTSTR lpDesc,
                  OFC_PATH *_map, OFC_FST_TYPE fsType, OFC_BOOL thumbnail) {
    _OFC_PATH *map = (_OFC_PATH *) _map;
    OFC_INT i;
    OFC_BOOL ret;
    PATH_MAP_ENTRY *free;
    OFC_TCHAR *lc;
    /*
     * Search for a free entry and make sure the device is not already mapped
     */
    ret = OFC_TRUE;
    free = OFC_NULL;

    ofc_lock(lockPath);

    for (i = 0; i < OFC_MAX_MAPS && ret == OFC_TRUE; i++) {
        if (OfcPathMaps[i].lpDevice == OFC_NULL) {
            if (free == OFC_NULL)
                free = &OfcPathMaps[i];
        } else {
            if (ofc_tstrcasecmp(OfcPathMaps[i].lpDevice, lpDevice) == 0) {
                ret = OFC_FALSE;
            }
        }
    }

    if (ret == OFC_TRUE) {
        if (free == OFC_NULL) {
            ret = OFC_FALSE;
        } else {
            if (lpDevice == OFC_NULL)
                ret = OFC_FALSE;
            else {
                map->type = fsType;
                free->lpDevice = ofc_tstrdup(lpDevice);
                for (lc = free->lpDevice; *lc != TCHAR_EOS; lc++)
                    *lc = OFC_TTOLOWER(*lc);
                free->lpDesc = ofc_tstrdup(lpDesc);
                free->map = map;
                free->thumbnail = thumbnail;
            }
        }
    }
    ofc_unlock(lockPath);
    return (ret);
}

OFC_CORE_LIB OFC_BOOL
ofc_path_add_mapA(OFC_LPCSTR lpDevice, OFC_LPCSTR lpDesc,
                  OFC_PATH *_map, OFC_FST_TYPE fsType, OFC_BOOL thumbnail) {
    OFC_BOOL ret;
    OFC_TCHAR *lptDevice;
    OFC_TCHAR *lptDesc;

    lptDevice = ofc_cstr2tstr(lpDevice);
    lptDesc = ofc_cstr2tstr(lpDesc);
    ret = ofc_path_add_mapW(lptDevice, lptDesc, _map, fsType, thumbnail);
    ofc_free(lptDevice);
    ofc_free(lptDesc);
    return (ret);
}

/**
 * Delete a map
 *
 * \param lpVirtual
 * Virtual path of map to delete
 */
OFC_CORE_LIB OFC_VOID
ofc_path_delete_mapW(OFC_LPCTSTR lpDevice) {
    OFC_INT i;
    PATH_MAP_ENTRY *pathEntry;

    /*
     * Search for a free entry and make sure the device is not already mapped
     */
    pathEntry = OFC_NULL;

    ofc_lock(lockPath);

    for (i = 0; i < OFC_MAX_MAPS && pathEntry == OFC_NULL; i++) {
        if (OfcPathMaps[i].lpDevice != OFC_NULL &&
            ofc_tstrcasecmp(OfcPathMaps[i].lpDevice, lpDevice) == 0)
            pathEntry = &OfcPathMaps[i];
    }

    if (pathEntry != OFC_NULL) {
        ofc_free(pathEntry->lpDevice);
        ofc_free(pathEntry->lpDesc);
        ofc_path_delete(pathEntry->map);
        pathEntry->lpDevice = OFC_NULL;
        pathEntry->map = OFC_NULL;
    }

    ofc_unlock(lockPath);
}

OFC_CORE_LIB OFC_VOID
ofc_path_delete_mapA(OFC_LPCSTR lpDevice) {
    OFC_TCHAR *lptDevice;

    lptDevice = ofc_cstr2tstr(lpDevice);
    ofc_path_delete_mapW(lptDevice);
    ofc_free(lptDevice);
}

OFC_CORE_LIB OFC_VOID
ofc_path_get_mapW(OFC_INT idx, OFC_LPCTSTR *lpDevice,
                  OFC_LPCTSTR *lpDesc, OFC_PATH **map,
                  OFC_BOOL *thumbnail) {
    ofc_lock(lockPath);

    *lpDevice = OfcPathMaps[idx].lpDevice;
    *lpDesc = OfcPathMaps[idx].lpDesc;
    *map = OfcPathMaps[idx].map;
    *thumbnail = OfcPathMaps[idx].thumbnail;
    ofc_unlock(lockPath);
}

OFC_CORE_LIB OFC_PATH *
ofc_path_map_deviceW(OFC_LPCTSTR lpDevice) {
    OFC_INT i;
    PATH_MAP_ENTRY *pathEntry;
    OFC_PATH *map;
    OFC_CTCHAR *p;
    OFC_TCHAR *lc;
    OFC_SIZET len;
    OFC_LPTSTR tstrDevice;

    /*
     * Search for a free entry and make sure the device is not already mapped
     */
    map = OFC_NULL;
    if (lpDevice != OFC_NULL) {
        /*
         * Peel off just the device portion.  Normal users pass in the
         * device only anyway but this allows the routine to be more general.
         * We prefer sending in a full path when determining if the path
         * is served by one of our maps
         */
        p = ofc_tstrtok(lpDevice, TSTR(":"));
        len = ((OFC_ULONG_PTR) p - (OFC_ULONG_PTR) lpDevice) /
              sizeof(OFC_TCHAR);
        tstrDevice = ofc_malloc((len + 1) * sizeof(OFC_TCHAR));

        ofc_tstrncpy(tstrDevice, lpDevice, len);
        tstrDevice[len] = TCHAR_EOS;

        for (lc = tstrDevice; *lc != TCHAR_EOS; lc++)
            *lc = OFC_TTOLOWER(*lc);

        pathEntry = OFC_NULL;

        for (i = 0; i < OFC_MAX_MAPS && pathEntry == OFC_NULL; i++) {
            if (OfcPathMaps[i].lpDevice != OFC_NULL &&
                ofc_tstrcasecmp(OfcPathMaps[i].lpDevice, tstrDevice) == 0) {
                pathEntry = &OfcPathMaps[i];
            }
        }

        ofc_free(tstrDevice);
        if (pathEntry != OFC_NULL)
            map = pathEntry->map;
    }
    return (map);
}

OFC_CORE_LIB OFC_PATH *
ofc_path_map_deviceA(OFC_LPCSTR lpDevice) {
    _OFC_PATH *ret;
    OFC_TCHAR *lptDevice;

    lptDevice = ofc_cstr2tstr(lpDevice);
    ret = ofc_path_map_deviceW(lptDevice);
    ofc_free(lptDevice);
    return ((OFC_PATH *) ret);
}

OFC_CORE_LIB OFC_SIZET ofc_path_make_urlW(OFC_LPTSTR *filename,
                                          OFC_SIZET *rem,
                                          OFC_LPCTSTR username,
                                          OFC_LPCTSTR password,
                                          OFC_LPCTSTR domain,
                                          OFC_LPCTSTR server,
                                          OFC_LPCTSTR share,
                                          OFC_LPCTSTR path,
                                          OFC_LPCTSTR file) {
    OFC_TCHAR delimeter;
    OFC_SIZET len;

    delimeter = TCHAR_BACKSLASH;
    /*
     * Need to do an update remainder kind of thing
     */
    len = 0;

    len += ofc_path_out_char(delimeter, filename, rem);
    len += ofc_path_out_char(delimeter, filename, rem);

    if (username != OFC_NULL || password != OFC_NULL || domain != OFC_NULL)
      {
       if (username != OFC_NULL)
         len += ofc_path_out_escaped(username, filename, rem);

       if (password != OFC_NULL || domain != OFC_NULL)
         {
           len += ofc_path_out_char(TCHAR_COLON, filename, rem);

           if (password != OFC_NULL)
             len += ofc_path_out_escaped(password, filename, rem);

           if (domain != OFC_NULL)
             {
               len += ofc_path_out_char(TCHAR_COLON, filename, rem);

               if (username == OFC_NULL ||
                   (ofc_tstrcmp(username, TSTR("")) == 0))
                 len += ofc_path_out_str(domain, filename, rem);
               else
                 len += ofc_path_out_escaped(domain, filename, rem);
             }
         }
        len += ofc_path_out_char(TCHAR_AMP, filename, rem);
      }

    if (server != OFC_NULL) {
        len += ofc_path_out_str(server, filename, rem);
        len += ofc_path_out_char(delimeter, filename, rem);
    }

    if (share != OFC_NULL) {
        len += ofc_path_out_str(share, filename, rem);
        len += ofc_path_out_char(delimeter, filename, rem);
    }

    if (path != OFC_NULL) {
        len += ofc_path_out_str(path, filename, rem);
        len += ofc_path_out_char(delimeter, filename, rem);
    }

    if (file != OFC_NULL) {
        len += ofc_path_out_str(file, filename, rem);
    }

    ofc_path_out_char(TCHAR_EOS, filename, rem);
    return (len);
}

OFC_CORE_LIB OFC_SIZET ofc_path_make_urlA(OFC_LPSTR *filename,
                                          OFC_SIZET *rem,
                                          OFC_LPCSTR username,
                                          OFC_LPCSTR password,
                                          OFC_LPCSTR domain,
                                          OFC_LPCSTR server,
                                          OFC_LPCSTR share,
                                          OFC_LPCSTR path,
                                          OFC_LPCSTR file) {
    OFC_LPTSTR tfilename;
    OFC_LPTSTR tcursor;
    OFC_LPTSTR tusername;
    OFC_LPTSTR tpassword;
    OFC_LPTSTR tdomain;
    OFC_LPTSTR tserver;
    OFC_LPTSTR tshare;
    OFC_LPTSTR tpath;
    OFC_LPTSTR tfile;
    OFC_SIZET len;
    OFC_SIZET orig_rem;
    OFC_LPSTR pfilename;
    OFC_INT i;

    tusername = ofc_cstr2tstr(username);
    tpassword = ofc_cstr2tstr(password);
    tdomain = ofc_cstr2tstr(domain);
    tserver = ofc_cstr2tstr(server);
    tshare = ofc_cstr2tstr(share);
    tpath = ofc_cstr2tstr(path);
    tfile = ofc_cstr2tstr(file);

    orig_rem = *rem;
    tfilename = ofc_malloc((*rem + 1) * sizeof(OFC_TCHAR));
    tcursor = tfilename;

    len = ofc_path_make_urlW(&tcursor, rem, tusername, tpassword, tdomain,
                             tserver, tshare, tpath, tfile);

    pfilename = *filename;
    for (i = 0; i < OFC_MIN (orig_rem, len + 1); i++)
        *pfilename++ = (OFC_CHAR) tfilename[i];

    ofc_free(tusername);
    ofc_free(tpassword);
    ofc_free(tdomain);
    ofc_free(tserver);
    ofc_free(tshare);
    ofc_free(tpath);
    ofc_free(tfile);
    ofc_free(tfilename);

    return (len);
}

#if defined(AUTHENTICATE)
OFC_CORE_LIB OFC_VOID
ofc_path_update_credentialsW(OFC_LPCTSTR filename, OFC_LPCTSTR username,
                             OFC_LPCTSTR password, OFC_LPCTSTR domain) {
    OFC_PATH *path;
    OFC_PATH *map;
    _OFC_PATH *_map;
    OFC_LPCTSTR server;

    path = ofc_path_createW(filename);

    server = ofc_path_server(path);

    if (server != OFC_NULL) {
        map = ofc_path_map_deviceW(ofc_path_server(path));

        if (map == OFC_NULL) {
            /*
             * We just want the path to contain a server (and authentication)
             */
            ofc_path_free_dirs(path);
            /*
             * Add server to data base
             */
            ofc_path_add_mapW(ofc_path_server(path), TSTR("Server"),
                              path, OFC_FST_SMB, OFC_TRUE);
            map = path;
        }

        ofc_lock(lockPath);

        _map = (_OFC_PATH *) map;

        if (_map->username != OFC_NULL)
            ofc_free(_map->username);
        _map->username = ofc_tstrdup(username);
        if (_map->password != OFC_NULL) {
            ofc_memset(_map->password, '\0',
                       ofc_tstrlen(_map->password) * sizeof(OFC_TCHAR));
            ofc_free(_map->password);
        }
        _map->password = ofc_tstrdup(password);
        if (_map->domain != OFC_NULL)
            ofc_free(_map->domain);
        _map->domain = ofc_tstrdup(domain);

        ofc_unlock(lockPath);
    }
}

#else
OFC_CORE_LIB OFC_VOID 
ofc_path_update_credentialsW(OFC_PATH *_path, OFC_LPCTSTR username,
                 OFC_LPCTSTR password, OFC_LPCTSTR domain)
{
  _OFC_PATH *path = (_OFC_PATH *) _path ;

  ofc_lock(lockPath) ;

  if (path->username != OFC_NULL)
    ofc_free(path->username) ;
  path->username = ofc_tstrdup (username) ;
  if (path->password != OFC_NULL)
     {
       ofc_memset (path->password, '\0',
           ofc_tstrlen (path->password) * sizeof (OFC_TCHAR)) ;
       ofc_free(path->password) ;
     }
  path->password = ofc_tstrdup (password) ;
  if (path->domain != OFC_NULL)
    ofc_free(path->domain) ;
  path->domain = ofc_tstrdup (domain) ;

  ofc_unlock (lockPath) ;
}
#endif


#if defined(AUTHENTICATE)
OFC_CORE_LIB OFC_VOID
ofc_path_update_credentialsA(OFC_LPCSTR filename, OFC_LPCSTR username,
                             OFC_LPCSTR password, OFC_LPCSTR domain) {
    OFC_TCHAR *tusername;
    OFC_TCHAR *tpassword;
    OFC_TCHAR *tdomain;
    OFC_TCHAR *tfilename;

    tfilename = ofc_cstr2tstr(filename);
    tusername = ofc_cstr2tstr(username);
    tpassword = ofc_cstr2tstr(password);
    tdomain = ofc_cstr2tstr(domain);
    ofc_path_update_credentialsW(tfilename, tusername, tpassword, tdomain);
    ofc_free(tfilename);
    ofc_free(tusername);
    ofc_free(tpassword);
    ofc_free(tdomain);
}

#else
OFC_CORE_LIB OFC_VOID 
ofc_path_update_credentialsA(OFC_PATH *path, OFC_LPCSTR username,
                OFC_LPCSTR password, OFC_LPCSTR domain)
{
  OFC_TCHAR *tusername ;
  OFC_TCHAR *tpassword ;
  OFC_TCHAR *tdomain ;
 
  tusername = ofc_cstr2tstr (username) ;
  tpassword = ofc_cstr2tstr (password) ;
  tdomain = ofc_cstr2tstr (domain) ;
  ofc_path_update_credentialsW(path, tusername, tpassword, tdomain) ;
  ofc_free(tusername) ;
  ofc_free(tpassword) ;
  ofc_free(tdomain) ;
}
#endif

OFC_CORE_LIB OFC_VOID
ofc_path_get_rootW(OFC_CTCHAR *lpFileName, OFC_TCHAR **lpRootName,
                   OFC_FST_TYPE *filesystem) {
    OFC_TCHAR *lpMappedPath;
    OFC_TCHAR *lpMappedName;
    _OFC_PATH *rootpath;
    _OFC_PATH *path;
    OFC_SIZET len;

    ofc_path_mapW(lpFileName, &lpMappedName, filesystem);
    path = ofc_path_createW(lpMappedName);

    if (path->remote) {
        rootpath = ofc_path_createW(TSTR("\\"));
        rootpath->absolute = OFC_TRUE;
        rootpath->device = ofc_tstrdup(path->device);
        /*
         * Then the share is part of the root
         */
        rootpath->remote = OFC_TRUE;
        rootpath->port = path->port;
        rootpath->username = ofc_tstrdup(path->username);
        rootpath->password = ofc_tstrdup(path->password);
        rootpath->domain = ofc_tstrdup(path->domain);
        if (path->num_dirs > 1) {
            rootpath->num_dirs = 2;
            rootpath->dir = ofc_malloc(sizeof(OFC_LPCTSTR *) *
                                       rootpath->num_dirs);
            rootpath->dir[0] = ofc_tstrdup(path->dir[0]);
            rootpath->dir[1] = ofc_tstrdup(path->dir[1]);
        }
    } else
        rootpath = ofc_path_createW(lpMappedName);

    len = 0;
    len = ofc_path_printW(rootpath, OFC_NULL, &len);

    if (rootpath->type == OFC_FST_WIN32)
        /* Terminating slash */
        len++;
    /* EOS */
    len++;
    *lpRootName = ofc_malloc(len * sizeof(OFC_TCHAR));
    lpMappedPath = *lpRootName;
    len = ofc_path_printW(rootpath, &lpMappedPath, &len);
    if (rootpath->type == OFC_FST_WIN32)
        (*lpRootName)[len++] = TCHAR_BACKSLASH;
    (*lpRootName)[len] = TCHAR_EOS;
    ofc_path_delete(rootpath);
    ofc_path_delete(path);
    ofc_free(lpMappedName);
}

OFC_CORE_LIB OFC_VOID
ofc_path_get_rootA(OFC_CCHAR *lpFileName, OFC_CHAR **lpRoot,
                   OFC_FST_TYPE *filesystem) {
    OFC_TCHAR *lptFileName;
    OFC_TCHAR *lptRoot;

    lptFileName = ofc_cstr2tstr(lpFileName);
    ofc_path_get_rootW(lptFileName, &lptRoot, filesystem);
    *lpRoot = ofc_tstr2cstr(lptRoot);
    ofc_free(lptFileName);
    ofc_free(lptRoot);
}

OFC_CORE_LIB OFC_BOOL ofc_path_remote(OFC_PATH *_path) {
    _OFC_PATH *path = (_OFC_PATH *) _path;
    OFC_BOOL ret;

    ret = OFC_FALSE;
    if (path->remote)
        ret = OFC_TRUE;
    return (ret);
}

OFC_CORE_LIB OFC_BOOL ofc_path_hidden(OFC_PATH *_path)
{
  OFC_BOOL ret;
  OFC_LPCTSTR filename;

  ret = OFC_FALSE;

  filename = ofc_path_filename(_path);
  if (filename != OFC_NULL && ofc_tstrlen(filename) > 0)
    {
      if (*filename == TCHAR('.'))
        ret = OFC_TRUE;
    }

  return (ret);
}

OFC_CORE_LIB OFC_INT ofc_path_port(OFC_PATH *_path) {
    _OFC_PATH *path = (_OFC_PATH *) _path;
    OFC_INT port;

    port = path->port;

    return (port);
}

OFC_CORE_LIB OFC_VOID ofc_path_set_port(OFC_PATH *_path, OFC_INT port) {
    _OFC_PATH *path = (_OFC_PATH *) _path;

    path->port = port;
}

OFC_CORE_LIB OFC_VOID ofc_path_set_device(OFC_PATH *_path,
					  OFC_LPCTSTR device) {
    _OFC_PATH *path = (_OFC_PATH *) _path;

    if (path->device != OFC_NULL)
      ofc_free(path->device);
    path->device = ofc_tstrdup(device);
}

OFC_CORE_LIB OFC_LPCTSTR ofc_path_server(OFC_PATH *_path) {
    _OFC_PATH *path = (_OFC_PATH *) _path;
    OFC_LPCTSTR server;

    server = OFC_NULL;

    if (path->remote && path->num_dirs > 0)
        server = path->dir[0];

    return (server);
}

OFC_CORE_LIB OFC_VOID ofc_path_free_server(OFC_PATH *_path) {
    _OFC_PATH *path = (_OFC_PATH *) _path;

    if (path->remote && path->num_dirs > 0) {
        ofc_free(path->dir[0]);
        path->dir[0] = OFC_NULL;
    }
}

OFC_CORE_LIB OFC_VOID
ofc_path_set_server(OFC_PATH *_path, OFC_LPCTSTR server)
{
  _OFC_PATH *path = (_OFC_PATH *) _path;

  if (path->remote)
    {
      if (path->num_dirs > 0)
        {
          ofc_free(path->dir[0]);
        }
      else
        {
          path->num_dirs++;
          path->dir = ofc_realloc(path->dir, (path->num_dirs *
                                              sizeof(OFC_LPCSTR)));
        }
      path->dir[0] = ofc_tstrdup(server);
    }
}

OFC_CORE_LIB OFC_VOID
ofc_path_set_share(OFC_PATH *_path, OFC_LPCTSTR share) {
    _OFC_PATH *path = (_OFC_PATH *) _path;

    if (path->remote) {
        if (path->num_dirs > 1) {
            ofc_free(path->dir[1]);
        } else {
            path->num_dirs++;
            path->dir = ofc_realloc(path->dir, (path->num_dirs *
                                                sizeof(OFC_LPCSTR)));
        }
        path->dir[1] = ofc_tstrdup(share);
    }
}

OFC_CORE_LIB OFC_VOID
ofc_path_set_filename(OFC_PATH *_path, OFC_LPCTSTR filename) {
    _OFC_PATH *path = (_OFC_PATH *) _path;
    OFC_UINT prefix;

    if (path->remote)
        prefix = 2;
    else
        prefix = 0;

    if (path->num_dirs > prefix) {
        ofc_free(path->dir[path->num_dirs - 1]);
    } else {
        path->num_dirs++;
        path->dir = ofc_realloc(path->dir, (path->num_dirs *
                                            sizeof(OFC_LPCSTR)));
    }
    path->dir[path->num_dirs - 1] = ofc_tstrdup(filename);
}

OFC_CORE_LIB OFC_BOOL
ofc_path_server_cmp(OFC_PATH *_path, OFC_LPCTSTR server) {
    _OFC_PATH *path = (_OFC_PATH *) _path;
    OFC_BOOL ret;

    ret = OFC_FALSE;
    if (path->remote && path->num_dirs > 0 && path->dir[0] != OFC_NULL) {
        if (ofc_tstrcmp(path->dir[0], server) == 0)
            ret = OFC_TRUE;
    }
    return (ret);
}

OFC_CORE_LIB OFC_BOOL
ofc_path_port_cmp(OFC_PATH *_path, OFC_UINT16 port) {
    _OFC_PATH *path = (_OFC_PATH *) _path;
    OFC_BOOL ret;

    ret = OFC_FALSE;
    if (path->remote && (path->port == port))
        ret = OFC_TRUE;
    return (ret);
}

OFC_CORE_LIB OFC_LPCTSTR ofc_path_share(OFC_PATH *_path) {
    _OFC_PATH *path = (_OFC_PATH *) _path;
    OFC_LPCTSTR share;

    share = OFC_NULL;

    if (path->remote && path->num_dirs > 1)
        share = path->dir[1];

    return (share);
}

OFC_CORE_LIB OFC_BOOL
ofc_path_share_cmp(OFC_PATH *_path, OFC_LPCTSTR share) {
    _OFC_PATH *path = (_OFC_PATH *) _path;
    OFC_BOOL ret;

    ret = OFC_FALSE;
    if (path->remote && path->num_dirs > 1 && path->dir[1] != OFC_NULL) {
        if (ofc_tstrcmp(path->dir[1], share) == 0)
            ret = OFC_TRUE;
    }
    return (ret);
}

OFC_CORE_LIB OFC_LPCTSTR ofc_path_username(OFC_PATH *_path) {
    _OFC_PATH *path = (_OFC_PATH *) _path;
    OFC_LPCTSTR username;

    username = OFC_NULL;

    if (path->remote)
        username = path->username;

    return (username);
}

OFC_CORE_LIB OFC_VOID ofc_path_set_username(OFC_PATH *_path,
                                            OFC_LPCTSTR username) {
    _OFC_PATH *path = (_OFC_PATH *) _path;

    if (path->remote) {
        if (path->username != OFC_NULL)
            ofc_free(path->username);
        path->username = ofc_tstrdup(username);
    }
}

OFC_CORE_LIB OFC_BOOL
ofc_path_username_cmp(OFC_PATH *_path, OFC_LPCTSTR username) {
    _OFC_PATH *path = (_OFC_PATH *) _path;
    OFC_BOOL ret;

    ret = OFC_FALSE;
    if (path->remote) {
        if (path->username == OFC_NULL && username == OFC_NULL)
            ret = OFC_TRUE;
        else {
            if (ofc_tstrcmp(path->username, username) == 0)
                ret = OFC_TRUE;
        }
    }
    return (ret);
}

OFC_CORE_LIB OFC_LPCTSTR ofc_path_password(OFC_PATH *_path) {
    _OFC_PATH *path = (_OFC_PATH *) _path;
    OFC_LPCTSTR password;

    password = OFC_NULL;

    if (path->remote)
        password = path->password;

    return (password);
}

OFC_CORE_LIB OFC_VOID ofc_path_set_password(OFC_PATH *_path,
                                            OFC_LPCTSTR password) {
    _OFC_PATH *path = (_OFC_PATH *) _path;

    if (path->remote) {
        if (path->password != OFC_NULL)
            ofc_free(path->password);
        path->password = ofc_tstrdup(password);
    }
}

OFC_CORE_LIB OFC_BOOL
ofc_path_password_cmp(OFC_PATH *_path, OFC_LPCTSTR password) {
    _OFC_PATH *path = (_OFC_PATH *) _path;
    OFC_BOOL ret;

    ret = OFC_FALSE;
    if (path->remote) {
        if (path->password == OFC_NULL && password == OFC_NULL)
            ret = OFC_TRUE;
        else if (path->password != OFC_NULL && password != OFC_NULL) {
            if (ofc_tstrcmp(path->password, password) == 0)
                ret = OFC_TRUE;
        }
    }
    return (ret);
}

OFC_CORE_LIB OFC_LPCTSTR ofc_path_domain(OFC_PATH *_path) {
    _OFC_PATH *path = (_OFC_PATH *) _path;
    OFC_LPCTSTR domain;

    domain = OFC_NULL;

    if (path->remote)
        domain = path->domain;

    return (domain);
}

OFC_CORE_LIB OFC_VOID ofc_path_set_domain(OFC_PATH *_path,
                                          OFC_LPCTSTR domain) {
    _OFC_PATH *path = (_OFC_PATH *) _path;

    if (path->remote) {
        if (path->domain != OFC_NULL)
            ofc_free(path->domain);
        path->domain = ofc_tstrdup(domain);
    }
}

OFC_CORE_LIB OFC_LPCTSTR ofc_path_device(OFC_PATH *_path) {
    _OFC_PATH *path = (_OFC_PATH *) _path;
    return (path->device);
}

OFC_CORE_LIB OFC_VOID ofc_path_free_device(OFC_PATH *_path) {
    _OFC_PATH *path = (_OFC_PATH *) _path;
    ofc_free(path->device);
    path->device = OFC_NULL;
}

OFC_CORE_LIB OFC_VOID ofc_path_free_username(OFC_PATH *_path) {
    _OFC_PATH *path = (_OFC_PATH *) _path;
    ofc_free(path->username);
    path->username = OFC_NULL;
}

OFC_CORE_LIB OFC_VOID ofc_path_free_password(OFC_PATH *_path) {
    _OFC_PATH *path = (_OFC_PATH *) _path;
    ofc_free(path->password);
    path->password = OFC_NULL;
}

OFC_CORE_LIB OFC_VOID ofc_path_free_domain(OFC_PATH *_path) {
    _OFC_PATH *path = (_OFC_PATH *) _path;
    ofc_free(path->domain);
    path->domain = OFC_NULL;
}

OFC_CORE_LIB OFC_VOID
ofc_path_promote_dirs(OFC_PATH *_path, OFC_UINT num_dirs) {
    _OFC_PATH *path = (_OFC_PATH *) _path;
    OFC_UINT i;

    num_dirs = OFC_MIN (num_dirs, path->num_dirs);

    for (i = 0; i < num_dirs; i++) {
        ofc_free(path->dir[i]);
        path->dir[i] = OFC_NULL;
    }

    for (i = num_dirs; i < path->num_dirs; i++) {
        path->dir[i - num_dirs] = path->dir[i];
        path->dir[i] = OFC_NULL;
    }

    path->num_dirs -= num_dirs;
}

OFC_CORE_LIB OFC_VOID ofc_path_insert_dir(OFC_PATH *_path, OFC_UINT ix,
                                          OFC_CTCHAR *dir)
{
  _OFC_PATH *path = (_OFC_PATH *) _path;
  OFC_UINT i;

  path->num_dirs++;
  path->dir = ofc_realloc(path->dir, (path->num_dirs *
                                      sizeof(OFC_LPCSTR)));

  /*
   * Copy from the back
   */
  for (i = path->num_dirs-1; i > ix; i--)
    {
      path->dir[i] = path->dir[i-1];
    }

  path->dir[ix] = ofc_tstrdup(dir);
}  

OFC_CORE_LIB OFC_VOID ofc_path_set_local(OFC_PATH *_path) {
    _OFC_PATH *path = (_OFC_PATH *) _path;
    path->remote = OFC_FALSE;
}

OFC_CORE_LIB OFC_VOID ofc_path_set_remote(OFC_PATH *_path) {
    _OFC_PATH *path = (_OFC_PATH *) _path;
    path->remote = OFC_TRUE;
}

OFC_CORE_LIB OFC_VOID ofc_path_set_relative(OFC_PATH *_path) {
    _OFC_PATH *path = (_OFC_PATH *) _path;
    path->absolute = OFC_FALSE;
}

OFC_CORE_LIB OFC_VOID ofc_path_set_absolute(OFC_PATH *_path) {
    _OFC_PATH *path = (_OFC_PATH *) _path;
    path->absolute = OFC_TRUE;
}

OFC_CORE_LIB OFC_BOOL ofc_path_absolute(OFC_PATH *_path) {
    _OFC_PATH *path = (_OFC_PATH *) _path;
    return (path->absolute);
}

OFC_CORE_LIB OFC_VOID ofc_path_set_type(OFC_PATH *_path, OFC_FST_TYPE fstype) {
    _OFC_PATH *path = (_OFC_PATH *) _path;
    path->type = fstype;
}

OFC_CORE_LIB OFC_LPCTSTR ofc_path_filename(OFC_PATH *_path) {
    _OFC_PATH *path = (_OFC_PATH *) _path;
    OFC_LPCTSTR filename;

    filename = OFC_NULL;

    if (path->remote) {
        if (path->num_dirs > 2)
            filename = path->dir[path->num_dirs - 1];
    } else {
        if (path->num_dirs > 0)
            filename = path->dir[path->num_dirs - 1];
    }
    return (filename);
}

OFC_CORE_LIB OFC_VOID ofc_path_free_filename(OFC_PATH *_path) {
    _OFC_PATH *path = (_OFC_PATH *) _path;
    if (path->remote) {
        if (path->num_dirs > 2) {
            ofc_free(path->dir[path->num_dirs - 1]);
            path->dir[path->num_dirs - 1] = OFC_NULL;
            path->num_dirs--;
        }
    } else {
        if (path->num_dirs > 0) {
            ofc_free(path->dir[path->num_dirs - 1]);
            path->dir[path->num_dirs - 1] = OFC_NULL;
            path->num_dirs--;
        }
    }
}

OFC_CORE_LIB OFC_INT ofc_path_num_dirs(OFC_PATH *_path) {
    _OFC_PATH *path = (_OFC_PATH *) _path;
    OFC_INT ret;

    ret = 0;
    if (path->remote) {
        if (path->num_dirs > 2)
            ret = path->num_dirs - 2;
    } else
        ret = path->num_dirs;

    return (ret);
}

OFC_CORE_LIB OFC_LPCTSTR ofc_path_dir(OFC_PATH *_path, OFC_UINT ix) {
    _OFC_PATH *path = (_OFC_PATH *) _path;
    OFC_LPCTSTR ret;
    OFC_UINT jx;

    ret = OFC_NULL;
    if (path->remote) {
        jx = path->num_dirs - 2;
        if (jx > 0 && ix < jx)
            ret = path->dir[ix + 2];
    } else {
        if (ix < path->num_dirs)
            ret = path->dir[ix];
    }
    return (ret);
}

OFC_CORE_LIB OFC_VOID ofc_path_free_dirs(OFC_PATH *_path) {
    _OFC_PATH *path = (_OFC_PATH *) _path;
    OFC_UINT jx;
    OFC_UINT ix;

    if (path->remote) {
        jx = 1;
    } else {
        jx = 0;
    }

    for (ix = jx; ix < path->num_dirs; ix++) {
        ofc_free(path->dir[ix]);
        path->dir[ix] = OFC_NULL;
    }

    if (path->num_dirs > jx)
        path->num_dirs = jx;

    if (path->num_dirs == 0) {
        ofc_free(path->dir);
        path->dir = OFC_NULL;
    }
}

OFC_VOID ofc_path_debug(OFC_PATH *_path) {
    _OFC_PATH *path = (_OFC_PATH *) _path;
    OFC_UINT i;

    ofc_log(OFC_LOG_INFO, "Path:\n");
    ofc_log(OFC_LOG_INFO, "  Type: %d\n", path->type);
    ofc_log(OFC_LOG_INFO, "  Device: %S\n",
               path->device == OFC_NULL ? TSTR("NULL") : path->device);
    ofc_log(OFC_LOG_INFO, "  Username: %S\n",
               path->username == OFC_NULL ? TSTR("NULL") : path->username);
    ofc_log(OFC_LOG_INFO, "  Password: %S\n",
               path->password == OFC_NULL ? TSTR("NULL") : path->password);
    ofc_log(OFC_LOG_INFO, "  Domain: %S\n",
               path->domain == OFC_NULL ? TSTR("NULL") : path->domain);
    ofc_log(OFC_LOG_INFO, "  Num Dirs: %d\n", path->num_dirs);
    for (i = 0; i < path->num_dirs; i++)
        ofc_log(OFC_LOG_INFO, "    %d: %S\n", i, path->dir[i]);

    ofc_log(OFC_LOG_INFO, "  Absolute: %s\n", path->absolute ? "yes" : "no");
    ofc_log(OFC_LOG_INFO, "  Remote: %s\n", path->remote ? "yes" : "no");
}

static OFC_HANDLE hWorkgroups;

OFC_CORE_LIB OFC_VOID init_workgroups(OFC_VOID) {
    hWorkgroups = ofc_queue_create();
}

OFC_CORE_LIB OFC_VOID destroy_workgroups(OFC_VOID) {
    OFC_LPTSTR pWorkgroup;

    for (pWorkgroup = ofc_queue_first(hWorkgroups);
         pWorkgroup != OFC_NULL;
         pWorkgroup = ofc_queue_first(hWorkgroups))
      remove_workgroup(pWorkgroup);

    ofc_queue_destroy(hWorkgroups);

    hWorkgroups = OFC_HANDLE_NULL;
}

static OFC_LPTSTR FindWorkgroup(OFC_LPCTSTR workgroup) {
    OFC_LPTSTR pWorkgroup;

    for (pWorkgroup = ofc_queue_first(hWorkgroups);
         pWorkgroup != OFC_NULL && ofc_tstrcmp(pWorkgroup, workgroup) != 0;
         pWorkgroup = ofc_queue_next(hWorkgroups, pWorkgroup));

    return (pWorkgroup);
}

OFC_CORE_LIB OFC_VOID update_workgroup(OFC_LPCTSTR workgroup) {
    OFC_LPTSTR pWorkgroup;

    ofc_lock(lockPath);
    pWorkgroup = FindWorkgroup(workgroup);

    if (pWorkgroup == OFC_NULL) {
        pWorkgroup = ofc_tstrndup(workgroup, OFC_MAX_PATH);
        ofc_enqueue(hWorkgroups, pWorkgroup);
    }
    ofc_unlock(lockPath);
}

OFC_CORE_LIB OFC_VOID remove_workgroup(OFC_LPCTSTR workgroup) {
    OFC_LPTSTR pWorkgroup;

    ofc_lock(lockPath);
    pWorkgroup = FindWorkgroup(workgroup);

    if (pWorkgroup != OFC_NULL) {
        ofc_queue_unlink(hWorkgroups, pWorkgroup);
        ofc_free(pWorkgroup);
    }
    ofc_unlock(lockPath);
}

OFC_CORE_LIB OFC_BOOL lookup_workgroup(OFC_LPCTSTR workgroup) {
    OFC_BOOL ret;
    OFC_LPTSTR pWorkgroup;

    ofc_lock(lockPath);
    pWorkgroup = FindWorkgroup(workgroup);

    ret = OFC_FALSE;
    if (pWorkgroup != OFC_NULL)
        ret = OFC_TRUE;

    ofc_unlock(lockPath);
    return (ret);
}

