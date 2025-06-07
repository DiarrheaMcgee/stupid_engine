#pragma once

#include "stupid/common.h"
#include "stupid/clock.h"
#include "stupid/logger.h"
#include "stupid/assert.h"

#include <stdatomic.h>
#include <threads.h>

/// Thread ID.
typedef u64 StThreadID;

/// Thread local identifier (used to avoid calling pthread_self() over and over).
static _Thread_local StThreadID STUPID_THREAD_ID = 1;

static STUPID_UNUSED STUPID_ATOMIC u64 STUPID_THREAD_COUNT = 1;

/**
 * Argument for a function passed to a thread.
 * @see stThreadAddPFN, Thread
 */
typedef union StThreadArg {
        /// (u64)[4]
        u64  u64[4];

        /// (u32)[8]
        u32  u32[8];

        /// (u16)[16]
        u16  u16[16];

        /// (u8)[32]
        u8   u8[32];

        /// (i64)[4]
        i64  i64[4];

        /// (i32)[8]
        i32  i32[8];

        /// (i64)[16]
        i16  i16[16];

        /// (i64)[32]
        i8   i8[32];

        /// (char)[32]
        char c[32];

        /// (f64)[4]
        f64  f64[4];

        /// (f32)[8]
        f32  f32[8];

        /// (void *)[4]
        void *p[4];
} StThreadArg;


/// Basically a worse version of pthread_mutex_t.
typedef struct StMutex {
        /// Total waiting for the mutex to be unlocked.
        STUPID_ATOMIC usize waiting;

        /// ID of the thread which owns the mutex.
        StThreadID owner;

        /// Mutex lock.
        STUPID_ATOMIC bool lock;
} StMutex;

/// This is just an boolean that threads wait for until its set to true.
typedef struct StFence {
	STUPID_ATOMIC bool lock;
	STUPID_ATOMIC usize waiting;
} StFence;

/**
 * Thread safe way to get the state of an atomic boolean.
 * @param input Pointer to an atomic boolean.
 * @return The state of the input atomic boolean.
 */
static STUPID_INLINE bool stGetAtomicBoolState(STUPID_ATOMIC bool *input)
{
	return atomic_load(input);
}

/**
 * Thread safe way to get the state of an atomic boolean.
 * @param input Pointer to an atomic boolean.
 * @param state State to set the input atomic boolean to.
 */
static STUPID_INLINE void stSetAtomicBoolState(STUPID_ATOMIC bool *input, const bool state)
{
        atomic_store(input, state);
}

/**
 * Locks a mutex.
 * @param pMutex Pointer to a mutex.
 * @note Blocks while the mutex is already locked.
 * @note If the engine becomes unresponsive, this could be the cuplrit.
 */
static STUPID_INLINE void (stMutexLock)(StMutex *pMutex STUPID_DBG_PROTO_PARAMS)
{
	while (true) {
		bool expected = false;
		if (atomic_compare_exchange_strong(&pMutex->lock, &expected, true)) {
			pMutex->owner = STUPID_THREAD_ID;
			break;
		}
		pMutex->waiting++;
		u64 counter = 0;
		while (stGetAtomicBoolState(&pMutex->lock)) {
                        stSleepu(1);
                        counter++;

                        if (counter == 50000)
                                STUPID_LOG_TRACEFN("thread %lu has been waiting on mutex %p for a really long time (fix this)", STUPID_THREAD_ID, (void *)pMutex);
		}
		pMutex->waiting--;
	}
}

#define stMutexLock(mutex) (stMutexLock)(mutex STUPID_DBG_PARAMS)

/**
 * Unlocks a StMutex.
 * @param pMutex Pointer to a StMutex.
 */
static STUPID_INLINE void stMutexUnlock(StMutex *pMutex)
{
        STUPID_ASSERT(pMutex->owner == STUPID_THREAD_ID, "attempted to unlock mutex from a thread which does not own it");
        STUPID_ASSERT(stGetAtomicBoolState(&pMutex->lock), "mutex is not locked");
        STUPID_ASSERT(pMutex->lock, "mutex is not locked");
	pMutex->owner = 0;
        stSetAtomicBoolState(&pMutex->lock, false);
}

/**
 * Waits for a mutex to be unlocked without locking it.
 * @param pMutex Pointer to a mutex.
 * @note Blocks while the mutex is already locked.
 */
static STUPID_INLINE void stMutexWait(StMutex *pMutex)
{
        if (stGetAtomicBoolState(&pMutex->lock)) {
                // wait while the mutex is locked
                while (stGetAtomicBoolState(&pMutex->lock))
                        stSleepu(1);
        }
}

/**
 * Resets a StFence to false.
 * @param pFence Fence to reset.
 */
