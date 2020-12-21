#ifndef __SUBSCRIBLE_H_
#define __SUBSCRIBLE_H_

#include "transport.h"
#include "transport_tcp.h"
#include "messageProtocal.h"

#include "EpollEvent.h"
#include "IEvent.h"

#include <iostream>
#include <vector>

#include <thread>

#include "timer.h"

namespace pcs{

static EpollEvent eventsBase;

class Subscriber
{
public:
	Subscriber();
	~Subscriber(){}
	
	Subscriber( const Subscriber &obj );

        /* create a tcp client of the publisher */
        void createASubscriber(  );
	
	/*  */
	void setServerInfo( int serverPort, char *serverAddr );
	
	/* tcp client receive callback function */
	void *recvCallback( int fd, void *arg );
	void *timerCallback( int fd, void *arg );	

	//void subscribeMessage( T &message, unsigned char *recvBuff );

	void stopRecv();

	/* connection function */
	bool connect2Server();

        /* update the topic name */
        void setTopicName( std::string topicName );

	TopicInfo& getTopicInfo(  );

	int getClientFd()
	{
		return client_fd;
	}

private:
	void startRecv();	
	void sendHeartBeats();
	
	int serverPort_;
	char *serverAddr_;
	
	std::thread tcpThread_;
	Timer timer;

	/* epoll event */
	//static EpollEvent eventsBase;
	Event recvEvent;
	Event timerEvent;

	/* an instance of the abstract class: Transport */
	Transport *transport;

	bool isRunning;
	
	/* descriptor of the tcp client */
	int client_fd;

	/* descriptor of the timer */
	int timer_fd;

	/* send buffer */
	unsigned char sendBuff[4];
	unsigned char recvBuff[4];

	/* a table of the topic information in this node */
	//std::vector<TopicInfo> topicTable;
	TopicInfo TI;	

	char topicName_[ TOPICNAMELEN ];

};

//EpollEvent Subscriber::eventsBase = EpollEvent(  );

}

#endif
