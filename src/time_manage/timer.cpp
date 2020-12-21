#include "timer.h"

namespace pcs
{

using namespace std::placeholders;

Timer::Timer() : timerFd(0), timeNow(0), timePrevious(0), isRunning(false)
{
	
}

bool Timer::setTimer( int initialTimeSec, long initialTimeNsec, int intervalTimeSec, long intervalTimeNsec )
{
	struct itimerspec itimer;
	itimer.it_value.tv_sec = initialTimeSec;
	itimer.it_value.tv_nsec = initialTimeNsec;

	itimer.it_interval.tv_sec = intervalTimeSec;
	itimer.it_interval.tv_nsec = intervalTimeNsec;

	int ret = timerfd_settime( timerFd, 0, &itimer, NULL );
	if( ret == -1 ){
		std::cerr<<"timerfd_settime failed ..."<<std::endl;
		return false;
	}

	std::cerr<<"timerfd_settime: "<<timerFd<<std::endl;
	return true;
}

bool Timer::createTimer()
{
	timerFd = timerfd_create( CLOCK_MONOTONIC, 0 );
	if( timerFd == -1 ){
		std::cerr<<"timerfd_create failed ..."<<std::endl;
		return false;
	}
	
	std::cerr<<"timerfd_create: "<<timerFd<<std::endl;
	return true;	
}

void Timer::handleRead()
{
	uint64_t timerfd_read;
	int ret = read( timerFd, &timerfd_read, sizeof( uint64_t ) );

	if( ret != sizeof( uint64_t ) )	{
		//std::cerr<<"timerfd_read: "<<ret<<std::endl;
	}
	else{
		std::cout<<"timerfd_read: "<<timerfd_read<<std::endl;
	}
}

uint64_t Timer::getClockTime()
{
	struct timespec now;
	int ret = clock_gettime(CLOCK_REALTIME, &now);//获取时钟时间
	if( ret == -1 ){
		std::cerr<<"gettime failed ..."<<std::endl;
		return false;
	}
	
	return now.tv_nsec + now.tv_sec * 1000000000;
}

void Timer::closeTimerFd()
{
	close( timerFd );
}

void Timer::caculateTimeDiff()
{
	uint64_t timerfd_read;
	//uint64_t timeNow = 0;
	//uint64_t timePrevious = 0;

        int ret = read( timerFd, &timerfd_read, sizeof( uint64_t ) );

        if( ret != sizeof( uint64_t ) ) {
                //std::cerr<<"timerfd_read: "<<ret<<std::endl;
        }
        else{
                std::cout<<"timerfd_read: "<<timerfd_read<<std::endl;
		timeNow = getClockTime();
		uint64_t timeDiff = timeNow - timePrevious;
		std::cout<<"timeDiff = "<<timeDiff<<std::endl;
		timePrevious = timeNow;
        }

}

void Timer::start()
{
	createTimer();
	setTimer( 1, 0, 1, 0 );

	event.fd = timerFd;
	event.event = EPOLLIN;
	event.arg = NULL;
	
	FUNC cb = std::bind( &Timer::timerCallback, this, _1, _2 );
	event.callback = cb;

        eventbase.addEvent( event );
	
	isRunning = true;

        while(isRunning){
                eventbase.dispatcher();
        }

}

void Timer::start( int initialTimeSec, long initialTimeNsec, int intervalTimeSec, long intervalTimeNsec, FUNC &callback )
{
	createTimer();
	setTimer( initialTimeSec,initialTimeNsec, intervalTimeSec,intervalTimeNsec);
	event.fd = timerFd;
        event.event = EPOLLIN;
        event.arg = NULL;
	event.callback = callback;
	
	isRunning = true;
	
        while(isRunning){
                eventbase.dispatcher();
        }
}

void Timer::stop()
{
	if( isRunning )
		isRunning = false;
}


void *Timer::timerCallback( int fd, void *arg )
{
        uint64_t uread;
        int ret = read( fd, &uread, sizeof( uint64_t ) );
        if( ret == 8 ){
		std::cout<<"fd == "<<fd<<std::endl; 
		std::cout<<"uread == "<<uread<<std::endl;
        }
}

}