static STUPID_INLINE void stFenceReset(StFence *pFence)
{
	STUPID_NC(pFence);
	while (pFence->waiting)
		stSleepu(1);
	stSetAtomicBoolState(&pFence->lock, false);
}

/**
 * Signals a StFence by setting it to true.
 * @param pFence Fence to reset.
 */
static STUPID_INLINE void stFenceSignal(StFence *pFence)
{
	STUPID_NC(pFence);
	stSetAtomicBoolState(&pFence->lock, true);
	while (pFence->waiting > 0) {}
}

/**
 * Waits for a StFence to be signaled.
 * @param pFence Fence to reset.
 * @note Blocks while the fence is unsignaled.
 */
static STUPID_INLINE void stFenceWait(StFence *pFence)
{
	STUPID_NC(pFence);
	atomic_fetch_add(&pFence->waiting, 1);
	while (!stGetAtomicBoolState(&pFence->lock))
		stSleepu(1);
	atomic_fetch_sub(&pFence->waiting, 1);
}

/**
 * Function passed to a thread.
 * @param pArg Pointer to a thread argument.
 * @see stThreadAddPFN, Thread
 */
typedef void (*StPFN_thread)(StThreadArg *pArg);

/// Function passed to a thread along with its arguments.
/// @see stThreadAddPFN, Thread, StThreadArg
typedef struct StThreadJob {
        /// Function pointer.
        StPFN_thread pfn;

        /// Pointer to the argument passed to this function.
        StThreadArg *pArg;

	/// Signaled when the job is done.
	StFence finished;
} StThreadJob;

/**
 * In order to use a thread, add a job to it, then start it.
 * Once the thread is started, it will do all the jobs in its job queue until there are no jobs left.
 * It can be cancelled, or paused, and jobs can be added to it while its started.
 * @note Dont create an entire thread for a single short term job (i.e. less than 0.1 seconds before exiting).
 * @see stThreadCreate, stThreadDestroy, ThreadJob
 */
typedef struct StThread {
        /// Job queue.
        StThreadJob *pJobs;

        /// Keeps track of time in the thread.
        StClock clock;

        /// Handle for the thread.
        void *pHandle;

        /// Mutex protecting the thread from concurrent access.
        StMutex lock;

	/// ID for this thread.
	/// @note This will typically end up being the number of threads when this thread was created.
	StThreadID id;

        /// Whether the thread is currently running or not.
        bool is_running;

        /// Whether the thread is currently inside a function or not.
        bool is_in_job;

        /// Whether the thread is paused or not.
        bool is_paused;

        /// Whether the thread has been requested to pause once the function finishes or not.
        bool pause_requested;

        /// Whether the thread has been requested to exit once the function finishes or not.
        bool exit_requested;

        /// Whether the thread should be joined.
        bool is_joined;
} StThread;

/**
 * Creates a new thread.
 * @param name The name of the thread.
 * @return A pointer to a thread which must be rejoined with stThreadJoin(), and then destroyed with stThreadDestroy().
 * @see stThreadDestroy, Thread
 */
StThread *(stThreadCreate)(void);

/**
 * Creates a new thread.
 * @param name The name of the thread.
 * @return A pointer to a thread which must be rejoined with stThreadJoin(), and then destroyed with stThreadDestroy().
 * @see stThreadDestroy, Thread
 */
//#define stThreadCreate(name)   (stThreadCreate)(name STUPID_DBG_PARAMS)
//
///**
// * Creates a new thread.
// * @param name The name of the thread.
// * @return A pointer to a thread which must be rejoined with stThreadJoin(), and then destroyed with stThreadDestroy().
// * @note Does not print logs.
// * @see stThreadDestroy, Thread
// */
//#define stThreadCreateNL(name) (stThreadCreate)(name STUPID_DBG_PARAMS_NL)

/**
 * Adds a job to the thread queue.
 * @param pThread Pointer to a thread.
 * @param pArg Pointer to an argument for the thread (this is also where the thread will store its return value).
 * @param pfn The function to add to the threads job queue.
 * @param job_name The name of the function.
 * @see stThreadCreate, Thread, StPFN_thread, StThreadArg
 */
void (stThreadAddPFN)(StThread *pThread, StThreadArg *pArg, const StPFN_thread pfn, const char *job_name STUPID_DBG_PROTO_PARAMS);

/**
 * Adds a job to the thread queue.
 * @param pThread Pointer to a thread.
 * @param pfn A StPFN_thread function.
 * @param job_name The name of the function.
 * @see stThreadCreate, Thread
 */
#define stThreadAddPFN(pThread, pArg, pfn)   (stThreadAddPFN)(pThread, pArg, pfn, #pfn STUPID_DBG_PARAMS)

