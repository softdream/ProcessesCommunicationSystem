#ifndef __TIMER_MANAGE_H_
#define __TIMER_MANAGE_H_

#include "timer.h"
#include "EpollEvent.h"


namespace pcs
{

class TimerManage : public Timer
{
public:
	TimerManage();
	TimerManage( void *( *callback )( int fd, void *arg ) );
	TimerManage( int initialTimeSec, long initialTimeNsec, int intervalTimeSec, long intervalTimeNsec, void *( *callback )( int fd, void *arg ) );
	~TimerManage(){}

	void start();
	void stop();

private:
	int initialTimeSec_;
	long initialTimeNsec_;
	int intervalTimeSec_;
	long intervalTimeNsec_;
	bool isRunning;
	
	EpollEvent eventBase;
	Event timerEvent;
};

}


#endif
