#include "publisher.h"
#include <string.h>
#include <unistd.h>
#include <algorithm>

namespace pcs{

using namespace std::placeholders;

Publisher::Publisher(): 
			server_fd(0),
			isRunning(false),
			isLinked(false)
{
	transport = new TransportTCP; 

	//tcpThread_.join();
//	tcpThread_.detach();
}

Publisher::Publisher( const Publisher &obj ): eventBase( obj.eventBase ),
					transport( obj.transport ),
					server_fd( obj.server_fd ),
					clientLinkTable( obj.clientLinkTable ),
					isRunning( obj.isRunning )
{
	//tcpThread_ = obj.tcpThread_;
	memcpy( &acceptEvent, &obj.acceptEvent, sizeof( acceptEvent ) );
	memcpy( &recvEvent, &obj.recvEvent, sizeof( recvEvent ) );
	memcpy( recvBuff, obj.recvBuff, sizeof( recvBuff ) );
	memcpy( sendBuff, obj.sendBuff, sizeof( sendBuff ) );
	memcpy( topicName_, obj.topicName_, sizeof( topicName_ ) );
}

void *Publisher::recvCallback( int fd, void *arg )
{
	memset( recvBuff, 0, sizeof( recvBuff ) );
	int ret = transport->read( fd, recvBuff, sizeof( recvBuff ) );
	
	if( ret > 0 ){
		if( recvBuff[0] == 'c' && recvBuff[1] == 'c'  ){
			std::cout<<"received heartbeats ... "<<std::endl;
			//sendHeartBeat();		
		}
	}
	else{
	//	if( isLinked ){
			std::cerr<<"the client is off line ... "<<ret<<std::endl;
			transport->closeSocket( fd );		
	
			auto it = find_if( clientLinkTable.begin(), clientLinkTable.end(), [fd](ClientInfo cI){ return cI.fd == fd; } );
			if( it != clientLinkTable.end() ){
				clientLinkTable.erase( it );
			}
			
			printLinkTable();		
	//		isLinked = false;
	//	}
	}
}

void *Publisher::acceptCallback( int fd, void *arg )
{
	int clientFd = transport->Accept();
	
	struct sockaddr_in clientAddr = transport->getClientInfo();
	
	ClientInfo cI;
	cI.fd = clientFd;
	strcpy( cI.ipAddr, inet_ntoa( clientAddr.sin_addr ) );
	cI.port = ntohs( clientAddr.sin_port );
	cI.type = 4;
	cI.linkState = true;

	/*  */
	isLinked = true;	

	clientLinkTable.push_back( cI );
	printLinkTable();

	recvEvent.fd = clientFd;
	recvEvent.event = EPOLLIN;
	recvEvent.arg = NULL;

	FUNC recvCb = std::bind( &Publisher::recvCallback, this, _1, _2 );

        recvEvent.callback = recvCb;
	
	eventBase.addEvent( recvEvent );
}


void Publisher::createAPublisher( std::vector<TopicInfo> &topic )
{
        /* initialize a tcp server for a publisher */
        if ( !transport->initSocketServer() ){
                std::cerr<<"init the server failed ..."<<std::endl;
                exit(-1);
        }
        else std::cerr<<"init the tcp server ..."<<std::endl;

        // get the port of the tcp server
        server_fd = transport->getServerFd();

        // initailize the epoll event of the acception of the tcp server
        acceptEvent.fd = server_fd;
        acceptEvent.event = EPOLLIN | EPOLLERR;
        acceptEvent.arg = NULL;

        FUNC acceptCb = std::bind( &Publisher::acceptCallback, this, _1, _2 );

        acceptEvent.callback = acceptCb;

        // add the event into the epoll
        eventBase.addEvent( acceptEvent );

        /* update the topic table */
        //TopicInfo TI;
	memset( &TI, 0, sizeof( TI ) );

        struct sockaddr_in serverInfo;
        serverInfo = transport->getLocalIp(  server_fd );

        std::cout<<"ip ============ "<<inet_ntoa( serverInfo.sin_addr )<<std::endl;
        std::cout<<"port ========== "<<ntohs( serverInfo.sin_port )<<std::endl;

        TI.port = ntohs( serverInfo.sin_port );
        strcpy( TI.ipAddr, inet_ntoa( serverInfo.sin_addr ) );
        strcpy( TI.topicName, topicName_ );
        TI.topicType = publisher;

	std::cout<<"TI.port      = "<<TI.port<<std::endl;
	std::cout<<"TI.ipAddr    = "<<TI.ipAddr<<std::endl;
	std::cout<<"TI.topicName = "<<TI.topicName<<std::endl;

	//topic.push_back( TI );

        /* start the epoll */
        startAccept();
}


void Publisher::startAccept()
{
	isRunning = true;
	
	while( isRunning ) {
		eventBase.dispatcher();
	}
}
	
void Publisher::stopAccept()
{
	if ( isRunning ) {
		isRunning = false;
	}
}

void Publisher::printLinkTable()
{
	int i = 0;
	std::cout<<"---------------------------------------"<<std::endl;
	for( auto it = clientLinkTable.begin(); it != clientLinkTable.end(); it++, i ++ ){
		std::cout<<"fd["<<i<<"] = "<<it->fd<<std::endl;
		std::cout<<"ipAddr["<<i<<"] = "<<it->ipAddr<<std::endl;
		std::cout<<"port["<<i<<"] = "<<it->port<<std::endl;
		std::cout<<"type["<<i<<"] = "<<it->type <<std::endl;
		std::cout<<"linkState["<<i<<"] = "<<it->linkState<<std::endl;
	}
	std::cout<<"---------------------------------------"<<std::endl;
}

void Publisher::setTopicName( std::string topicName )
{
	if( topicName.length() > TOPICNAMELEN ){
		std::cout<<"Topic Name is too long, it must be less than 30 bytes ..."<<std::endl;
		exit(-1);
	}
	else{
		topicName.copy( topicName_, TOPICNAMELEN, 0 );
	}
}


void Publisher::sendHeartBeat()
{
        HeartBeats heart;
        heart.heart1 = 'c';
        heart.heart2 = 'c';
        heart.nodeNum = 1;

        memset( sendBuff, 0, sizeof( sendBuff ) );

        int ret = transport->write( server_fd, sendBuff, sizeof( heart ) );
        if( ret > 0 ){
                std::cout<<"send heartbeat to the server ..."<<ret<<std::endl;
        }

}

TopicInfo Publisher::getTopicInfo()
{
	return  TI;
}


}
