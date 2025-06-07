#include "stupid/thread.h"
#include "stupid/clock.h"
#include "stupid/memory.h"
#include "stupid/assert.h"

#include <pthread.h>
#include <signal.h>
#include <threads.h>

/**
 * Each thread runs this function, which just manages the thread's job queue.
 * @param _pThread Pointer to the thread running this function.
 * @return This doesnt actually return anything, but pthread_create() expects it to return a (void *).
 * @note _pThread is a (void *) instead of a (Thread *) because pthread_create() requires a (void *) argument.
 * @see stThreadStart
 */
static void *THREAD(void *_pThread)
{
	STUPID_THREAD_COUNT++;
	STUPID_THREAD_ID = STUPID_THREAD_COUNT;

	// this is just to avoid casting it each time
	StThread *pThread = (StThread *)_pThread;
	pThread->id = STUPID_THREAD_ID;
	pThread->is_running = true;

	// keep executing the next job as long as there is one
	while (stMemLength(pThread->pJobs)) {
		// exit if requested
		if (pThread->exit_requested)
			goto thread_exit;

		stMutexLock(&pThread->lock);
		StThreadJob job = {0};
		stMemRemove(pThread->pJobs, 0, &job);
		STUPID_NC(job.pfn);
		stMutexUnlock(&pThread->lock);

		// pause if requested
		if (pThread->pause_requested) {
			STUPID_LOG_TRACE("thread %lu paused", STUPID_THREAD_ID);
			pThread->is_paused = true;

			// wait as long as the thread is paused
			while (pThread->pause_requested) {
				stSleepu(100);
				if (pThread->exit_requested)
					goto thread_exit;
			}

			if (pThread->exit_requested)
				goto thread_exit;
		}

		stClockUpdate(&pThread->clock);

		pThread->is_in_job = true;

		// execute the next job
		job.pfn(job.pArg);
		stFenceSignal(&job.finished);
		pThread->is_in_job = false;
	}

	// OH NO ITS A GOTO STATEMENT
thread_exit:
	stMutexLock(&pThread->lock);
	pThread->is_in_job       = false;
	pThread->pause_requested = false;
	pThread->is_paused       = false;
	pThread->is_running      = false;

	if (pThread->exit_requested)
		STUPID_LOG_TRACE("thread %lu exited", STUPID_THREAD_ID);
	else
		STUPID_LOG_TRACE("thread %lu finished", STUPID_THREAD_ID);

	pThread->exit_requested = false;
	stMutexUnlock(&pThread->lock);

	// this is supposedly better than return at cleaning up thread stuff
	pthread_exit(NULL);
}

StThread *stThreadCreate(void)
{
	StThread *pThread  = stMemAlloc(StThread, 1);
	pThread->pHandle   = stMemAllocNL(pthread_t, 1);
	pThread->pJobs     = stMemAllocNL(StThreadJob, 16);
	pThread->is_joined = true;
	STUPID_LOG_TRACE("thread %lu created", pThread->id);

	stClockStart(&pThread->clock);

	return pThread;
}

void (stThreadAddPFN)(StThread *pThread, StThreadArg *pArg, const StPFN_thread pfn, const char *job_name STUPID_DBG_PROTO_PARAMS)
{
	STUPID_NC(pThread);
	STUPID_NC(pfn);
	STUPID_NC(job_name);

	stMutexLock(&pThread->lock);

	StThreadJob job = {0};
	job.pfn = pfn;
	job.pArg = pArg;
	stFenceReset(&job.finished);
	stMemAppend(pThread->pJobs, job);

	stMutexUnlock(&pThread->lock);

	STUPID_LOG_TRACEFN("job %p '%s' added to thread %lu job queue", pfn, job_name, pThread->id);
}

bool (stThreadStart)(StThread *pThread STUPID_DBG_PROTO_PARAMS)
{
	STUPID_NC(pThread);

	stMutexLock(&pThread->lock);

	// exit if there are no jobs to execute
	if (stMemLength(pThread->pJobs) == 0) {
		STUPID_LOG_ERROR("stThreadStart(): thread %lu has no jobs", pThread->id);
		stMutexUnlock(&pThread->lock);
		return false;
	}

	// exit of the thread is already started
	if (pThread->is_running) {
		STUPID_LOG_ERROR("stThreadStart(): thread %lu is already started", pThread->id);
		stMutexUnlock(&pThread->lock);
		return false;
	}

	pThread->is_in_job  = false;
	pThread->is_joined  = false;
	pThread->is_running = true;
	stMutexUnlock(&pThread->lock);

	// try to start the thread
	STUPID_ASSERT(pthread_create(pThread->pHandle, NULL, THREAD, pThread) == 0, "failed to create pthread");

	STUPID_LOG_TRACEFN("thread %lu started", pThread->id);

	return true;
}

void (stThreadDestroy)(StThread *pThread STUPID_DBG_PROTO_PARAMS)
{
	STUPID_NC(pThread);

	stMemDeallocNL(pThread->pHandle);
	stMemDeallocNL(pThread->pJobs);

	STUPID_LOG_TRACEFN("thread %lu destroyed", pThread->id);

	stMemDealloc(pThread);
}

bool (stThreadJoin)(StThread *pThread, const u64 timeout STUPID_DBG_PROTO_PARAMS)
{
	STUPID_NC(pThread);
	STUPID_NC(pThread->pHandle);

	if (pThread->is_joined) {
		STUPID_LOG_WARN("stThreadJoin(): thread %lu already joined", pThread->id);
		return true;
	}

	if (!pThread->is_running) return true;

	stThreadRequestExit(pThread);

	pThread->pause_requested = false;
	pThread->is_paused = false;

	u64 time_waited = 0;

	// wait for the thread to finish until the timeout has ended
	while (stThreadIsInJob(pThread)) {
		if (time_waited >= timeout) {
			// go to pthread_join thereby waiting forever if the timeout is 0
			if (timeout == 0) break;

			// otherwise kill the thread
			if (pthread_cancel(*((pthread_t *)pThread->pHandle)) != 0) {
				STUPID_LOG_CRITICAL("failed to cancel thread %p resorting to violence (SIGKILL)", pThread);
				stSleep(100);
				pthread_kill(*(pthread_t *)pThread->pHandle, SIGKILL);
			}

			pThread->is_running = false;
			STUPID_LOG_ERROR("thread %lu forcibly closed since it didnt finish before the timeout %lu", pThread->id, timeout);
			return false;
		}
		stSleep(1);
		time_waited++;
	}

	// rejoin the thread
	pthread_join(*((pthread_t *)pThread->pHandle), NULL);
	pThread->is_running = false;
	STUPID_LOG_TRACEFN("thread %lu joined", pThread->id);
	return true;
}

