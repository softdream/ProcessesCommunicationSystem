#ifndef __TIMER_H_
#define __TIMER_H_

#include <iostream>
#include <sys/timerfd.h>
#include <unistd.h>
#include <sys/time.h>

#include "EpollEvent.h"

namespace pcs
{

class Timer
{
public:
	Timer();

	Timer( const Timer &obj ): timerFd( obj.timerFd )
	{

	}

	~Timer()
	{
		close( timerFd );
	}
	
	bool setTimer( int initialTimeSec, long initialTimeNsec, int intervalTimeSec, long intervalTimeNsec  );
	bool createTimer();

	void handleRead();

	uint64_t getClockTime();

	const int getTimerFd()
	{
		return timerFd;
	}
	
	void closeTimerFd();
	
	void caculateTimeDiff();
	
	void start();
	void start( int initialTimeSec, long initialTimeNsec, int intervalTimeSec, long intervalTimeNsec, FUNC &callback );
	
	void stop();
	
	void *timerCallback( int fd, void *arg );

private:
	int timerFd;	
	uint64_t timeNow = 0;
        uint64_t timePrevious = 0;

	EpollEvent eventbase;
	Event event;
	
	bool isRunning;
};

}


#endif
