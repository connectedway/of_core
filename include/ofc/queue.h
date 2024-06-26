/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__OFC_QUEUE_H__)
#define __OFC_QUEUE_H__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/handle.h"

/**
 * \{
 * \defgroup queue Open Files Queue Management Facility
 *
 * Routines that provide for queue management
 *
 * Function | Description
 * ---------|-------------
 * \ref ofc_queue_create | Create a queue
 * \ref ofc_queue_destroy | Destroy a queue
 * \ref ofc_enqueue | Enqueue to the end of a queue
 * \ref ofc_dequeue | Dequeue from the head of a queue
 * \ref ofc_queue_empty | Test if queue is empty
 * \ref ofc_queue_first | Return the head of a queue without dequeueing
 * \ref ofc_queue_next | Return the next element after an element
 * \ref ofc_queue_unlink | Remove an element from within the queue
 * \ref ofc_queue_clear | Remove all elements from the queue
 */

#if defined(__cplusplus)
extern "C"
{
#endif
/**
 * Create a linked list
 *
 * \returns
 * Handle to the linked list
 */
OFC_CORE_LIB OFC_HANDLE
ofc_queue_create(OFC_VOID);
/**
 * Destroy a queue
 *
 * \param qHead
 * Head of queue
 */
OFC_CORE_LIB OFC_VOID
ofc_queue_destroy(OFC_HANDLE qHead);
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
ofc_enqueue(OFC_HANDLE qHead, OFC_VOID *qElement);
/**
 * Remove an element from the front of the list
 *
 * \param qHead
 * Pointer to list header
 *
 * \returns
 * Item at the front or OFC_NULL
 */
OFC_CORE_LIB OFC_VOID *
ofc_dequeue(OFC_HANDLE qHead);
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
ofc_queue_empty(OFC_HANDLE qHead);
/**
 * Return the head of the linked list
 *
 * \param qHead
 * Pointer to the head of the list
 *
 * \returns
 * Element that was at the front of the list
 */
OFC_CORE_LIB OFC_VOID *
ofc_queue_first(OFC_HANDLE qHead);
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
 * Pointer to the next element
 */
OFC_CORE_LIB OFC_VOID *
ofc_queue_next(OFC_HANDLE qHead, OFC_VOID *qElement);
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
ofc_queue_unlink(OFC_HANDLE qHead, OFC_VOID *qElement);
/**
 * Clear the contents of a linked list
 *
 * \param qHandle
 * Handle to the linked list
 *
 * \remarks
 * All items on the list will be removed.  If those items contain elements
 * allocated from the heap, or if the items themselves were allocated from
 * the heap, a leak will likely occur.  Care should be taken to insure that
 * either only static information is contained in the list, or the dynamic
 * information is freed someother way.
 */
OFC_CORE_LIB OFC_VOID
ofc_queue_clear(OFC_HANDLE qHandle);

#if defined(__cplusplus)
}
#endif
/** \} */
#endif
