/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__OFC_TYPES_H__)
#define __OFC_TYPES_H__

#include "ofc/config.h"

/**
 * \{
 * \defgroup types Global Types Used by all Components
 *
 * Open Files is designed as a highly portable subsystem.  As such, it is
 * necessary to define all types we use consistently throughout the product.
 * Any platform generic code should use only types defined within this 
 * group.  APIs to all Open Files components will use types either defined
 * in this group, or in some other facility dependent on this group.
 *
 * When porting Open Files to a new platform, some attention should be
 * paid to the definition of these types.  In most cases, no modification
 * to any of these definitions is necessary.  
 *
 * Type | Description
 * -----|------------
 * \ref OFC_SIZET | Represents a size
 * \ref OFC_INT | Represents a generic integer
 * \ref OFC_UINT | Represents a generic unsigned integer
 * \ref OFC_INT32 | Represents a 32 bit integer
 * \ref OFC_INT16 | Represents a 16 bit integer
 * \ref OFC_INT8 | Represents an 8 bit integer
 * \ref OFC_CHAR | Represents a normal character
 * \ref OFC_BYTE | Represents a byte
 * \ref OFC_LPBYTE | Pointer to a byte string 
 * \ref OFC_UCHAR | Represents an unsigned character
 * \ref OFC_VOID | Represents a null type
 * \ref OFC_LONG | Represents a generic long integer
 * \ref OFC_PLONG | Pointer to a generic long integer
 * \ref OFC_SHORT | Pointer to a generic short integer
 * \ref OFC_UINT32 | Represents a 32 bit unsigned
 * \ref OFC_UINT16 | Represents a 16 bit unsigned
 * \ref OFC_UINT8 | Represents a 8 bit unsigned
 * \ref OFC_UINT64 | Represents a 64 bit unsigned
 * \ref OFC_INT64 | Represents a 64 bit signed
 * \ref OFC_LARGE_INTEGER | Represents a generic large (i.e. 64 bit) Integer
 * \ref OFC_LARGE_INTEGER_HIGH | Returns high 32 bits of Large Integer
 * \ref OFC_LARGE_INTEGER_LOW | Returns Low 32 bits of Large Integer
 * \ref OFC_LARGE_INTEGER_ASSIGN | Assign low and high portions of Large Integer
 * \ref OFC_LARGE_INTEGER_SET | Set a large integer
 * \ref OFC_LARGE_INTEGER_EQUAL | Compare large integers
 * \ref OFC_LARGE_INTEGER_INCR | Increment a large integer
 * \ref OFC_LARGE_INTEGER_AND | Binary AND of Large Integers
 * \ref OFC_LARGE_INTEGER_INIT | Initialize a large integer
 * \ref OFC_OFFT | Represents a file offset
 * \ref OFC_ULONG | Represents a generic unsigned
 * \ref OFC_ULONG_MAX | Maximum Unsigned Long
 * \ref OFC_LONG_MAX | Maximum Signed
 * \ref OFC_LONG_MIN | Minimum Long
 * \ref OFC_LPULONG | Pointer to a ULONG
 * \ref OFC_ULONG_PTR | Pointer to a ULONG
 * \ref OFC_CVOID | Untyped constant value
 * \ref OFC_LPWSTR | Pointer to wide character string
 * \ref OFC_LMSTR | Pointer to wide character string
 * \ref OFC_CWCHAR | Represents a constant wide character
 * \ref OFC_LPCWSTR | Pointer to constant wide character string
 * \ref OFC_CCHAR | Constant character
 * \ref OFC_LPSTR | Pointer to a character string
 * \ref OFC_LPCSTR | Pointer to a constant character string
 * \ref OFC_WORD | Represents a Generic Word
 * \ref OFC_DWORD | Represents a Generic Double Word
 * \ref OFC_DWORD_MASK | Mask for a double word
 * \ref OFC_LPDWORD | Pointer to a DWORD
 * \ref OFC_DWORD_PTR | Pointer to a DWORD
 * \ref OFC_LPVOID | Pointer to a void type
 * \ref OFC_PVOID | Pointer to a void type
 * \ref OFC_LPCVOID | Pointer to a constant void type
 * \ref OFC_TCHAR | A wide character
 * \ref OFC_CTCHAR | A constant wide character
 * \ref OFC_TACHAR | Default character (wide or normal)
 * \ref OFC_CTACHAR | Default constant character (wide or normal)
 * \ref OFC_LPTSTR | Pointer to wide character string
 * \ref OFC_LPTASTR | Pointer to default character string (wide or normal)
 * \ref OFC_LPCTSTR | Pointer to const wide character string
 * \ref OFC_LPCTASTR | Pointer to default const character string (wide or normal)
 * \ref OFC_MSTIME | Millisecond Time
 * \ref TCHAR_BACKSLASH | Wide backslash
 * \ref TCHAR_SLASH | Wide Forward Slash
 * \ref TCHAR_COLON | Wide colon
 * \ref TCHAR_EOS | Wide end of string
 * \ref TCHAR_AMP | Wide apersand
 * \ref TSTR | Form a wide character string
 * \ref TCHAR | Form a wide character
 * \ref TACHAR_BACKSLASH | Default backslash
 * \ref TACHAR_SLASH | Default forward slash
 * \ref TACHAR_COLON | Default colon
 * \ref TACHAR_EOS | Default EOS
 * \ref TACHAR_AMP | Default Ampersand
 * \ref TASTR | Form a default string
 * \ref TACHAR | Form a default char
 * \ref OFC_UUID_LEN | Length of a UUID
 * \ref OFC_UUID | Representation of a UUID
 * \ref OFC_BOOL | Boolean
 * \ref OFC_FALSE | False Value
 * \ref OFC_TRUE | True Value
 * \ref OFC_LOG_LEVEL | Level of Log Message
 * \ref OFC_NULL | Null Pointer
 * \ref OFC_IOVEC | Vectord Buffer
 * \ref container_of | Get offset of member
 */

