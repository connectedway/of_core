/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#include "unity.h"
#include "unity_fixture.h"

#include "ofc/config.h"
#include "ofc/types.h"
#include "ofc/handle.h"
#include "ofc/timer.h"
#include "ofc/thread.h"
#include "ofc/libc.h"
#include "ofc/sched.h"
#include "ofc/app.h"
#include "ofc/heap.h"
#include "ofc/core.h"
#include "ofc/framework.h"
#include "ofc/env.h"
#include "ofc/persist.h"
#include "ofc/event.h"

extern OFC_CHAR config_path[OFC_MAX_PATH+1];

extern OFC_HANDLE hScheduler;
extern OFC_HANDLE hDone;

OFC_VOID test_shutdown(OFC_VOID);
OFC_INT test_startup(OFC_VOID);

/*
 * The loop interval. 5 seconds
 */
#define THREAD_TEST_RUN_INTERVAL 5000

/*
 * Forward Declaration of a Daemon App for the test thread
 *
 * Daemon apps are internal constructs to Open Files that are leveraged
 * by the thread test.  It may be educational to review the deamon
 * code structure, but it is not necessarily required to understand it
 * unless you will be writing daemon apps.
 */
static OFC_DWORD ThreadTestApp(OFC_HANDLE hThread, OFC_VOID *context);

/*
 * States that the deamon can be in
 */
typedef enum {
    THREAD_TEST_STATE_IDLE,    /* The Daemon is idle */
    THREAD_TEST_STATE_RUNNING    /* The Daemon is running */
} THREAD_TEST_STATE;

/*
 * The deamon's context
 */
typedef struct {
    THREAD_TEST_STATE state;    /* The state that the deamon is in */
    OFC_HANDLE hThread;        /* The handle to the created thread */
    OFC_HANDLE hScheduler;    /* The scheduler that the deamon is on */
    OFC_HANDLE hTimer;        /* The timer for thread creation */
} THREAD_TEST_APP;

/*
 * Forward declaration to the deamon apps preselect routine
 */
static OFC_VOID ThreadTestPreSelect(OFC_HANDLE app);

/*
 * Forward declaration to the deamon apps postselect routine
 */
static OFC_HANDLE ThreadTestPostSelect(OFC_HANDLE app,
                                       OFC_HANDLE hSocket);

/*
 * Forward declaration to the deamon apps destroy routine
 */
static OFC_VOID ThreadTestDestroy(OFC_HANDLE app);

#if defined(OFC_APP_DEBUG)
/*
 * Forward declearation to the deamon apps dump routine
 */
static OFC_VOID thread_test_dump (OFC_HANDLE app) ;
#endif

/*
 * Definition of the thread test deamon application
 */
static OFC_APP_TEMPLATE ThreadTestAppDef =
        {
                "Thread Test App",        /* The deamon name */
                &ThreadTestPreSelect,    /* The preselect routine */
                &ThreadTestPostSelect,    /* The post select routine */
                &ThreadTestDestroy,        /* The destroy routine */
#if defined(OFC_APP_DEBUG)
                &thread_test_dump		/* The dump routine */
#else
                OFC_NULL
#endif
        };

#if defined(OFC_APP_DEBUG)
/*
 * The deamon's dump routine.
 *
 * The routine will dump the context on the console
 *
 * \param app
 * The dump callback is given a handle to the app.  The context variable
 * can be obtained from this handle
 */
static OFC_VOID thread_test_dump (OFC_HANDLE app)
{
  THREAD_TEST_APP *ThreadApp ;	/* The deamon's context */

  /*
   * Get the deamon context from the application
   */
  ThreadApp = ofc_app_get_data (app) ;
  if (ThreadApp != OFC_NULL)
    {
      /*
       * Just dump it's state
       */
      ofc_printf ("%-20s : %d\n", "Thread Test State", 
           (OFC_UINT16) ThreadApp->state) ;
    }
}
#endif

/*
 * The test thread's run routine
 *
 * \param hThread
 * Handle to the thread
 *
 * \param context
 * Pointer to the thread's context
 *
 * \returns
 * A status.  0 is success
 */
static OFC_DWORD ThreadTestApp(OFC_HANDLE hThread, OFC_VOID *context) {
    /*
     * The test thread should continue running until it is deleted
     */
    while (!ofc_thread_is_deleting(hThread)) {
        ofc_printf("Test Thread is Running\n");
        /*
         * Wait for a second
         */
        ofc_sleep(1000);
    }
    ofc_printf("Test Thread is Exiting\n");
    return (0);
}

/*
 * The thread deamon's preselect routine
 *
 * \param app
 * Handle of the application
 *
 * \returns
 * Nothing
 */
static OFC_VOID ThreadTestPreSelect(OFC_HANDLE app) {
    THREAD_TEST_APP *ThreadApp;    /* The deamon's context */
    THREAD_TEST_STATE entry_state;
    /*
     * Obtain the deamon's context
     */
    ThreadApp = ofc_app_get_data(app);
    if (ThreadApp != OFC_NULL) {
        do /* while ThreadApp->state != entry_state */
        {
            entry_state = ThreadApp->state;
            ofc_sched_clear_wait(ThreadApp->hScheduler, app);
            /*
             * dispatch on the deamon's state
             */
            switch (ThreadApp->state) {
                default:
                case THREAD_TEST_STATE_IDLE:
                    /*
                     * deamons are initialy created in the IDLE state.  A
                     * call to the deamon's preselect routine in the IDLE state
                     * is an indication of the deamon startup
                     *
                     * Create a timer event for the deamon.  We use this timer
                     * event to send a delete to the test thread
                     */
                    ofc_timer_set(ThreadApp->hTimer, THREAD_TEST_RUN_INTERVAL);
                    /*
                     * Add the timer event to scheduler for our deamon
                     */
                    ofc_sched_add_wait(ThreadApp->hScheduler, app,
                                       ThreadApp->hTimer);
                    /*
                     * And set our state to running
                     */
                    ThreadApp->state = THREAD_TEST_STATE_RUNNING;
                    break;

                case THREAD_TEST_STATE_RUNNING:
                    /*
                     * Keep the timer event active
                     */
                    ofc_sched_add_wait(ThreadApp->hScheduler, app,
                                       ThreadApp->hTimer);
                    break;
            }
        } while (ThreadApp->state != entry_state);
    }
}

