/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#define __OFC_CORE_DLL__

#include "ofc/core.h"
#include "ofc/config.h"
#include "ofc/types.h"
#include "ofc/libc.h"
#include "ofc/lock.h"
#include "ofc/console.h"
#include "ofc/thread.h"
#include "ofc/process.h"
#include "ofc/heap.h"
#include "ofc/backtrace.h"
#include "ofc/impl/heapimpl.h"

#undef BACKTRACE_SYM

/*
 * Soft ceiling on openfiles-tracked allocations.  When Total exceeds this,
 * ofc_heap_malloc_acct calls ofc_heap_dump() and ofc_process_crash() so we
 * catch a runaway leak well before system-wide OOM makes the process
 * unkillable.  Set to 0 to disable.  Hardcoded for now — bump if the
 * legitimate working set turns out to be bigger.
 */
#define OFC_HEAP_LIMIT_BYTES (128u * 1024u * 1024u)

struct heap_chunk {
    OFC_SIZET alloc_size;
#if defined(OFC_HEAP_DEBUG)
    struct heap_chunk *dbgnext;
    struct heap_chunk *dbgprev;
    OFC_BOOL snap;

#if (defined(__GNUC__) || defined(__clang__)) && defined(OFC_STACK_TRACE)
#if defined(BACKTRACE_SYM)
    OFC_CHAR caller1[120];
    OFC_CHAR caller2[120];
    OFC_CHAR caller3[120];
    OFC_CHAR caller4[120];
#else
    OFC_VOID *caller1;
    OFC_VOID *caller2;
    OFC_VOID *caller3;
    OFC_VOID *caller4;
#endif
#else
    OFC_VOID *caller ;
#endif
#endif
};

typedef struct {
    OFC_LOCK lock;
    OFC_UINT32 Max;
    OFC_UINT32 Total;
#if defined(OFC_HEAP_DEBUG)
    struct heap_chunk *Allocated;
#endif
} OFC_HEAP_STATS;

static OFC_HEAP_STATS ofc_heap_stats = {0};

static OFC_VOID
ofc_heap_malloc_acct(OFC_SIZET size, struct heap_chunk *chunk) {
    static OFC_BOOL crashing = OFC_FALSE;
    OFC_BOOL exceeded = OFC_FALSE;

    /*
     * Put on the allocation queue
     */
    chunk->alloc_size = size;
    /*
     * If heap_stats.lock is null, it's during initialization.
     * skip locking
     */
    if (ofc_heap_stats.lock != OFC_NULL)
      ofc_lock(ofc_heap_stats.lock);

    ofc_heap_stats.Total += size;
    if (ofc_heap_stats.Total >= ofc_heap_stats.Max)
        ofc_heap_stats.Max = ofc_heap_stats.Total;
#if OFC_HEAP_LIMIT_BYTES > 0
    /* One-shot: latch crashing under the lock so recursive allocs from the
     * dump / crash path don't retrigger. */
    if (!crashing && ofc_heap_stats.Total > OFC_HEAP_LIMIT_BYTES) {
        crashing = OFC_TRUE;
        exceeded = OFC_TRUE;
    }
#endif

    if (ofc_heap_stats.lock != OFC_NULL)
      ofc_unlock(ofc_heap_stats.lock);

#if OFC_HEAP_LIMIT_BYTES > 0
    if (exceeded) {
        OFC_CHAR msg[128];
        ofc_snprintf(msg, sizeof(msg),
                     "openfiles heap limit exceeded: Total=%u, limit=%u\n",
                     (OFC_UINT) ofc_heap_stats.Total,
                     (OFC_UINT) OFC_HEAP_LIMIT_BYTES);
        ofc_write_console(msg);
        ofc_heap_dump();
        ofc_process_crash(msg);
    }
#endif
}

static OFC_VOID
ofc_heap_free_acct(struct heap_chunk *chunk) {
    ofc_lock(ofc_heap_stats.lock);
    ofc_heap_stats.Total -= chunk->alloc_size;
    ofc_unlock(ofc_heap_stats.lock);
}

OFC_CORE_LIB OFC_VOID
ofc_heap_load(OFC_VOID) {
    ofc_heap_stats.Max = 0;
    ofc_heap_stats.Total = 0;
#if defined(OFC_HEAP_DEBUG)
    ofc_heap_stats.Allocated = OFC_NULL;
#endif
    /* Make NULL until we init the heap.  Since nothing
     * is running, we don't need locks yet
     */
    ofc_heap_stats.lock = OFC_NULL;
    ofc_heap_init_impl();
    ofc_heap_stats.lock = ofc_lock_init();
}

