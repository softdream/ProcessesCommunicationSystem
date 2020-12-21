#ifndef __TIME_STAMP_H_
#define __TIME_STAMP_H_

#include <sys/time.h>
#include <time.h>


namespace pcs{
long getCurrentTime_s();

long getCurrentTime_ms();

long getCurrentTime_us();

long getCurrentRelativeTime_s();

long getCurrentRelativeTime_ns();

}


#endif
