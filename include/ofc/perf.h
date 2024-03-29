/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__OFC_PERF_H__)
#define __OFC_PERF_H__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/handle.h"
#include "ofc/lock.h"

/** \{ */

struct perf_measurement {
  OFC_MSTIME start_stamp;
  OFC_MSTIME stop_stamp;
  OFC_BOOL stop;
  OFC_INT nqueues;
  OFC_INT nrts;
  OFC_HANDLE queues;
  OFC_HANDLE rts;
  OFC_HANDLE notify;
  OFC_HANDLE hThread;
  OFC_UINT instance;
  OFC_LOCK lock;
} ;
  
struct perf_queue {
  OFC_MSTIME basis;
  OFC_ULONG num_requests;
  OFC_ULONG total_byte_count;
  OFC_UINT depth ;
  OFC_UINT total_depth;
  OFC_UINT depth_samples;
  OFC_CTCHAR *description;
  OFC_INT instance;
  OFC_LOCK lock;
};

struct perf_rt {
  OFC_CTCHAR *description;
  OFC_INT instance;
  OFC_MSTIME total;
  OFC_MSTIME start;
};

struct perf_statistics {
  OFC_CTCHAR *description;
  OFC_INT instance;
  OFC_LONG elapsed_ms;
  OFC_LONG total_byte_count;
  OFC_LONG num_requests;
  OFC_LONG avg_packet_size;
  OFC_LONG request_throughput;
  OFC_LONG average_depth_x1000;
  OFC_LONG basis;
  OFC_LONG depth_samples;
  OFC_LONG lead_x1000;
  OFC_LONG total_depth;
};

/**
 * A structure for helping us measure queing delay
 */
typedef enum {
    OFC_PERF_FSSMB_READ = 0,
    OFC_PERF_FSSMB_WRITE = 1,
    OFC_PERF_CLIENT_READ = 2,
    OFC_PERF_CLIENT_WRITE = 3,
    OFC_PERF_SERVER_READ = 4,
    OFC_PERF_SERVER_WRITE = 5,
    OFC_PERF_SESSION_READ = 6,
    OFC_PERF_SESSION_WRITE = 7,
    OFC_PERF_NUM = 8
} OFC_PERF_ID;

extern struct perf_measurement *g_measurement;

#if defined(__cplusplus)
extern "C"
{
#endif
  OFC_VOID measurement_init(OFC_VOID);
  OFC_VOID measurement_destroy(OFC_VOID);
  OFC_VOID measurement_wait(struct perf_measurement *measurement);

  struct perf_measurement *measurement_alloc (OFC_VOID);
  OFC_VOID measurement_free(struct perf_measurement *measurement);
  OFC_BOOL measurement_start(struct perf_measurement *measurement);
  OFC_BOOL measurement_statistics(struct perf_measurement *measurement);
  OFC_BOOL measurement_stop(struct perf_measurement *measurement,
			    OFC_HANDLE notify);
  OFC_VOID measurement_poll(struct perf_measurement *measurement);
  OFC_BOOL measurement_notify(struct perf_measurement *measurement);
  OFC_VOID perf_queue_reset(struct perf_queue *queue);
  struct perf_rt *
  perf_rt_create (struct perf_measurement *measurement,
		  OFC_CTCHAR *description,
		  OFC_INT instance);
  OFC_VOID perf_rt_destroy(struct perf_measurement *measurement,
			   struct perf_rt *rt);
  OFC_VOID perf_rt_reset(struct perf_rt *rt);
  OFC_VOID perf_rt_start(struct perf_rt *rt);
  OFC_VOID perf_rt_stop(struct perf_rt *rt);
  struct perf_queue *
  perf_queue_create (struct perf_measurement *measurement,
		     OFC_CTCHAR *description,
		     OFC_INT instance);
  OFC_VOID perf_queue_destroy(struct perf_measurement *measurement,
			      struct perf_queue *queue);
  OFC_VOID perf_queue_statistics(struct perf_measurement *measurement,
				 struct perf_queue *queue,
				 struct perf_statistics *statistics);
  OFC_VOID perf_request_start (struct perf_measurement *measurement,
			       struct perf_queue *queue);
  OFC_VOID perf_request_stop (struct perf_measurement *measurement,
			      struct perf_queue *queue,
			      OFC_LONG byte_count);
  OFC_VOID perf_queue_poll(struct perf_measurement *measurement,
			   struct perf_queue *queue);
  OFC_VOID perf_statistics_print(struct perf_statistics *statistics);
#if defined(__cplusplus)
}
#endif
/** \} */
#endif

