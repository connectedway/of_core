/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#define __OFC_CORE_DLL__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/libc.h"
#include "ofc/time.h"
#include "ofc/perf.h"
#include "ofc/event.h"
#include "ofc/impl/timeimpl.h"

#if defined(OFC_64BIT_INTEGER)

OFC_VOID epoch_time_to_file_time(const OFC_ULONG tv_sec,
                                 const OFC_ULONG tv_nsec,
                                 OFC_FILETIME *filetime) {
    OFC_UINT64 secs;
    OFC_UINT64 ns;

    secs = (OFC_UINT64) tv_sec + OFC_TIME_S_EPOCH_OFFSET_1900;

    /*
     * Convert to number of 100 ns
     */
    ns = secs * 1000 * 1000 * 10 + (tv_nsec / 100);
    /*
     * Name into 100ns chunks
     */
    filetime->dwHighDateTime = (OFC_DWORD) (ns >> 32);
    filetime->dwLowDateTime = (OFC_DWORD) (ns & 0xFFFFFFFF);
}

OFC_VOID file_time_to_epoch_time(const OFC_FILETIME *filetime,
                                 OFC_ULONG *tv_sec,
                                 OFC_ULONG *tv_nsec) {
    OFC_UINT64 ns;

    /*
     * Name into 100ns chunks
     */
    ns = (OFC_UINT64) filetime->dwHighDateTime << 32 | filetime->dwLowDateTime;
    /*
     * Convert to number of secs
     */
    *tv_sec = (OFC_ULONG) ((ns / (1000 * 1000 * 10)) -
                           OFC_TIME_S_EPOCH_OFFSET_1900);
    *tv_nsec = (ns % (1000 * 1000 * 10)) * 100;
}

#else
#include "int64.h"

#if 1
/*
 * This brings us to 1900
 */
static OFC_UINT64 ofc_64_epoch_offset =
  {
    0xB6109100,
    0x00000002
  } ;
#else
/*
 * This brings us to 1970
 */
static OFC_UINT64 ofc_64_epoch_offset =
  {
    0x32661280
    0x00000002
  } ;
#endif

OFC_VOID EpochTimeToFileTime (const OFC_ULONG tv_sec,
                   const OFC_ULONG tv_nsec,
                   OFC_FILETIME *filetime)
{
  OFC_UINT64 ns ;
  OFC_UINT64 temp ;

  ofc_u64_assign_u32 (&ns, tv_sec) ;
  ofc_u64_add(&ns, &ofc_64_epoch_offset) ;
  /*
   * Convert to number of 100 ns
   */
  ofc_u64_mult_u32(&ns, 1000 * 1000 * 10) ;

  ofc_u64_assisgn_u32(&temp, tv_nsec / 100) ;
  ofc_u64_add(&ns, &temp) ;
  /*
   * Name into 100ns chunks
   */
  filetime->dwHighDateTime = ofc_u64_retrieve_high (&ns) ;
  filetime->dwLowDateTime = ofc_u64_retrieve_low (&ns) ;
}

OFC_VOID file_time_to_epoch_time (const OFC_FILETIME *filetime,
                   OFC_ULONG *tv_sec,
                   OFC_ULONG *tv_nsec)
{
  OFC_UINT64 secs ;
  OFC_UINT64 ns ;
  OFC_UINT64 temp ;
  OFC_UINT64 quot ;
  OFC_UINT64 rem ;

  /*
   * Name into 100ns chunks
   */
  ofc_u64_assign_u64(&ns,
          filetime->dwLowDateTime, filetime->dwHighDateTime) ;
  /*
   * Convert to number of secs
   */
  ofc_u64_assign(&secs, &ns) ;
  ofc_u64_assign_u32(&temp, 1000 * 1000 * 10) ;
  ofc_u64_divide(&secs, &temp, &quot, &rem) ;
  ofc_u64_sub(&quot, &ofc_64_epoch_offset) ;
  *tv_sec = ofc_u64_retrieve_low (&quot) ;
  *tv_nsec = ofc_u64_retrieve_low (&rem) * 100 ;
}
#endif

OFC_CORE_LIB OFC_MSTIME
ofc_time_get_now(OFC_VOID) {
    return (ofc_time_get_now_impl());
}

OFC_CORE_LIB OFC_VOID
ofc_time_get_file_time(OFC_FILETIME *filetime) {
    ofc_time_get_file_time_impl(filetime);
}

OFC_CORE_LIB OFC_UINT16
ofc_time_get_time_zone(OFC_VOID) {
    return (ofc_time_get_timezone_impl());
}

OFC_CORE_LIB OFC_BOOL
ofc_file_time_to_dos_date_time(const OFC_FILETIME *lpFileTime,
                               OFC_WORD *lpFatDate, OFC_WORD *lpFatTime) {
    return (ofc_file_time_to_dos_date_time_impl(lpFileTime, lpFatDate, lpFatTime));
}

OFC_CORE_LIB OFC_BOOL
ofc_dos_date_time_to_file_time(OFC_WORD FatDate, OFC_WORD FatTime,
                               OFC_FILETIME *lpFileTime) {
    return (ofc_dos_date_time_to_file_time_impl(FatDate, FatTime, lpFileTime));
}

OFC_CORE_LIB OFC_VOID
ofc_time_elements_to_dos_date_time(OFC_UINT16 month,
                                   OFC_UINT16 day,
                                   OFC_UINT16 year,
                                   OFC_UINT16 hour,
                                   OFC_UINT16 min,
                                   OFC_UINT16 sec,
                                   OFC_WORD *lpFatDate,
                                   OFC_WORD *lpFatTime) {
    *lpFatDate =
            month << OFC_DOS_MONTH_SHIFT |
            day << OFC_DOS_DAY_SHIFT |
            (year - OFC_DOS_YEAR_BASE) << OFC_DOS_YEAR_SHIFT;
    *lpFatTime =
            hour << OFC_DOS_HRS_SHIFT |
            min << OFC_DOS_MINS_SHIFT |
            sec << OFC_DOS_SECS_SHIFT;
}

OFC_CORE_LIB OFC_VOID
ofc_dos_date_time_to_elements(OFC_WORD FatDate,
                              OFC_WORD FatTime,
                              OFC_UINT16 *month,
                              OFC_UINT16 *day,
                              OFC_UINT16 *year,
                              OFC_UINT16 *hour,
                              OFC_UINT16 *min,
                              OFC_UINT16 *sec) {
    *hour = (FatTime & OFC_DOS_HRS) >> OFC_DOS_HRS_SHIFT;
    *min = (FatTime & OFC_DOS_MINS) >> OFC_DOS_MINS_SHIFT;
    *sec = (FatTime & OFC_DOS_SECS) >> OFC_DOS_SECS_SHIFT;

    *month = ((FatDate & OFC_DOS_MONTH) >> OFC_DOS_MONTH_SHIFT);
    *day = (FatDate & OFC_DOS_DAY) >> OFC_DOS_DAY_SHIFT;
    *year = ((FatDate & OFC_DOS_YEAR) >> OFC_DOS_YEAR_SHIFT) +
            OFC_DOS_YEAR_BASE;
}

OFC_CORE_LIB OFC_MSTIME
ofc_get_runtime(OFC_VOID) {
    return (ofc_get_runtime_impl());
}

