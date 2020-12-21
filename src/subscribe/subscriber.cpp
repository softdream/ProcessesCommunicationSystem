#include "subscriber.h"
#include <string.h>
#include <algorithm>
#include <unistd.h>

namespace pcs{

using namespace std::placeholders;


Subscriber::Subscriber(): //tcpThread_( std::bind( &Subscriber::createASubscriber, this ) ),
			isRunning(false),
			client_fd(0),
			timer_fd(0),
			serverPort_(0)
{

	transport = new TransportTCP;
	//tcpThread_.join();
}

Subscriber::Subscriber( const Subscriber &obj ): serverPort_( obj.serverPort_ ),
						 timer( obj.timer ),
						 //eventBase( obj.eventBase ),
						 transport( obj.transport ),
						 client_fd( obj.client_fd ),
						 timer_fd( obj.timer_fd ),
						 TI( obj.TI )
{	
	memcpy( serverAddr_, obj.serverAddr_, INET_ADDRSTRLEN );
	memcpy( &recvEvent, &obj.recvEvent, sizeof( recvEvent ) );
	memcpy( &timerEvent, &obj.timerEvent, sizeof( timerEvent ) );
	
	memcpy( topicName_, obj.topicName_, sizeof( TOPICNAMELEN ) );
}

void* Subscriber::recvCallback( int fd, void *arg )
{
	memset( recvBuff, 0, sizeof( recvBuff ) );
	uint8_t ret = transport->read( fd, recvBuff, sizeof( recvBuff ) );
	
	if( ret > 0 ){
		if( recvBuff[0] == 'c' && recvBuff[1] == 'c' ){
			std::cout<<"received a heartbeat from the server: "<<ret<<std::endl;
		}
		/* received the message from the publisher */
		if( recvBuff[0] == 'g' && recvBuff[1] == 'g' ){
			//subscribeMessage( message, recvBuff );		
		}
	}
	else {
		std::cerr<<"the server is out of the net ..."<<std::endl;;
		
	}
}

void* Subscriber::timerCallback( int fd, void *arg )
{
	uint64_t uread;
	int ret = read( fd, &uread, sizeof( uint64_t ) );
	if( ret == 8 ){
		std::cout<<"timer ..."<<std::endl;
	//	sendHeartBeats();	
	}
}

void Subscriber::createASubscriber(  )
{
	/* initialize a tcp client for a subscriber */
	if( !transport->initSocketClient() ){
		std::cerr<<"init the client failed ..."<<std::endl;
		exit(-1);
	}
	else std::cerr<<"init the tcp client ..."<<std::endl;

	// get the port of the tcp client
	client_fd = transport->getClientFd();
	if( client_fd <= 0 ){
		std::cerr<<"client_fd <0, error: "<<client_fd<<std::endl;
		exit(-1);
	}

	// connect to the tcp server
	/*if( !transport->connectSocket( serverPort_, serverAddr_ ) ){
		exit(-1);
	}*/
	
	// initialize the epoll event of the tcp client
	recvEvent.fd = client_fd;
	recvEvent.event = EPOLLIN | EPOLLERR;
	recvEvent.arg = NULL;
	
	FUNC recvCb = std::bind( &Subscriber::recvCallback, this, _1, _2 );
	
	recvEvent.callback = recvCb;

	// create a timer
	if( !timer.createTimer() ){
		std::cerr<<"create timer failed ..."<<std::endl;
		exit(-1);
	}
	timer_fd = timer.getTimerFd();
	if( timer_fd <= 0 ){
		std::cerr<<"timer fd < 0, error: "<<timer_fd<<std::endl;
		exit(-1);
	}

	timer.setTimer( 1, 0, 1, 0 ); // 1s/per

	timerEvent.fd = timer_fd;
	timerEvent.event = EPOLLIN;
	timerEvent.arg = NULL;
	
	FUNC timerCb = std::bind( &Subscriber::timerCallback, this, _1, _2 );

	timerEvent.callback = timerCb;	

	// add the events to the epoll
	eventsBase.addEvent( recvEvent );
	eventsBase.addEvent( timerEvent );	

	/* add the topic info into the topic Table */
	//TopicInfo TI;
	memset( &TI, 0, sizeof( TI ) );

	struct sockaddr_in clientInfo = transport->getLocalIp( client_fd );
	std::cout<<"ip ========== "<<inet_ntoa(clientInfo.sin_addr)<<std::endl;
	std::cout<<"port ======== "<<ntohs( clientInfo.sin_port )<<std::endl;
	
	TI.port = ntohs( clientInfo.sin_port );
	strcpy( TI.ipAddr, inet_ntoa( clientInfo.sin_addr ) );
	strcpy( TI.topicName, topicName_ );
	TI.topicType = subscriber;

	//topic.push_back( TI );
	
	// start receive
	//startRecv();
}

bool Subscriber::connect2Server()
{
        // connect to the tcp server
        if( !transport->connectSocket( serverPort_, serverAddr_ ) ){
               	std::cerr<<"failed to connect to the TCP server ..."<<std::endl;
		//exit(-1);
		return false;
        }
	else std::cerr<<" ----------------- Connected to the TCP server ------------"<<std::endl;
	return true;
}

void Subscriber::startRecv()
{
	isRunning = true;
	
	while( isRunning ){
		eventsBase.dispatcher();
	}	
}

void Subscriber::stopRecv()
{
	if( isRunning ){
		isRunning = false;
	}
}

void Subscriber::sendHeartBeats()
{
	HeartBeats heart;
	heart.heart1 = 'c';
	heart.heart2 = 'c';
	heart.nodeNum = 1;
	
	memset( sendBuff, 0, sizeof( sendBuff ) );
	
	int ret = transport->write( client_fd, sendBuff, sizeof( heart ) );
	if( ret > 0 ){
		std::cout<<"send heartbeat to the server ..."<<ret<<std::endl;
	}
}

void Subscriber::setServerInfo( int serverPort, char *serverAddr )
{
	serverPort_ = serverPort;	
	serverAddr_ = serverAddr;
}


void Subscriber::setTopicName( std::string topicName )
{
        if( topicName.length() > TOPICNAMELEN ){
                std::cout<<"Topic Name is too long, it must be less than 30 bytes ..."<<std::endl;
                exit(-1);
        }
        else{
                topicName.copy( topicName_, TOPICNAMELEN, 0 );
        }

}

TopicInfo& Subscriber::getTopicInfo()
{
	return TI;
}

}