/**
 * Represents the size of a string or structure
 */
typedef long int OFC_SIZET;
/**
 * Represents a generic platform default integer
 */
typedef int OFC_INT;
/**
 * Represents a generic platform default unsigned integer
 */
typedef unsigned int OFC_UINT;
/**
 * Represents a 32 bit signed integer
 */
#if defined(OFC_LONGINT_64)
typedef int OFC_INT32;
#else
typedef long int OFC_INT32 ;
#endif
/**
 * Represents a 16 bit signed integer
 */
typedef short int OFC_INT16;
/**
 * Represents an 8 bit signed integer
 */
typedef char OFC_INT8;
/**
 * Represents a character byte
 */
typedef char OFC_CHAR;
/**
 * Another representation of a character byte
 */
typedef char OFC_BYTE;
/**
 * A pointer to a character byte array
 */
typedef OFC_BYTE *OFC_LPBYTE;
/**
 * Represents an unsigned 8 bit character
 */
typedef unsigned char OFC_UCHAR;
/**
 * Represents a generic VOID 
 */
typedef void OFC_VOID;
/**
 * Represents a generic long value
 */
typedef long OFC_LONG;
/**
 * Represents a pointer to a generic long value
 */
typedef OFC_LONG *OFC_PLONG;
/**
 * Represents a generic short value
 */
typedef short OFC_SHORT;
/**
 * Represents a 32 bit unsigned integer
 */
#if defined(OFC_LONGINT_64)
typedef unsigned int OFC_UINT32;
#else
typedef unsigned long int OFC_UINT32 ;
#endif
/**
 * Represents a 16 bit unsigned integer
 */
typedef unsigned short int OFC_UINT16;
/**
 * Represents an 8 bit unsigned integer
 */
typedef unsigned char OFC_UINT8;

/**
 * Represents an unsigned 64 bit value
 */
#if defined(OFC_64BIT_INTEGER)
typedef unsigned long long int OFC_UINT64;
#else
typedef struct
{
  /**
   * The low order 32 bits
   */
  OFC_UINT32 low ;
  /**
   * The high order 32 bits
   */
  OFC_UINT32 high ;
} OFC_UINT64 ;
#endif
/**
 * Represents a signed 64 bit value
 */
