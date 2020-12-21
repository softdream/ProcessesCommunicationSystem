#include "timer_manage.h"

namespace pcs{

TimerManage::TimerManage() : Timer(),
                             initialTimeSec_(0),
                             initialTimeNsec_(0),
                             intervalTimeSec_(0),
                             intervalTimeNsec_(0),
                             isRunning(false)

{
        createTimer();
        timerEvent.fd = getTimerFd();
        timerEvent.event = EPOLLIN;
        timerEvent.arg = NULL;
        //timerEvent.callback = ;
}


TimerManage::TimerManage( void *( *callback )( int fd, void *arg ) ) : Timer(),	
			     initialTimeSec_(0),
			     initialTimeNsec_(0),
			     intervalTimeSec_(0),
			     intervalTimeNsec_(0),
			     isRunning(false)

{
	createTimer();
	timerEvent.fd = getTimerFd();
	timerEvent.event = EPOLLIN;
	timerEvent.arg = NULL;
	timerEvent.callback = callback;
}

TimerManage::TimerManage( int initialTimeSec, long initialTimeNsec, int intervalTimeSec, long intervalTimeNsec, void *( *callback )( int fd, void *arg ) ) : Timer(), 
			    initialTimeSec_( initialTimeSec ),
                            initialTimeNsec_( initialTimeNsec ),
                            intervalTimeSec_( initialTimeNsec ),
                            intervalTimeNsec_( intervalTimeNsec ),
                            isRunning(false)
{
        createTimer();
        timerEvent.fd = getTimerFd();
        timerEvent.event = EPOLLIN;
        timerEvent.arg = NULL;
        timerEvent.callback = callback;

}
 
void TimerManage::start()
{
	isRunning = true;
	setTimer( initialTimeSec_, initialTimeNsec_, intervalTimeSec_, intervalTimeNsec_ );
	
	eventBase.addEvent( timerEvent );
	
	while( isRunning ){
		eventBase.dispatcher();
	}
}

void TimerManage::stop()
{
	if( isRunning )
		isRunning = false;
}

}
