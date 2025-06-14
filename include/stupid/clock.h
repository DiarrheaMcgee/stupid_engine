#pragma once

#include "stupid/common.h"
#include "stupid/math/basic.h"

#include <bits/time.h>
#include <time.h>

#define STUPID_SEC_TO_MS(t) _Generic ((t), f32: ((t) * 1000.0), f64: ((t) * 1000.0), default: ((t) * 1000))
#define STUPID_SEC_TO_US(t) _Generic ((t), f32: ((t) * 1000.0 * 1000.0), f64: ((t) * 1000.0 * 1000.0), default: ((t) * 1000 * 1000))
#define STUPID_SEC_TO_NS(t) _Generic ((t), f32: ((t) * 1000.0 * 1000.0 * 1000.0), f64: ((t) * 1000.0 * 1000.0 * 1000.0), default: ((t) * 1000 * 1000 * 1000))

#define STUPID_MS_TO_SEC(t) ((t) * 0.001)
#define STUPID_MS_TO_US(t)  _Generic ((t), f32: ((t) * 1000.0), f64: ((t) * 1000.0), default: ((t) * 1000))
#define STUPID_MS_TO_NS(t)  _Generic ((t), f32: ((t) * 1000.0 * 1000.0), f64: ((t) * 1000.0 * 1000.0), default: ((t) * 1000 * 1000))

#define STUPID_US_TO_SEC(t) ((t) * 0.001 * 0.001)
#define STUPID_US_TO_MS(t)  _Generic ((t), f32: ((t) * 0.001), f64: ((t) * 0.001), default: ((t) / 1000))
#define STUPID_US_TO_NS(t)  _Generic ((t), f32: ((t) * 1000.0), f64: ((t) * 1000.0), default: ((t) * 1000))

#define STUPID_NS_TO_SEC(t) ((t) * 0.001 * 0.001 * 0.001)
#define STUPID_NS_TO_MS(t)  _Generic ((t), f32: ((t) * 0.001 * 0.001), f64: ((t) * 0.001 * 0.001), default: ((t) / 1000 / 1000))
#define STUPID_NS_TO_US(t)  _Generic ((t), f32: ((t) * 0.001), f64: ((t) * 0.001), default: ((t) / 1000))

/// Basically a stopwatch.
typedef struct STUPID_A8 StClock {
	/// Time the clock was started.
	f64 start_time;

	/// Time since the clock was started (updated on clockUpdate()).
	f64 lifetime;

	/// The time the clock was last updated (updated on clockUpdate()).
	f64 update_time;
} StClock;

/**
 * Gets the current time in seconds.
 * @return The absolute time in seconds.
 * @note Latency is probably around 250NS.
 */
static STUPID_UNUSED STUPID_NOINLINE f64 stGetTime(void)
{
        struct timespec now = {0};
        clock_gettime(CLOCK_MONOTONIC_RAW, &now);
        return (f64)now.tv_sec + STUPID_NS_TO_SEC((f64)now.tv_nsec);
}

/**
 * Gets the resolution of the system clock.
 * @return The resolution of the system clock.
 * @note 1NS/0.000000001 is optimal (and just about as fast as it gets).
 */
static STUPID_INLINE u64 stGetClockResolution(void)
{
        struct timespec resolution = {0};
        if (clock_getres(CLOCK_MONOTONIC_RAW, &resolution) != 0) return STUPID_US_TO_SEC(1);
        return resolution.tv_nsec + STUPID_SEC_TO_NS(resolution.tv_sec);
}

/**
 * Sleeps for a specified number of milliseconds.
 * @param ms Number of milliseconds to sleep for.
 * @note Not inline to reduce executable size.
 */
static STUPID_UNUSED STUPID_NOINLINE void stSleep(const u64 ms)
{
	if (ms == 0) return;

	// for some reason longjmp breaks this unless its static
	// TODO: figure out why
        static struct timespec ts = {0};

	ts.tv_sec = ms / 1000;
	ts.tv_nsec = (ms % 1000) * 1000;

        nanosleep(&ts, NULL);
}

/**
 * Sleeps for a specified number of microseconds.
 * @param us Number of microseconds to sleep for.
 * @note Not inline to reduce executable size.
 */
static STUPID_UNUSED STUPID_NOINLINE void stSleepu(const u64 us)
{
	if (us == 0) return;

	// for some reason longjmp breaks this unless its static
	// TODO: figure out why
        static struct timespec ts = {0};

	ts.tv_sec = us / (1000 * 1000);
	ts.tv_nsec = (us % (1000 * 1000)) * 1000;

        nanosleep(&ts, NULL);
}

/**
 * @brief Updates a clock.
 * Sets update_time to the current time, and updates the lifetime.
 * @param c Pointer to a clock.
 */
static STUPID_INLINE void stClockUpdate(StClock *c)
{
        if (c->start_time != 0) {
                const f64 time = stGetTime();
                c->lifetime += time - c->update_time;
                c->update_time = time;
        }
}

/**
 * Gets the time since the clock was updated.
 * @param c Pointer to a clock.
 * @return Time since the last update.
 */
static STUPID_INLINE f64 stGetClockElapsed(const StClock *c)
{
        return stGetTime() - c->update_time;
}

/**
 * @brief Initializes a clock.
 * Sets lifetime to 0, and updates the clock.
 * @param c Pointer to a clock.
 * @note Can also be used to reset a clock.
 */
static STUPID_INLINE void stClockStart(StClock *c)
{
        c->lifetime    = 0.0;
        c->start_time  = stGetTime();
        c->update_time = c->start_time;
}