#if defined(OFC_64BIT_INTEGER)
typedef long long int OFC_INT64;
#else
typedef struct
{
  /**
   * The low order 32 bits
   */
  OFC_UINT32 low ;
  /**
   * The high order 32 bits
   */
  OFC_INT32 high ;
} OFC_INT64 ;
#endif

/**
 * Represents a generic large integer
 */
typedef OFC_INT64 OFC_LARGE_INTEGER;

/**
 * Return the high order 32 bits of a large integer
 * 
 * \param x
 * A 64 bit integer that we wish to get the high order from
 */
#if defined(OFC_64BIT_INTEGER)
#define OFC_LARGE_INTEGER_HIGH(x) ((OFC_UINT32)((x)>>32))
#else
#define OFC_LARGE_INTEGER_HIGH(x) ((x).high)
#endif

/**
 * Return the low order 32 bits of a large integer
 * 
 * \param x
 * A 64 bit integer that we wish to get the low order from
 */
#if defined(OFC_64BIT_INTEGER)
#define OFC_LARGE_INTEGER_LOW(x) ((OFC_UINT32)((x)&0xFFFFFFFF))
#else
#define OFC_LARGE_INTEGER_LOW(x) ((x).low)
#endif

/**
 * Assign one large integer to another
 *
 * \param x
 * 64 bit integer to receive assignment
 *
 * \param y
 * 64 bit integer to assign
 */
#if defined(OFC_64BIT_INTEGER)
#define OFC_LARGE_INTEGER_ASSIGN(x, y) x=y
#else
#define OFC_LARGE_INTEGER_ASSIGN(x,y) ofc_memcpy(&x,&y, sizeof(OFC_LARGE_INTEGER))
#endif

/**
 * Initialize a large integer from two 32 bit integers
 *
 * \param x
 * A 64 bit integer to receive assignment
 *
 * \param y
 * A 32 bit integer to assign to the low order 32 bits
 *
 * \param z
 * A 32 bit integer to assign to the high order 32 bits
 */
#if defined(OFC_64BIT_INTEGER)
#define OFC_LARGE_INTEGER_SET(x, y, z) (x)=(OFC_LARGE_INTEGER)(z)<<32|(y)
#else
#define OFC_LARGE_INTEGER_SET(x, y, z) {(x).low = (y); (x).high=(z);}
#endif

/**
 * Compare two large integers
 *
 * \param x
 * First 64 bit integer to compare
 *
 * \param y
 * Second 64 bit integer to compare
 *
 * \returns
 * OFC_TRUE if equal, OFC_FALSE otherwise
 */
#if defined(OFC_64BIT_INTEGER)
#define OFC_LARGE_INTEGER_EQUAL(x, y) (x==y)
#else
#define OFC_LARGE_INTEGER_EQUAL(x,y) ((x).high==(y).high && (x).low==(y).low)
#endif

/**
 * Increment a 64 bit integer
 *
 * This insure carry if using two 32 bit values to represent a 64 bit value
 *
 * \param x
 * 64 bit value to increment
 */
#if defined(OFC_64BIT_INTEGER)
#define OFC_LARGE_INTEGER_INCR(x) x++
#else
#define OFC_LARGE_INTEGER_INCR(x) ((x).low==0xFFFFFFFF?\
                    (x).high++,(x).low=0:(x).low++)
#endif

/**
 * Perform a binary AND between two 64 bit values
 *
 * \param x
 * Result of AND and first 64 bit value
 *
 * \param y
 * low order 32 bit values to AND with
 *
 * \param z
 * high order 32 bit values to AND with
 */
#if defined(OFC_64BIT_INTEGER)
#define OFC_LARGE_INTEGER_AND(x, y, z) (x)&=((OFC_LARGE_INTEGER)(z)<<32|(y))
#else
#define OFC_LARGE_INTEGER_AND(x, y, z) {(x).low &= (y); (x).high&=(z);}
#endif

/**
 * Initialize one 64 bit value from two 32 bit values
 *
 * \param x
 * Low order 32 bit values to initialize with
 *
 * \param y
 * High order 32 bit values to initialize with
 *
 * \returns
 * 64 bit value
 */
