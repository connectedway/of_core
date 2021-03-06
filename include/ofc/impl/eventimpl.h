/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__OFC_EVENT_IMPL_H__)
#define __OFC_EVENT_IMPL_H__

#include "ofc/core.h"
#include "ofc/types.h"

#if defined(__cplusplus)
extern "C"
{
#endif

/**
 * Create a Platform Specific Event
 *
 * This function is called to create a platform specific event.
 * There are two types of events, AUTO and MANUAL.  Auto events
 * are automatically reset after waiting on them.  Manual events stay
 * set until explicitly reset.
 *
 * \param eventType
 * Type of event
 *
 * \returns
 * Handle to event
 */
OFC_HANDLE
ofc_event_create_impl(OFC_EVENT_TYPE eventType);

/**
 * Return the event type
 *
 * \param hEvent
 * The handle to the event to obtail the type for
 *
 * \returns
 * The event type
 */
OFC_EVENT_TYPE ofc_event_get_type_impl(OFC_HANDLE hEvent);

/**
 * Set an event.
 *
 * Cause an event to be signalled in the platform
 *
 * \param hEvent
 * The event to signal
 */
OFC_VOID ofc_event_set_impl(OFC_HANDLE hEvent);

/**
 * Reset an Event
 *
 * Rearm a manual event in the platform
 *
 * \param hEvent
 * The event to rearm
 */
OFC_VOID ofc_event_reset_impl(OFC_HANDLE hEvent);

/**
 * Test if an event has been signalled
 *
 * \param hEvent
 * The event handle to test
 *
 * \returns
 * OFC_TRUE if set, OFC_FALSE otherwise
 */
OFC_BOOL ofc_event_test_impl(OFC_HANDLE hEvent);

/**
 * Destroy an Event
 *
 * \param hEvent
 * The handle to the event to destroy
 */
OFC_VOID ofc_event_destroy_impl(OFC_HANDLE hEvent);

/**
 * Wait for an event to fire.
 *
 * If the event is an automatic event, it is automatically reset after
 * this return returns
 */
OFC_VOID ofc_event_wait_impl(OFC_HANDLE hEvent);

#if defined(__cplusplus)
}
#endif

#endif