/**
 * Adds a job to the thread queue.
 * @param pThread Pointer to a thread.
 * @param pfn A StPFN_thread function.
 * @param job_name The name of the function.
 * @note Does not print logs.
 * @see stThreadCreate, Thread
 */
#define stThreadAddPFNNL(pThread, pArg, pfn) (stThreadAddPFN)(pThread, pArg, pfn, #pfn STUPID_DBG_PARAMS_NL)

/**
 * Starts a threads job queue.
 * @param pThread Pointer to a thread.
 * @return True if successful.
 * @note Dont call this if the thread has no jobs in the job queue.
 * @see stThreadCreate, Thread, StPFN_thread
 */
bool stThreadStart(StThread *pThread STUPID_DBG_PROTO_PARAMS);

/**
 * Starts a threads job queue.
 * @param pThread Pointer to a thread.
 * @return True if successful.
 * @note Dont call this if the thread has no jobs in the job queue.
 * @see stThreadCreate, Thread, StPFN_thread
 */
#define stThreadStart(pThread)   (stThreadStart)(pThread STUPID_DBG_PARAMS)

/**
 * Starts a threads job queue.
 * @param pThread Pointer to a thread.
 * @return True if successful.
 * @note Dont call this if the thread has no jobs in the job queue.
 * @note Does not print logs.
 * @see stThreadCreate, Thread, StPFN_thread
 */
#define stThreadStartNL(pThread) (stThreadStart)(pThread STUPID_DBG_PARAMS_NL)

/**
 * Destroys a thread.
 * @param pThread Pointer to a thread.
 * @note A started thread must be joined before destroying it.
 * @see stThreadCreate, stThreadJoin, Thread.
 */
void (stThreadDestroy)(StThread *pThread STUPID_DBG_PROTO_PARAMS);

/**
 * Destroys a thread.
 * @param pThread Pointer to a thread.
 * @note A started thread must be joined before destroying it.
 * @see stThreadJoin, Thread.
 */
#define stThreadDestroy(pThread)   (stThreadDestroy)(pThread STUPID_DBG_PARAMS)

/**
 * Destroys a thread.
 * @param pThread Pointer to a thread.
 * @note A started thread must be joined before destroying it.
 * @note Does not print logs.
 * @see stThreadJoin, Thread.
 */
#define stThreadDestroyNL(pThread) (stThreadDestroy)(pThread STUPID_DBG_PARAMS_NL)

/**
 * Waits for either the thread to finish its job queue or the timeout to be reached, and then rejoins it with the main thread.
 * @param pThread Pointer to a thread.
 * @param timeout Number of milliseconds to wait before forcing the thread to close.
 * @return True if the thread finished normally, false if the thread had to be closed forcibly.
 * @see stThreadCreate, stThreadStart, stThreadSetPause, stThreadRequestExit, stThreadDestroy, Thread, StPFN_thread
 */
bool (stThreadJoin)(StThread *pThread, const u64 timeout STUPID_DBG_PROTO_PARAMS);

/**
 * Waits for either the thread to finish its job queue or the timeout to be reached, and then rejoins it with the main thread.
 * @param pThread Pointer to a thread.
 * @param timeout Number of milliseconds to wait before forcing the thread to close.
 * @return True if the thread finished normally, false if the thread had to be closed forcibly.
 * @see stThreadCreate, stThreadStart, stThreadSetPause, stThreadRequestExit, stThreadDestroy, Thread, StPFN_thread
 */
#define stThreadJoin(pThread, timeout)   (stThreadJoin)(pThread, timeout STUPID_DBG_PARAMS)

/**
 * Waits for either the thread to finish its job queue or the timeout to be reached, and then rejoins it with the main thread.
 * @param pThread Pointer to a thread.
 * @param timeout Number of milliseconds to wait before forcing the thread to close.
 * @return True if the thread finished normally, false if the thread had to be closed forcibly.
 * @note Dont forcibly close a thread unless you have to.
 * @note Does not print logs.
 * @see stThreadCreate, stThreadStart, stThreadSetPause, stThreadRequestExit, stThreadDestroy, Thread, StPFN_thread
 */
#define stThreadJoinNL(pThread, timeout) (stThreadJoin)(pThread, timeout STUPID_DBG_PARAMS_NL)

/**
 * Enables or disables pause for a thread.
 * @param pThread Pointer to a thread.
 * @param state True to request that the thread pauses, false to unpause it.
 * @note If pause is set to true, then the thread will pause once it is no longer in a function.
 * @note A thread can be joined while it is paused.
 * @see Thread
 */
