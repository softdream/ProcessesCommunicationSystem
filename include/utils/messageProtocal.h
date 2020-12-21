#ifndef __MESSAGE_PROTOCAL_H_
#define __MESSAGE_PROTOCAL_H_

#define TOPICNAMELEN 30

namespace pcs{

#include <netinet/in.h>
#include <iostream>

/* Header of the finder */
typedef struct{
	char head1;
	char head2;
	char head3;
	char head4;
}Header;


/* HeartBeat mechanic */
typedef struct{
	char heart1;
	char heart2;
	char nodeNum;
}HeartBeats;


/* node information */
struct nodeInfo{
	nodeInfo(){}
	~nodeInfo(){}

	bool operator()( nodeInfo &p )
	{
		std::string s1 = ipAddr;
		std::string s2 = p.ipAddr;
		if( !s1.compare( s2 ) && port == p.port ) return true;
		else return false;
	}
	
	
	char ipAddr[ INET_ADDRSTRLEN ];
	int port;
	int type;
	int timeOfClient;
};

/* Node */
struct Node{
	Node(){}
	
	Node( const Node &obj ): head1( obj.head1 ),
					 head2( obj.head2 ),
					 head3( obj.head3 ),
					 port( obj.port ),
					 type( obj.type )
	{
		memcpy( ipAddr, obj.ipAddr, INET_ADDRSTRLEN );
	}
	
	Node& operator=( const Node &other )
	{
		if( this == &other )
			return *this;
		head1 = other.head1;
		head2 = other.head2;
		head3 = other.head3;
		
		port = other.port;
		type = other.type;
		memcpy( ipAddr, other.ipAddr, INET_ADDRSTRLEN );

		return *this;
	}

	~Node(){}

	char head1;
	char head2;
	char head3;
	char ipAddr[ INET_ADDRSTRLEN ];
	int port;
	int type;
};
typedef struct Node Node;


/* Notify */
typedef struct Notify{
	char head1;
	char head2;
}Notify;

typedef enum{
	publisher = 1,
	subscriber
}TopicType;

typedef enum{
	udpServer = 1,
	udpClient
}UdpType;

/* Sub-Pub Node Information */
struct TopicInfo{
	TopicInfo(){}

	TopicInfo( const TopicInfo &obj ): port( obj.port ),
					   topicType( obj.topicType ),
					   head1( obj.head1 ),
					   head2( obj.head2 ),
					   udpPort( obj.udpPort )
	{
		memcpy( topicName, obj.topicName, TOPICNAMELEN );
		memcpy( ipAddr, obj.ipAddr, INET_ADDRSTRLEN );
	}	

	TopicInfo& operator=( const TopicInfo &other )
	{
		if( this == &other )	
			return *this;
		head1 = other.head1;
		head2 = other.head2;
		port = other.port;
		udpPort = other.udpPort;
		topicType = other.topicType;
		memcpy( topicName, other.topicName, TOPICNAMELEN );
		memcpy( ipAddr, other.ipAddr, INET_ADDRSTRLEN );
		
		return *this;
	}
	
	~TopicInfo(){}	

	char head1;
	char head2;
	char topicName[ TOPICNAMELEN ];	
	int port;
	char ipAddr[ INET_ADDRSTRLEN ];
	TopicType topicType;
	int udpPort;
};
typedef struct TopicInfo TopicInfo;

/* clients' information linked to the tcp server */
struct ClientInfo{
	int fd;
	char ipAddr[ INET_ADDRSTRLEN ];
	int port;
	int type;
	bool linkState;
};
typedef struct ClientInfo ClientInfo;

}


#endif
