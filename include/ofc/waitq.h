/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__OFC_WAIT_QUEUE_H__)
#define __OFC_WAIT_QUEUE_H__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/handle.h"

/**
 * \defgroup waitq Wait Queue Facility
 */

/** \{ */

#if defined(__cplusplus)
extern "C"
{
#endif
/**
 * Create a linked list that can be blocked on
 *
 * \returns
 * Handle to the wait queue
 */
OFC_CORE_LIB OFC_HANDLE
ofc_waitq_create(OFC_VOID);
/**
 * Destroy a queue
 *
 * \param qHead
 * Head of queue
 */
OFC_CORE_LIB OFC_VOID
ofc_waitq_destroy(OFC_HANDLE qHead);
/**
 * Add an element to the end of the list
 *
 * \param qHead
 * Pointer to list header
 *
 * \param qElement
 * Pointer to element to add to list
 */
OFC_CORE_LIB OFC_VOID
ofc_waitq_enqueue(OFC_HANDLE qHead, OFC_VOID *qElement);
/**
 * Remove an element from the front of the list
 *
 * \param qHead
 * Pointer to list header
 *
 * \returns
 * Item at the front or NB_NULL
 */
OFC_CORE_LIB OFC_VOID *
ofc_waitq_dequeue(OFC_HANDLE qHead);
/**
 * See if queue is empty
 *
 * \param qHead
 * Queue Head
 *
 * \returns
 * OFC_TRUE if empty, OFC_FALSE otherwise
 */
OFC_CORE_LIB OFC_BOOL
ofc_waitq_empty(OFC_HANDLE qHead);
/**
 * Return the head of the linked list
 *
 * \param qHead
 * Pointer to the head of the list
 *
 * \returns
 * Pointer to the first element on the queue
 */
OFC_CORE_LIB OFC_VOID *
ofc_waitq_first(OFC_HANDLE qHead);
/**
 * Return the next element on the list
 *
 * \param qHead
 * Pointer to the head of the list
 *
 * \param qElement
 * Pointer to the current element
 *
 * \returns
 * Pointer to the next element on the queue.
 */
OFC_CORE_LIB OFC_VOID *
ofc_waitq_next(OFC_HANDLE qHead, OFC_VOID *qElement);
/**
 * Unlink the current element from the list
 *
 * \param qHead
 * Pointer to the head of the list
 *
 * \param qElement
 * Pointer to the current element
 */
OFC_CORE_LIB OFC_VOID
ofc_waitq_unlink(OFC_HANDLE qHead, OFC_VOID *qElement);
/**
 * Clear the contents of a wait queue
 *
 * \param qHandle
 * Handle to the waitqueue to destroy
 */
OFC_CORE_LIB OFC_VOID
ofc_waitq_clear(OFC_HANDLE qHandle);
/**
 * Get the event handle that the wait queue waits on
 *
 * \param qHandle
 * Handle to the wait queue
 *
 * \returns
 * Handle to the event for the queue
 */
OFC_CORE_LIB OFC_HANDLE
ofc_waitq_get_event_handle(OFC_HANDLE qHandle);
/**
 * Wake up the waitq without queueing anything
 *
 * \param qHandle
 * Handle to the wait queue
 *
 * \returns
 * nothing
 */
OFC_CORE_LIB OFC_VOID
ofc_waitq_wake(OFC_HANDLE qHandle);
/**
 * Wait for an element to be placed on the wait queue
 *
 * \param waitq
 * Wait Q to wait for
 *
 * \remarks
 * When an element has been placed on the queue, the calling thread will
 * be woken
 */
OFC_CORE_LIB OFC_VOID
ofc_waitq_block(OFC_HANDLE waitq);

#if defined(__cplusplus)
}
#endif
/** \} */
#endif
