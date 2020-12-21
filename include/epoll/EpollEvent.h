#ifndef __EPOLLEVENT_H_
#define __EPOLLEVENT_H_

#include <iostream>
#include <sys/epoll.h>
#include <map>
#include "IEvent.h"

class EpollEvent : public IEvent {
public:
	EpollEvent(): epollCreateSize(16) {
		initEvent();	
	}
	EpollEvent( int createSize_ )
	{
		if( createSize_ < 16 ) createSize_ = 16;
		epollCreateSize = createSize_;
		initEvent();	
	}
	
	/* copy constructor */
	EpollEvent( const EpollEvent &obj ): epollCreateSize( obj.epollCreateSize ),	
					epollFd( obj.epollFd ),
					events( obj.events )
	{

	}
	

	/*  */
	EpollEvent& operator=( const EpollEvent &other )
	{
		if( this == &other ) return *this;
			
		epollCreateSize = other.epollCreateSize;
		epollFd = other.epollFd;
		events = other.events;
		
		return *this;
	}

	virtual int addEvent( const Event &event );
	virtual int delEvent( const Event &event );
        virtual int dispatcher();

private:
	int initEvent()
	{
		int epollFd = epoll_create( this->epollCreateSize );
		if( epollFd <= 0 ){
			std::cerr<<"epoll create error ..."<<std::endl;
			return epollFd;
		}
		this->epollFd = epollFd;
		return true;
	}
	int epollCreateSize;
	int epollFd;
	std::map<int, Event> events;
};


#endif 


