static STUPID_INLINE void (stThreadSetPause)(StThread *pThread, const bool state STUPID_DBG_PROTO_PARAMS)
{
        STUPID_NC(pThread);
        stMutexLock(&pThread->lock);
        pThread->pause_requested = state;
        stMutexUnlock(&pThread->lock);
        STUPID_LOG_TRACEFN("thread %lu pause requested", pThread->id);
}

/**
 * Enables or disables pause for a thread.
 * @param pThread Pointer to a thread.
 * @param state True to request that the thread pauses, false to unpause it.
 * @note If pause is set to true, then the thread will pause once it is no longer in a function.
 * @note A thread can be joined while it is paused.
 * @see Thread
 */
#define stThreadSetPause(pThread, state)   (stThreadSetPause)(pThread, state STUPID_DBG_PARAMS)

/**
 * Enables or disables pause for a thread.
 * @param pThread Pointer to a thread.
 * @param state True to request that the thread pauses, false to unpause it.
 * @note If pause is set to true, then the thread will pause once it is no longer in a function.
 * @note A thread can be joined while it is paused.
 * @note Does not print logs.
 * @see Thread
 */
#define stThreadSetPauseNL(pThread, state) (stThreadSetPause)(pThread, state STUPID_DBG_PARAMS_NL)

/**
 * Makes a thread exit when it is no longer in a function.
 * @param pThread Pointer to a thread.
 * @note The thread must be joined after doing this.
 * @see stThreadJoin, stThreadDestroy, Thread
 */
static STUPID_INLINE void (stThreadRequestExit)(StThread *pThread STUPID_DBG_PROTO_PARAMS)
{
        STUPID_NC(pThread);
        stMutexLock(&pThread->lock);
        pThread->exit_requested = true;
        stMutexUnlock(&pThread->lock);
        STUPID_LOG_TRACEFN("thread %lu exit requested", pThread->id);
}

/**
 * Makes a thread exit when it is no longer in a function.
 * @param pThread Pointer to a thread.
 * @see stThreadDestroy, Thread
 */
#define stThreadRequestExit(pThread)   (stThreadRequestExit)(pThread STUPID_DBG_PARAMS)

/**
 * Makes a thread exit when it is no longer in a function.
 * @param pThread Pointer to a thread.
 * @note Does not print logs.
 * @see stThreadDestroy, Thread
 */
#define stThreadRequestExitNL(pThread) (stThreadRequestExit)(pThread STUPID_DBG_PARAMS_NL)

/**
 * Checks if a thread is running.
 * @param pThread Pointer to a thread.
 * @return True if the thread is currently running.
 * @see Thread
 */
static STUPID_INLINE bool stThreadIsRunning(StThread *pThread)
{
        STUPID_NC(pThread);
        stMutexLock(&pThread->lock);
        const bool res = pThread->is_running;
        stMutexUnlock(&pThread->lock);
        return res;
}

/**
 * Checks if a thread is paused.
 * @param pThread Pointer to a thread.
 * @return True if the thread is paused.
 */
static STUPID_INLINE bool stThreadIsPaused(StThread *pThread)
{
        STUPID_NC(pThread);
        stMutexLock(&pThread->lock);
        const bool res = pThread->is_paused;
        stMutexUnlock(&pThread->lock);
        return res;
}

/**
 * Checks if a thread is paused.
 * @param pThread Pointer to a thread.
 * @return True if the thread is paused.
 */
static STUPID_INLINE bool stThreadIsInJob(StThread *pThread)
{
        STUPID_NC(pThread);
        stMutexLock(&pThread->lock);
        const bool res = pThread->is_in_job;
        stMutexUnlock(&pThread->lock);
        return res;
}

/**
 * Checks if a thread is joined.
 * @param pThread Pointer to a thread.
 * @return True if the thread is already joined.
 * @note Also returns true if the thread hasent been started.
 */
static STUPID_INLINE bool stThreadIsJoined(StThread *pThread)
{
        STUPID_NC(pThread);
        stMutexLock(&pThread->lock);
        const bool res = pThread->is_joined;
        stMutexUnlock(&pThread->lock);
        return res;
}

/**
 * Checks how long a thread has existed.
 * @param pThread Pointer to a thread.
 * @return The lifetime of a thread.
 */
static STUPID_INLINE f64 stThreadLifetime(const StThread *pThread)
{
        STUPID_NC(pThread);
        return stGetTime() - pThread->clock.start_time;
}

/**
 * Checks how long a thread has spent in a job.
 * @param pThread Pointer to a thread.
 * @return The number of seconds spent in the current job.
 */
static STUPID_INLINE f64 stThreadJobElapsed(const StThread *pThread)
{
        STUPID_NC(pThread);
        if (!pThread->is_running) return 0.0;
        return stGetClockElapsed(&pThread->clock);
}

