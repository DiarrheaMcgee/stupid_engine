#include "asserts.h"

void _stAssertLog(const char *expr, const char *message, const char *file, const int line)
{
        STUPID_LOG_FATAL("assertion failure%s: '%s' %lf %s:%d", expr, message, stGetTime(), file, line);
}

