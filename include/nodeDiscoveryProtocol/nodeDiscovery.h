#ifndef __NODE_DISCOVERY_H_
#define __NODE_DISCOVERY_H_

#include "transport_udp.h"
#include "EpollEvent.h"
#include "timer.h"
#include "messageProtocal.h"

#include <vector>

namespace pcs{
	
#define TOPICTYPE( x ) ( ( x ) == 1 ? ( "PUBLISHER" ) : ( "SUBSCRIBER" ) )

extern std::vector<TopicInfo> topicTable;

class NodeDiscovery
{
public:
	NodeDiscovery();
	~NodeDiscovery()
	{
		
	}

	void startDiscovery();

	void stopDiscovery();

	void *timerCallback( int fd, void *arg );
	void *finderCallback( int fd, void *arg );
	void *serverCallback( int fd, void *arg );
	void *clientCallback( int fd, void *arg );
	void *timer2Callback( int fd, void *arg );	
	void *timer3Callback( int fd, void *arg );


	int getPort()
	{
		return port;
	}

	UdpType getUdpType()
	{
		return udpType;
	}

	/* send a topic info to the udp server */
	void sendTopicInfo( TopicInfo &topicInfo );

	void setTopicUpdateStatus( bool status );
	bool getTopicUpdateStatus(  );

private:
	void initFinderClient();
	void updateLinkTable();
	void sendNotifyMessage();
	
	void topicUpdateNotify();
	void updateTopicTable();

	void printTopicTable();

	int client_fd;
	int timer_fd;
	int server_fd;

	Transport *finder;
	
	EpollEvent eventBase;
	Event finderEvent;
	Event timerEvent;
	Event serverEvent;
	Event clientEvent;
	
	Timer timer;
	int timerCount;
	int timeOfServer;
	int timeOfClient;

	unsigned char sendBuff[128];// send buffer
	unsigned char recvBuff[128]; //recv buffer
	//pcs::Header head;

	bool isRunning;
	bool isFirstNode;
	bool received;
	
	int findCount;

	std::vector<nodeInfo> linkTable;


	bool isServerOffLine;

	int updateCount;
	int topicUpdateCount;
	
	bool needHeart;

	UdpType udpType;

	/* the udp port of the client or server */
	int port;

protected:
	bool topicUpdateStatus;

};


}

#endif
