#pragma once

#include "stupid/common.h"
#include "stupid/clock.h"
#include "stupid/logger.h"
#include "stupid/assert.h"
#include "stupid/memory.h"

#include <stdatomic.h>
#include <threads.h>

// great power great responsibility
#include <setjmp.h>

/// Thread ID.
typedef u64 StThreadID;

/// Thread local identifier (used to avoid calling pthread_self() over and over).
static _Thread_local StThreadID STUPID_THREAD_ID = 1;

static STUPID_UNUSED STUPID_ATOMIC u64 STUPID_THREAD_COUNT = 1;

typedef enum st_thread_priority {
	/// Low latency, and generally lower overhead than the
	/// other options at the cost of higher idle cpu usage.
	ST_THREAD_PRIORITY_HIGH,

	/// Average latency, and somewhat lower idle cpu usage than ST_THREAD_PRIORITY_HIGH.
	ST_THREAD_PRIORITY_MED,

	/// Medium latency, and low idle cpu usage.
	ST_THREAD_PRIORITY_LOW,

	/// High latency, and practically no idle cpu usage.
	/// @note Dont use this for anything important.
	ST_THREAD_PRIORITY_WORTHLESS,

	/// THIS IS NOT AN ACTUAL PRIORITY
	ST_THREAD_PRIORITY_MAX
} st_thread_priority;


/// Basically a worse version of pthread_mutex_t.
typedef struct StMutex {
        /// Total waiting for the mutex to be unlocked.
        STUPID_ATOMIC usize waiting;

        /// ID of the thread which owns the mutex.
        StThreadID owner;

	/// Where the magic happens.
        STUPID_ATOMIC bool lock;
} StMutex;

/// This is just an boolean that threads wait for until its set to true.
typedef struct StFence {
	/// Where the magic happens.
	STUPID_ATOMIC bool lock;

	/// Number of threads waiting for this fence.
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

/**
 * Locks a mutex.
 * @param pMutex Pointer to a mutex.
 * @note Blocks while the mutex is already locked.
 * @note If the engine becomes unresponsive, this could be the cuplrit.
 */
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
	while (pFence->waiting) {
		stSleepu(1);
	}
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
	while (!stGetAtomicBoolState(&pFence->lock)) {
		stSleepu(1);
	}
	atomic_fetch_sub(&pFence->waiting, 1);
}

/// Thing for the thread to do.
typedef struct StThreadJob {
        /// Instruction to jump to.
	jmp_buf jmp;

	/// Signaled when the job is done.
	StFence finished;
} StThreadJob;

/**
 * In order to use a thread, add a job to it, then start it.
 * Once the thread is started, it will do all the jobs in its job queue until there are no jobs left.
 * It can be cancelled, or paused, and jobs can be added to it while its started.
 * @note Dont create an entire thread for a single short term job (i.e. less than 0.1 seconds before exiting).
 * @see stThreadCreate, stThreadDestroy, StThreadJob
 */
typedef STUPID_A32 struct StThread {
	/// Custom stack pointer when inside of a job.
	u8 stack[sizeof(StKb) * 64];

        /// Handle for the thread.
        void *pHandle;

	/// Scheduling priority for this thread.
	/// @note If theres a backlog of jobs, the thread will increase in scheduling priority.
	/// Theres no easy way to prevent this, because theres no reason to.
	st_thread_priority priority;

        /// Job queue.
	/// @note Jobs are popped from the bottom of the stack.
        StThreadJob *pJobs;

        /// Keeps track of time in the thread.
        StClock clock;

	/// Keeps track of how long the thread has worked on jobs.
	StClock work_timer;

	/// How long the thread has worked on jobs.
	f64 time_worked;

        /// Mutex protecting the thread from concurrent access.
        StMutex lock;

	/// Used to jump back to the thread loop after finishing a job.
	jmp_buf loop;

	/// ID for this thread.
	/// @note This will almost certainly end up being the number of threads when this thread was created.
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

	/// Temporary thread job placeholder used when adding a new job to the queue.
	StThreadJob tmp_job;
} StThread;

/**
 * Creates a new thread.
 * @param priority Basically determines how often this thread will check for new jobs.
 * @return A pointer to a thread which must be destroyed stThreadDestroy().
 * @see stThreadDestroy, STUPID_THREAD_JOB, StThread
 */
StThread *(stThreadCreate)(st_thread_priority priority STUPID_DBG_PROTO_PARAMS);

/**
 * Creates a new thread.
 * @param priority Basically determines how often this thread will check for new jobs (higher = faster).
 * @return A pointer to a thread which must be destroyed stThreadDestroy().
 * @see stThreadDestroy, STUPID_THREAD_JOB, StThread
 */
#define stThreadCreate(priority) (stThreadCreate)(priority STUPID_DBG_PARAMS)

/**
 * Destroys a thread.
 * @param pThread Pointer to a thread.
 * @note A started thread must be joined before destroying it.
 * @see stThreadCreate, stThreadJoin, StThread.
 */
void (stThreadDestroy)(StThread *pThread STUPID_DBG_PROTO_PARAMS);

/**
 * Destroys a thread.
 * @param pThread Pointer to a thread.
 * @note A started thread must be joined before destroying it.
 * @see stThreadJoin, StThread.
 */
#define stThreadDestroy(pThread)   (stThreadDestroy)(pThread STUPID_DBG_PARAMS)

/**
 * Destroys a thread.
 * @param pThread Pointer to a thread.
 * @note A started thread must be joined before destroying it.
 * @note Does not print logs.
 * @see stThreadJoin, StThread.
 */