#if defined(OFC_64BIT_INTEGER)
#define OFC_LARGE_INTEGER_INIT(x, y) (OFC_LARGE_INTEGER)(y)<<32|(x)
#else
#define OFC_LARGE_INTEGER_INIT(x,y) {x; y}
#endif

/**
 * Represents an offset into a structure
 */
typedef OFC_LARGE_INTEGER OFC_OFFT;
/**
 * Represents a generic unsigned long value
 */
typedef unsigned long OFC_ULONG;
/**
 * Represents the maximum unsigned long value
 */
#define OFC_ULONG_MAX ((OFC_ULONG)(~0L))
/**
 * Represents the maximum positive long value
 */
#define OFC_LONG_MAX ((OFC_LONG)(OFC_ULONG_MAX >> 1))
/**
 * Represents the minimum negative long value
 */
#define OFC_LONG_MIN ((OFC_LONG)(~OFC_LONG_MAX))
/**
 * Represents a pointer to an Unsigned Long
 */
typedef OFC_ULONG *OFC_LPULONG;

/**
 * Represents a pointer to a 64 bit value
 */
#if defined(OFC_64BIT_POINTER)
typedef OFC_UINT64 OFC_ULONG_PTR;
#else
typedef OFC_UINT32 OFC_ULONG_PTR ;
#endif

/**
 * Represents a void constant
 */
typedef const OFC_VOID OFC_CVOID;
/**
 * Represents a pointer to a wide character string
 */
typedef OFC_WCHAR *OFC_LPWSTR;
/**
 * Another representation of a pointer to a wide character string
 */
typedef OFC_WCHAR *OFC_LMSTR;
/**
 * Represents a constant wide character
 */
typedef const OFC_WCHAR OFC_CWCHAR;
/**
 * Represents a pointer to a constant wide character string
 */
typedef const OFC_WCHAR *OFC_LPCWSTR;
/**
 * Represents a constant character
 */
typedef const OFC_CHAR OFC_CCHAR;
/**
 * Represents a pointer to a character string
 */
typedef OFC_CHAR *OFC_LPSTR;
/**
 * Represents a pointer to a constant character string
 */
typedef const OFC_CHAR *OFC_LPCSTR;
/**
 * Represents a Generic WORD
 */
typedef OFC_UINT16 OFC_WORD;
/**
 * Represents a generic double word
 */
typedef OFC_UINT32 OFC_DWORD;
/**
 * The mask to associate with a double word
 */
#define OFC_DWORD_MASK 0xFFFFFFFF
/**
 * Represents a pointer to an array of double words
 */
typedef OFC_DWORD *OFC_LPDWORD;
/**
 * Represents a pointer to a double word
 */
#if defined(OFC_64BIT_POINTER)
typedef OFC_UINT64 OFC_DWORD_PTR;
#else
typedef OFC_UINT32 OFC_DWORD_PTR ;
#endif
/**
 * Represents a pointer to a void element
 */
typedef OFC_VOID *OFC_LPVOID;
/**
 * Another representation of a pointer to a void element
 */
typedef OFC_VOID *OFC_PVOID;
/**
 * Represents a pointer to a constant element
 */
typedef const OFC_VOID *OFC_LPCVOID;

/**
 * Represents a wide character
 */
typedef OFC_WCHAR OFC_TCHAR;
/**
 * Represents a wide character constant
 */
typedef const OFC_WCHAR OFC_CTCHAR;

/**
 * Represents a wide character
 */
#if defined(OFC_UNICODE_API)
typedef OFC_TCHAR OFC_TACHAR;
#else
typedef OFC_CHAR OFC_TACHAR ;
#endif

/**
 * Represents a wide character constant
 */
#if defined(OFC_UNICODE_API)
typedef const OFC_TCHAR OFC_CTACHAR;
#else
typedef const OFC_CHAR OFC_CTACHAR ;
#endif

/**
 * Represents a Pointer to a wide character string
 */
typedef OFC_TCHAR *OFC_LPTSTR;
/**
 * Represents a Pointer to a default API wide character
 */
typedef OFC_TACHAR *OFC_LPTASTR;
/**
 * Represents a pointer to a wide character constant string
 */
typedef const OFC_TCHAR *OFC_LPCTSTR;
/**
 * Represents a pointer to a default API wide character constant string
 */
