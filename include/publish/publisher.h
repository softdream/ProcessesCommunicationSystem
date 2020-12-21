#ifndef __PUBLISHER_H_
#define __PUBLISHER_H_

#include "transport.h"
#include "transport_tcp.h"
#include "messageProtocal.h"

#include "EpollEvent.h"
#include "IEvent.h"

#include <iostream>
#include <vector>

#include <thread>

namespace pcs{

class Publisher
{
public:
	/* constructor function */
	Publisher();
	
	/* copy constructor function */
	Publisher( const Publisher &obj );
	
	/* deconstructor function */
	~Publisher(){
		transport->closeSocket( server_fd );
	}
	
	/* create a tcp server of the publisher */
	void createAPublisher( std::vector<TopicInfo> &topic );

		
	/* accept action of the tcp server */
	void startAccept();
	void stopAccept();
	
	/* when a client is linked, call this */
	void *acceptCallback( int fd, void *arg );
	/* when server received a message, call this */
	void *recvCallback( int fd, void *arg );	

	/* update the topic name */
	void setTopicName( std::string topicName );

	/* publish a frame of message data */
	template<typename T>
	void publishMessage( T &message, unsigned char *sendBuff );

	TopicInfo getTopicInfo();

	/* print the link table */
	void printLinkTable();

private:
	
	/* send the heartbeat to the client */
	void sendHeartBeat();	

	/* build a thread of the tcp server */
	std::thread tcpThread_;

	/* an instance of the epoll class */
	EpollEvent eventBase;
	/* events */
	Event acceptEvent;
	Event recvEvent;	

	/* an instance of the abstract class: Transport */
	Transport *transport;

	/* a table of the topic */
	//std::vector<TopicInfo> topicTable;
	TopicInfo TI;

	/* descriptor of the tcp server */
	int server_fd;

	/* tcp server mantain a link table of the clients */
	std::vector<ClientInfo> clientLinkTable;
	
	bool isRunning;
	bool isLinked;
	
	/* receive buffer */
	unsigned char recvBuff[4];	
	unsigned char sendBuff[4];
	
	/* topic name */
	char topicName_[ TOPICNAMELEN ];
};

template<typename T>
void Publisher::publishMessage( T &message, unsigned char *sendBuff )
{
        memset( sendBuff, 0 , sizeof( sendBuff ) );
        memcpy( sendBuff, &message, sizeof( message ) );

        if( !clientLinkTable.empty() ){
                for( auto it = clientLinkTable.begin(); it != clientLinkTable.end(); it ++ ){
                        int ret = send( it->fd, sendBuff, sizeof( message ), 0 );
                        if( ret > 0 ){
                                std::cout<<"publish the message: "<<ret<<std::endl;
                        }
                        else std::cout<<"cannot publish the message: "<<ret<<std::endl;
                }

        }
}


}

#endif