#define stThreadDestroyNL(pThread) (stThreadDestroy)(pThread STUPID_DBG_PARAMS_NL)

/**
 * Waits for either the thread to finish its job queue, or the
 * timeout to be reached, and then rejoins it with the main thread.
 * @param pThread Pointer to a thread.
 * @param timeout Number of milliseconds to wait before forcing the thread to close.
 * @return True if the thread finished normally, false if the thread had to be closed forcibly.
 * @see stThreadCreate, stThreadRequestExit, stThreadDestroy
 */
bool (stThreadJoin)(StThread *pThread, const u64 timeout STUPID_DBG_PROTO_PARAMS);

/**
 * Waits for either the thread to finish its job queue or the timeout to be reached, and then rejoins it with the main thread.
 * @param pThread Pointer to a thread.
 * @param timeout Number of milliseconds to wait before forcing the thread to close.
 * @return True if the thread finished normally, false if the thread had to be closed forcibly.
 * @see stThreadCreate, stThreadRequestExit, stThreadDestroy
 */
#define stThreadJoin(pThread, timeout)   (stThreadJoin)(pThread, timeout STUPID_DBG_PARAMS)

/**
 * Waits for either the thread to finish its job queue or the timeout to be reached, and then rejoins it with the main thread.
 * @param pThread Pointer to a thread.
 * @param timeout Number of milliseconds to wait before forcing the thread to close.
 * @return True if the thread finished normally, false if the thread had to be closed forcibly.
 * @note Dont forcibly close a thread unless you have to.
 * @note Does not print logs.
 * @see stThreadCreate, stThreadRequestExit, stThreadDestroy
 */
#define stThreadJoinNL(pThread, timeout) (stThreadJoin)(pThread, timeout STUPID_DBG_PARAMS_NL)

/**
 * Enables or disables pause for a thread.
 * @param pThread Pointer to a thread.
 * @param state True to request that the thread pauses, false to unpause it.
 * @note If pause is set to true, then the thread will pause once it is no longer in a function.
 * @note A thread can be joined while it is paused.
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
 */
#define stThreadSetPause(pThread, state)   (stThreadSetPause)(pThread, state STUPID_DBG_PARAMS)

/**
 * Enables or disables pause for a thread.
 * @param pThread Pointer to a thread.
 * @param state True to request that the thread pauses, false to unpause it.
 * @note If pause is set to true, then the thread will pause once it is no longer in a function.
 * @note A thread can be joined while it is paused.
 * @note Does not print logs.
 */
#define stThreadSetPauseNL(pThread, state) (stThreadSetPause)(pThread, state STUPID_DBG_PARAMS_NL)

/**
 * Makes a thread exit when it is no longer in a function.
 * @param pThread Pointer to a thread.
 * @note The thread must be joined after doing this.
 * @see stThreadJoin, stThreadDestroy
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
 * @note The thread must be joined after doing this.
 * @see stThreadJoin, stThreadDestroy
 */
#define stThreadRequestExit(pThread)   (stThreadRequestExit)(pThread STUPID_DBG_PARAMS)

/**
 * Makes a thread exit when it is no longer in a function.
 * @param pThread Pointer to a thread.
 * @note The thread must be joined after doing this.
 * @note Does not print logs.
 * @see stThreadJoin, stThreadDestroy
 */
#define stThreadRequestExitNL(pThread) (stThreadRequestExit)(pThread STUPID_DBG_PARAMS_NL)

/**
 * Checks if a thread is running.
 * @param pThread Pointer to a thread.
 * @return True if the thread is currently running.
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
 * Checks how long a thread has existed.
 * @param pThread Pointer to a thread.
 * @return The lifetime of a thread.
 */
static STUPID_INLINE f64 stThreadLifetime(const StThread *pThread)
{
        STUPID_NC(pThread);
        return pThread->clock.lifetime;
}

/**
 * Checks how long a thread has spent in a job.
 * @param pThread Pointer to a thread.
 * @return The number of seconds spent in the current job.
 */
static STUPID_INLINE f64 stThreadJobElapsed(const StThread *pThread)
{
        STUPID_NC(pThread);
        if (!pThread->is_in_job) return 0.0;
        return stGetClockElapsed(&pThread->work_timer);
}

/**
 * Waits for a thread to finish its current job.
 * @param pThread Pointer to a thread.
 */
static STUPID_INLINE void stThreadWaitForJob(const StThread *pThread)
{
	STUPID_NC(pThread);
	while (pThread->is_in_job)
		stSleepu(1);
}

/**
 * Waits for a thread to finish all jobs in its queue.
 * @param pThread Pointer to a thread.
 */
static STUPID_INLINE void stThreadWaitForAllJobs(const StThread *pThread)
{
	STUPID_NC(pThread);
	while (stMemLength(pThread->pJobs) > 0)
		stSleepu(2);
}

#define STUPID_THREAD_START(pThread, label)\
do {\
	stMutexLock(&(pThread)->lock);\
	if (setjmp((pThread)->tmp_job.jmp) == 0) goto label;\
	__asm__ __volatile__ ("mov %0, %%rsp" : : "r" ((pThread)->stack + sizeof((pThread)->stack)));\
} while (0)

#define STUPID_THREAD_STOP(pThread, label)\
do {\
	longjmp((pThread)->loop, 0);\
	label:\
	stMemAppend((pThread)->pJobs, (pThread)->tmp_job);\
	stMutexUnlock(&(pThread)->lock);\
} while (0)

#define STUPID_THREAD_JOB(pThread, label, job)\
do {\
	STUPID_THREAD_START(pThread, label);\
	{\
		job\
	}\
	STUPID_THREAD_STOP(pThread, label);\
} while (0)