typedef const OFC_TACHAR *OFC_LPCTASTR;
/**
 * Represents a Millisecond time value
 */
typedef OFC_INT32 OFC_MSTIME;

/**
 * A wide character backslash
 */
#define TCHAR_BACKSLASH L'\\'
/**
 * A wide character slash
 */
#define TCHAR_SLASH L'/'
/**
 * A wide character colon
 */
#define TCHAR_COLON L':'
/**
 * A wide character end of string
 */
#define TCHAR_EOS L'\0'
/**
 * A wide character ampersand
 */
#define TCHAR_AMP L'@'
/**
 * A wide character String
 *
 * \param x
 * String to convert to wide character
 */
#define TSTR(x) (const OFC_TCHAR *) L##x
/**
 * A wide character
 * 
 * \param x
 * Character to convert to wide
 */
#define TCHAR(x) (const OFC_TCHAR) L##x

/**
 * A default API wide character backslash
 */
#if defined(OFC_UNICODE_API)
#define TACHAR_BACKSLASH TCHAR('\\')
#else
/**
 * A wide character backslash
 */
#define TACHAR_BACKSLASH '\\'
#endif

/**
 * A default API wide character slash
 */
#if defined(OFC_UNICODE_API)
#define TACHAR_SLASH TCHAR('/')
#else
#define TACHAR_SLASH '/'
#endif

/**
 * A default API wide character colon
 */
#if defined(OFC_UNICODE_API)
#define TACHAR_COLON TCHAR(':')
#else
#define TACHAR_COLON ':'
#endif

/**
 * A default API wide character end of string
 */
#if defined(OFC_UNICODE_API)
#define TACHAR_EOS TCHAR('\0')
#else
#define TACHAR_EOS '\0'
#endif

/**
 * A default API wide character ampersand
 */
#if defined(OFC_UNICODE_API)
#define TACHAR_AMP TCHAR('@')
#else
#define TACHAR_AMP '@'
#endif

/**
 * A default API wide character String
 */
#if defined(OFC_UNICODE_API)
#define TASTR(x) TSTR(x)
#else
#define TASTR(x) x
#endif
  
/**
 * A default API wide character
 */
#if defined(OFC_UNICODE_API)
#define TACHAR(x) TCHAR(x)
#else
#define TACHAR(x) x
#endif

/**
 * The length of a UUID
 */
#define OFC_UUID_LEN 16
/**
 * Represents a UUID
 */
typedef OFC_UCHAR OFC_UUID[OFC_UUID_LEN];

/**
 * Represents a Boolean value
 *
 * This should be a 8 bit value on a windows system to be compatable with
 * Windows BOOL
 */
typedef OFC_UINT8 OFC_BOOL;
/**
 * Values of a Boolean
 */
enum {
    /**
     * The value is false
     */
    OFC_FALSE = 0,
    /**
     * The value is true
     */
    OFC_TRUE = 1
};

/**
 * Log Level
 *
 * Values that represent the criticality of a log message
 * and the threshold of logging on the system
 */
typedef enum
  {
    OFC_LOG_FATAL = 0,		/*!< Fatal error  */
    OFC_LOG_WARN = 1,		/*!< Warning  */
    OFC_LOG_INFO = 2,		/*!< Info Level  */
    OFC_LOG_DEBUG = 3,		/*!< Debug Only  */
  } OFC_LOG_LEVEL;

/**
 * A NULL Pointer
 */
#define OFC_NULL ((OFC_LPVOID) 0)
/**
 * An IO Vector used by SASL
 */
typedef struct _iovec {
    OFC_LONG iov_len;		/*!< The Length of the region  */
    OFC_CHAR *iov_base;		/*!< The base o the region  */
} OFC_IOVEC;

/**
 * Container of macro
 * 
 * \param ptr
 * Pointer to field within structure
 *
 * \param type
 * Type of structure
 *
 * \param member
 * member name withing structure
 *
 * \returns
 * offset to member field within structure
 */
#define container_of(ptr, type, member) \
  (type *)((OFC_CHAR *)(ptr)-(OFC_CHAR *)&((type *)0)->member)

/** \} */
#endif