OFC_CORE_LIB OFC_VOID
ofc_heap_unload(OFC_VOID) {
    /* The client or server doesn't shutdown */
    OFC_LOCK save;
    save = ofc_heap_stats.lock;
    ofc_heap_stats.lock = OFC_NULL;
    ofc_lock_destroy(save);
    ofc_heap_unload_impl();
    ofc_heap_dump();
    ofc_heap_unmap_impl();
}

#if defined(OFC_HEAP_DEBUG)

static OFC_VOID
ofc_heap_debug_alloc(OFC_SIZET size, struct heap_chunk *chunk)
{
#if defined(BACKTRACE_SYM)
    char **trace;
#else
    void *trace[8];
#endif

    ofc_lock(ofc_heap_stats.lock);
    chunk->dbgnext = ofc_heap_stats.Allocated;
    if (ofc_heap_stats.Allocated != OFC_NULL)
        ofc_heap_stats.Allocated->dbgprev = chunk;
    ofc_heap_stats.Allocated = chunk;
    chunk->dbgprev = 0;

#if defined(BACKTRACE_SYM)
    ofc_backtrace_sym(&trace, 8);

    ofc_strncpy(chunk->caller1, trace[4], 119);
    chunk->caller1[120] = '\0';
    ofc_strncpy(chunk->caller2, trace[5], 119);
    chunk->caller2[120] = '\0';
    ofc_strncpy(chunk->caller3, trace[6], 119);
    chunk->caller3[120] = '\0';
    ofc_strncpy(chunk->caller4, trace[7], 119);
    chunk->caller4[120] = '\0';

    ofc_backtrace_sym_free(trace);
#else
    ofc_backtrace(trace, 8);

    chunk->caller1 = trace[4];
    chunk->caller2 = trace[5];
    chunk->caller3 = trace[6];
    chunk->caller4 = trace[7];
#endif

    chunk->snap = OFC_FALSE;

    ofc_unlock(ofc_heap_stats.lock);
}

#endif

#if defined(OFC_HEAP_DEBUG)

static OFC_VOID
ofc_heap_debug_free(struct heap_chunk *chunk) {
#if defined(BACKTRACE_SYM)
    char **trace;
#else
    void *trace[8];
#endif
    /*
     * Pull off the allocation queue
     */
    ofc_lock(ofc_heap_stats.lock);
    /*
     * If there is a previous to our chunk, tell it that it's next
     * is our next.
     * If there is no previous to our chunk, it means that we were
     * the head of the list.  This implies that the Allocated pointer
     * will have been us.  Set the allocated pointer to the next guy.
     */
    if (chunk->dbgprev != OFC_NULL)
        chunk->dbgprev->dbgnext = chunk->dbgnext;
    else
      {
        ofc_assert (ofc_heap_stats.Allocated == chunk, "Head should be us");
        ofc_heap_stats.Allocated = chunk->dbgnext;
      }

    /* 
     * if there is a next to us, we want the next's previous to
     * be our previous.
     * if there isn't a next to us, it means we are the end of the list.
     * The previous to the head
     */
    if (chunk->dbgnext != OFC_NULL)
        chunk->dbgnext->dbgprev = chunk->dbgprev;

#if defined(BACKTRACE_SYM)
    ofc_backtrace_sym(&trace, 8);

    ofc_strncpy(chunk->caller1, trace[4], 119);
    chunk->caller1[120] = '\0';
    ofc_strncpy(chunk->caller2, trace[5], 119);
    chunk->caller2[120] = '\0';
    ofc_strncpy(chunk->caller3, trace[6], 119);
    chunk->caller3[120] = '\0';
    ofc_strncpy(chunk->caller4, trace[7], 119);
    chunk->caller4[120] = '\0';

    ofc_backtrace_sym_free(trace);
#else
    ofc_backtrace(trace, 8);

    chunk->caller1 = trace[4];
    chunk->caller2 = trace[5];
    chunk->caller3 = trace[6];
    chunk->caller4 = trace[7];
#endif

    ofc_unlock(ofc_heap_stats.lock);
}

#endif

