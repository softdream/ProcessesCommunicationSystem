#include "time_stamp.h"

namespace pcs{

long getCurrentTime_s()
{
        time_t sectime;
        sectime = time( NULL );
        return sectime;
}

long getCurrentTime_ms()
{
        struct timeval tv;
        gettimeofday( &tv, NULL );
        return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

long getCurrentTime_us()
{
        struct timeval tv;
        gettimeofday( &tv, NULL );
        return tv.tv_sec * 1000000 + tv.tv_usec;
}

long getCurrentRelativeTime_s()
{
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	return ts.tv_sec;
}

long getCurrentRelativeTime_ns()
{
	struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
	return ts.tv_nsec;
}

}
