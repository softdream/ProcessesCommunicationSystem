#ifndef __PCS_H_
#define __PCS_H_

#include "nodeDiscovery.h"
#include "timer.h"
#include "publisher.h"
#include <vector>
#include <map>

#include "subscriber.h"
#include <thread>

namespace pcs{

using CallBack = std::function<void( void* )>;

class PCS
{
public:
	PCS();
	~PCS(){}
	
	Publisher advertise( std::string topicName );
	
	template<typename T>
	void subscribe( std::string topicName, CallBack callback );

	void printTopicTable();	

	void *recvCallback( int fd, void *arg );
	void *timerCallback( int fd, void *arg );

	/* update the topic name */
        void setTopicName( std::string topicName );

	void spin();
	
	template<typename T>
	T& getData();

	void addEvents( Event &event );

	void sleep_for( int millseconds );

	friend UdpType getUdpType( PCS &p );
	friend void sendTopicInfo(PCS &p, TopicInfo &topicInfo);
	friend int getNodeDiscoveryUdpPort( PCS &p );

protected:
	/* process function of the nodeDiscovery thread */
	void initANodeDiscovery();

	void createASubscriber();

	void sendHeartBeats( int fd );	

	void connect2Server( int fd );
	
	/* create a new thread for nodes discovering */
	std::thread nodeDiscoveryThread_;

	/* define a NodeDiscovery instance */
	NodeDiscovery nodeDiscovery;

	Transport *transport;
	
	EpollEvent eventBase;

	TopicInfo TI;

	char topicName_[ TOPICNAMELEN ];

	Timer timer;

	int client_fd;
	
	std::map<int, bool> isConnected;

	std::map<int, TopicInfo> subscribersTable;
	std::map<int, int> fdMap;

	/* container for storaging the pointer of the callback functions */
	std::map<int, CallBack> cbMap;

	int recvSize;

        unsigned char sendBuff[4];
        unsigned char recvBuff[256];

};

template<typename T>
void PCS::subscribe( std::string topicName, CallBack callback )
{	
	recvSize = sizeof(T);
        std::cout<<"Create a subscribe ..."<<std::endl;

	memset( topicName_, 0, sizeof( topicName_ ) );
	setTopicName( topicName );
        while( true ){
                if( nodeDiscovery.getUdpType() == udpServer || nodeDiscovery.getUdpType() == udpClient ) {
			sleep(1);
			createASubscriber();
			TI.udpPort = nodeDiscovery.getPort();
			//std::cerr<<"UDP port =================================== "<<TI.udpPort<<std::endl;			

			topicTable.push_back( TI );
                        // send the topic information to the udp server
                        nodeDiscovery.sendTopicInfo( TI );

			// add the pointer of the callback function to the container
			cbMap.insert( std::make_pair( client_fd, callback ) );			

                        break;
                }
        }
}

template<typename T>
T& PCS::getData()
{
	T parameter;
	memset( &parameter, 0, sizeof( parameter ) );
	memcpy( &parameter, recvBuff, sizeof( parameter ) );
	
	return parameter;
}

template<typename T>
T getData( void *arg )
{
	T parameter;
        memset( &parameter, 0, sizeof( parameter ) );
        memcpy( &parameter, arg, sizeof( parameter ) );
	return parameter;
}

}

#endif