#if defined(BACKTRACE_SYM)
#define OBUF_SIZE 1024
#else
#define OBUF_SIZE 200
#endif
OFC_CORE_LIB OFC_VOID
ofc_heap_dump_stats(OFC_VOID) {
#if defined(OFC_HEAP_DEBUG)
    OFC_CHAR obuf[OBUF_SIZE];
    OFC_SIZET len;

    len = ofc_snprintf(obuf, OBUF_SIZE,
                       "Total Allocated Memory %d, Max Allocated Memory %d\n",
                       ofc_heap_stats.Total, ofc_heap_stats.Max);
    ofc_write_console(obuf);
#endif
}

OFC_CORE_LIB OFC_VOID
ofc_heap_dump(OFC_VOID) {
#if defined(OFC_HEAP_DEBUG)
    struct heap_chunk *chunk;
    OFC_CHAR obuf[OBUF_SIZE];
    OFC_SIZET len;
#endif

    ofc_heap_dump_stats();
#if defined(OFC_HEAP_DEBUG)
#if (defined(__GNUC__) || defined(__clang__)) && defined(OFC_STACK_TRACE)
    if (ofc_heap_stats.Allocated == OFC_NULL) {
        len = ofc_snprintf(obuf, OBUF_SIZE,
                           "\nHeap is Empty, No leaks detected\n");
        ofc_write_console(obuf);
    } else {

#if defined(BACKTRACE_SYM)
        len = ofc_snprintf(obuf, OBUF_SIZE,
                           "%-16s %-10s %-16s\n",
                           "Address", "Size", "Trace");
#else
        len = ofc_snprintf(obuf, OBUF_SIZE,
                           "%-16s %-10s %-16s %-16s %-16s %-16s\n",
                           "Address", "Size", "Caller1", "Caller2", "Caller3",
                           "Caller4");
#endif
        ofc_write_console(obuf);
    }

    ofc_lock(ofc_heap_stats.lock);
    for (chunk = ofc_heap_stats.Allocated;
         chunk != OFC_NULL;
         chunk = chunk->dbgnext) {
        if (!chunk->snap) {
#if defined(BACKTRACE_SYM)
	  len = ofc_snprintf(obuf, OBUF_SIZE,
			     "%0-16p %-10d\n"
			     "                           %s\n"
			     "                           %s\n"
			     "                           %s\n"
			     "                           %s\n",
			     chunk + 1, (OFC_INT) chunk->alloc_size,
			     chunk->caller1,
			     chunk->caller2,
			     chunk->caller3,
			     chunk->caller4);
#else
#if defined(__cyg_profile)
	  len = ofc_snprintf (obuf, OBUF_SIZE,
			      "%0-16p %-10d %0-16s %0-16s %0-16s %0-16s\n",
			      chunk+1, (OFC_INT) chunk->alloc_size,
			      __cyg_profile_addr2sym(chunk->caller1),
			      __cyg_profile_addr2sym(chunk->caller2),
			      __cyg_profile_addr2sym(chunk->caller3),
			      __cyg_profile_addr2sym(chunk->caller4)) ;
#else
	  len = ofc_snprintf(obuf, OBUF_SIZE,
			     "%0-16p %-10d %0-16p %0-16p %0-16p %0-16p\n",
			     chunk + 1, (OFC_INT) chunk->alloc_size,
			     ofc_process_relative_addr(chunk->caller1),
			     ofc_process_relative_addr(chunk->caller2),
			     ofc_process_relative_addr(chunk->caller3),
			     ofc_process_relative_addr(chunk->caller4));
#endif
#endif
	  ofc_write_console(obuf);
        }
    }
    ofc_unlock(ofc_heap_stats.lock);
#else
    len = ofc_snprintf (obuf, OBUF_SIZE, "%-20s %-10s %-20s\n",
                 "Address", "Size", "Caller") ;
    ofc_write_console(obuf) ;
    ofc_lock (ofc_heap_stats.lock) ;
    for (chunk = ofc_heap_stats.Allocated ;
         chunk != OFC_NULL ;
         chunk = chunk->dbgnext)
      {
        len = ofc_snprintf (obuf, OBUF_SIZE, "%-20p %-10d %-20p\n",
                 chunk+1, chunk->alloc_size, chunk->caller) ;
        ofc_write_console (obuf) ;
      }
    ofc_unlock (ofc_heap_stats.lock) ;
#endif
    len = ofc_snprintf(obuf, OBUF_SIZE, "\n");
    ofc_write_console(obuf);
#endif
}

