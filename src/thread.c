#include "stupid/thread.h"
#include "stupid/clock.h"
#include "stupid/memory.h"
#include "stupid/assert.h"

#include <pthread.h>
#include <setjmp.h>
#include <signal.h>
#include <stddef.h>
#include <threads.h>

static u64 thread_wait_times[ST_THREAD_PRIORITY_MAX] = {
	1, 2, 5, 20
};

static u64 thread_paused_wait_times[ST_THREAD_PRIORITY_MAX] = {
	5, 12, 20, 40
};

static void *THREAD(void *_pThread)
{
	StThread *pThread = _pThread;
	STUPID_THREAD_COUNT++;
	STUPID_THREAD_ID = STUPID_THREAD_COUNT;
	pThread->id = STUPID_THREAD_ID;

	stClockStart(&pThread->work_timer);

	setjmp(pThread->loop);

	if (pThread->is_in_job)
		pThread->time_worked += stGetClockElapsed(&pThread->work_timer);

	pThread->is_in_job = false;

	while (true) {
		if (STUPID_UNLIKELY(pThread->pause_requested)) {
			STUPID_LOG_TRACE("thread %zu paused", STUPID_THREAD_ID);
			stClockUpdate(&pThread->clock);
			pThread->is_paused = true;

			while (STUPID_LIKELY(pThread->pause_requested)) {
				if (STUPID_UNLIKELY(pThread->exit_requested)) {
					STUPID_LOG_TRACE("thread %zu exiting (idiot index %lf)", STUPID_THREAD_ID, pThread->clock.lifetime / pThread->time_worked);
					pThread->is_running = false;
					pthread_exit(NULL);
				}

				stSleepu(thread_paused_wait_times[pThread->priority]);
			}

			pThread->is_paused = false;
			STUPID_LOG_TRACE("thread %zu unpaused after %lf", STUPID_THREAD_ID, stGetClockElapsed(&pThread->clock));
		}

		if (STUPID_UNLIKELY(pThread->exit_requested)) {
			STUPID_LOG_TRACE("thread %zu exiting (idiot index %lf)", STUPID_THREAD_ID, pThread->clock.lifetime / pThread->time_worked);
			pthread_exit(NULL);
		}

		if (stMemLength(pThread->pJobs) > 0) {
			stMutexLock(&pThread->lock);
			pThread->is_in_job = true;
			StThreadJob job = {0};
			stMemRemove(pThread->pJobs, 0, &job);
			stMutexUnlock(&pThread->lock);
			stClockUpdate(&pThread->work_timer);
			longjmp(job.jmp, -1);
		}

		stSleepu(thread_wait_times[pThread->priority]);
		stClockUpdate(&pThread->clock);
	}
}

StThread *(stThreadCreate)(st_thread_priority priority STUPID_DBG_PROTO_PARAMS)
{
	StThread *pThread   = stMemAllocNL(StThread, 1);
	pThread->pHandle    = stMemAllocNL(pthread_t, 1);
	pThread->pJobs      = stMemAllocNL(StThreadJob, 16);
	pThread->is_running = true;
	STUPID_LOG_TRACEFN("thread %lu created with priority %u", pThread->id, priority);

	stClockStart(&pThread->clock);
	STUPID_ASSERT(pthread_create(pThread->pHandle, NULL, THREAD, pThread) == 0, "failed to create pthread");

	return pThread;
}

bool (stThreadDestroy)(StThread *pThread, const u64 timeout STUPID_DBG_PROTO_PARAMS)
{
	STUPID_NC(pThread);
	STUPID_NC(pThread->pHandle);

	stThreadRequestExit(pThread);
	pThread->exit_requested = true;
	pThread->pause_requested = false;

	u64 time_waited = 0;

	// wait for the thread to finish until the timeout has ended
	while (pThread->is_in_job) {
		if (time_waited >= timeout) {
			// go to pthread_join thereby waiting forever if the timeout is 0
			if (timeout == 0) break;

			// otherwise kill the thread
			if (pthread_cancel(*((pthread_t *)pThread->pHandle)) != 0) {
				STUPID_LOG_CRITICAL("failed to cancel thread %lu resorting to violence (SIGKILL)", pThread->id);
				stSleep(100);
				pthread_kill(*(pthread_t *)pThread->pHandle, SIGKILL);
			}

			STUPID_LOG_ERROR("thread %lu forcibly closed since it didnt finish before the timeout %lu", pThread->id, timeout);

			stMemDeallocNL(pThread->pHandle);
			stMemDeallocNL(pThread->pJobs);

			return false;
		}
		stSleep(1);
		time_waited++;
	}

	// rejoin the thread
	pthread_join(*((pthread_t *)pThread->pHandle), NULL);

	stMemDeallocNL(pThread->pHandle);
	stMemDeallocNL(pThread->pJobs);

	STUPID_LOG_TRACEFN("thread %lu destroyed", pThread->id);

	stMemDeallocNL(pThread);

	return true;
}