/*
 * The post select routine for the thread deamon
 *
 * \param app
 * Handle to the deamon app
 *
 * \param hEvent
 * Event that we should service
 *
 * \returns Cascaded event
 */
static OFC_HANDLE ThreadTestPostSelect(OFC_HANDLE app, OFC_HANDLE hEvent) {
    THREAD_TEST_APP *ThreadApp;    /* The deamon's context */
    OFC_BOOL progress;        /* Whether we have serviced anything */

    /*
     * Get the deamon's context
     */
    ThreadApp = ofc_app_get_data(app);
    if (ThreadApp != OFC_NULL) {
        /*
         * While we have just serviced something and while we are not
         * being destroyed
         */
        for (progress = OFC_TRUE; progress && !ofc_app_destroying(app);) {
            /*
             * Say we haven't processed anything
             */
            progress = OFC_FALSE;
            /*
             * Switch on our state
             */
            switch (ThreadApp->state) {
                default:
                case THREAD_TEST_STATE_IDLE:
                    /*
                     * We should never be idle anymore
                     */
                    break;

                case THREAD_TEST_STATE_RUNNING:
                    /*
                     * The only event we expect is the timer event.  Let's make
                     * sure that's what it is
                     */
                    if (hEvent == ThreadApp->hTimer) {
                        /*
                         * Let's schedule us to be destroyed
                         */
                        ofc_app_kill(app);
                        /*
                         * And say we've serviced something
                         */
                        progress = OFC_TRUE;
                    }
                    break;
            }
        }
    }
    return (OFC_HANDLE_NULL);
}

/*
 * The destroy callback for the deamon
 *
 * \param app
 * The deamon's handle
 *
 * \returns Nothing
 */
static OFC_VOID ThreadTestDestroy(OFC_HANDLE app) {
    THREAD_TEST_APP *ThreadApp;    /* Our deamon's context */

    /*
     * Get the deamon's context
     */
    ThreadApp = ofc_app_get_data(app);
    if (ThreadApp != OFC_NULL) {
        /*
         * Switch on our state
         */
        switch (ThreadApp->state) {
            default:
            case THREAD_TEST_STATE_IDLE:
                /*
                 * We should never be idle
                 */
                break;

            case THREAD_TEST_STATE_RUNNING:
                /*
                 * We just have a timer to delete
                 */
                ofc_timer_destroy(ThreadApp->hTimer);
                break;
        }
    }
}

TEST_GROUP(thread);

TEST_SETUP(thread) {
    TEST_ASSERT_FALSE_MESSAGE(test_startup(), "Failed to Startup Framework");
}

TEST_TEAR_DOWN(thread) {
    test_shutdown();
}

TEST(thread, test_thread) {
    THREAD_TEST_APP *ThreadApp;
    OFC_HANDLE hThread;
    OFC_HANDLE hApp;

    /*
     * Create the Thread that this deamon will interact with
     */
    hThread = ofc_thread_create(&ThreadTestApp,
                                OFC_THREAD_THREAD_TEST, OFC_THREAD_SINGLETON,
                                OFC_NULL,
                                OFC_THREAD_JOIN, OFC_HANDLE_NULL);
    if (hThread == OFC_HANDLE_NULL)
        ofc_printf("Could not create ThreadTestApp\n");
    else {
        /*
         * Created the thread, so let's create the deamon application.
         *
         * Allocate the deamon's context
         */
        ThreadApp = ofc_malloc(sizeof(THREAD_TEST_APP));
        ThreadApp->hScheduler = hScheduler;
        /*
         * Create a timer that the thread will wait on
         */
        ThreadApp->hTimer = ofc_timer_create("THREAD");
        ThreadApp->state = THREAD_TEST_STATE_IDLE;
        ThreadApp->hThread = hThread;
        /*
         * Create the deamon application
         */
        hApp = ofc_app_create(ThreadApp->hScheduler, &ThreadTestAppDef,
                              ThreadApp);
        if (hDone != OFC_HANDLE_NULL) {
            ofc_app_set_wait(hApp, hDone);
            ofc_event_wait(hDone);

            ofc_printf("Deleting Thread\n");
            /*
             * Delete the thread
             */
            ofc_thread_delete(ThreadApp->hThread);
            ofc_thread_wait(ThreadApp->hThread);
            /*
             * And delete our deamon's context
             */
            ofc_free(ThreadApp);
        }
    }
}

TEST_GROUP_RUNNER(thread) {
    RUN_TEST_CASE(thread, test_thread);
}

#if !defined(NO_MAIN)
static void runAllTests(void)
{
  RUN_TEST_GROUP(thread);
}

int main(int argc, const char *argv[])
{
  if (argc >= 2) {
    if (ofc_strcmp(argv[1], "--config") == 0) {
      ofc_strncpy(config_path, argv[2], OFC_MAX_PATH);
    }
  }

  return UnityMain(argc, argv, runAllTests);
}
#endif