OFC_CORE_LIB OFC_VOID
ofc_heap_check_alloc(OFC_LPCVOID mem) {
    const struct heap_chunk *chunk;

    if (mem != OFC_NULL) {
        chunk = mem;
        chunk--;

        ofc_heap_check_alloc_impl(chunk);
    }
}

OFC_CORE_LIB OFC_VOID
ofc_heap_snap(OFC_VOID) {
#if defined(OFC_HEAP_DEBUG)
    struct heap_chunk *chunk;

    ofc_lock(ofc_heap_stats.lock);
    for (chunk = ofc_heap_stats.Allocated;
         chunk != OFC_NULL;
         chunk = chunk->dbgnext) {
        chunk->snap = OFC_TRUE;
    }

    ofc_unlock(ofc_heap_stats.lock);
#endif
}

OFC_CORE_LIB OFC_VOID
ofc_heap_dump_chunk(OFC_LPVOID mem) {
#if defined(OFC_HEAP_DEBUG)
#if (defined(__GNUC__) || defined(__clang__)) && defined(OFC_STACK_TRACE)
    struct heap_chunk *chunk;

    if (mem != OFC_NULL) {
        chunk = mem;
        chunk--;


#if defined(BACKTRACE_SYM)
	ofc_printf("%0-16p %-10d\n"
		   "                           %s\n"
		   "                           %s\n"
		   "                           %s\n"
		   "                           %s\n",
		   chunk + 1, (OFC_INT) chunk->alloc_size,
		   chunk->caller1,
		   chunk->caller2,
		   chunk->caller3,
		   chunk->caller4);
#else
	ofc_printf("%0-16p %-10d %0-16p %0-16p %0-16p %0-16p\n",
		   chunk + 1, (OFC_INT) chunk->alloc_size,
		   ofc_process_relative_addr(chunk->caller1),
		   ofc_process_relative_addr(chunk->caller2),
		   ofc_process_relative_addr(chunk->caller3),
		   ofc_process_relative_addr(chunk->caller4));
#endif	
    }
#endif
#endif
}

OFC_CORE_LIB OFC_LPVOID
ofc_malloc(OFC_SIZET size) {
    OFC_LPVOID mem;
    struct heap_chunk *chunk;

    mem = OFC_NULL;
    chunk = ofc_malloc_impl(size + sizeof(struct heap_chunk));
    if (chunk != OFC_NULL) {
        ofc_heap_malloc_acct(size, chunk);
#if defined(OFC_HEAP_DEBUG)
        ofc_heap_debug_alloc(size, chunk);
#endif
        mem = (OFC_LPVOID) (++chunk);
    } else {
        ofc_process_crash("ofc_malloc Allocation Failed\n");
    }
    return (mem);
}

OFC_CORE_LIB OFC_LPVOID
ofc_calloc(OFC_SIZET nmemb, OFC_SIZET size) {
    return (ofc_malloc(nmemb * size));
}

OFC_CORE_LIB OFC_VOID
ofc_free(OFC_LPVOID mem) {
    struct heap_chunk *chunk;

    if (mem != OFC_NULL) {
        chunk = mem;
        chunk--;

#if defined(OFC_HEAP_DEBUG)
        ofc_heap_debug_free(chunk);
#endif
        ofc_heap_free_acct(chunk);
        ofc_free_impl(chunk);
    }
}

OFC_CORE_LIB OFC_LPVOID
ofc_realloc(OFC_LPVOID ptr, OFC_SIZET size) {
    struct heap_chunk *chunk;
    struct heap_chunk *newchunk;
    OFC_LPVOID mem;

    mem = OFC_NULL;
    chunk = ptr;
    if (chunk != OFC_NULL) {
        chunk--;

        ofc_heap_free_acct(chunk);

#if defined(OFC_HEAP_DEBUG)
        ofc_heap_debug_free(chunk);
#endif

        newchunk = ofc_realloc_impl(chunk,
                                    size + sizeof(struct heap_chunk));

        if (newchunk != OFC_NULL) {
#if defined(OFC_HEAP_DEBUG)
          ofc_heap_debug_alloc(size, newchunk);
#endif
            ofc_heap_malloc_acct(size, newchunk);
            chunk = newchunk;
            mem = chunk + 1;
        } else {
            ofc_process_crash("ofc_realloc Allocation Failed\n");
        }
    } else
        mem = ofc_malloc(size);

    return (mem);
}

