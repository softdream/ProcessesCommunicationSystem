#ifndef __MESSAGE_SYNCHRONIZE_H_
#define __MESSAGE_SYNCHRONIZE_H_

#include <map>
#include "timer.h"
#include "subscriber.h"
#include "pcs.h"
#include "transport_tcp.h"
#include <queue>

namespace pcs{

using namespace std::placeholders;

template<typename D1, typename D2>
class MessageSync{
public:
	MessageSync();
	MessageSync( const MessageSync &obj ){}

	~MessageSync()
	{
	}
	
	void subscribe( std::string topicName1 , std::string topicName2, PCS &pcs );

	using CB = std::function<void( D1, D2 )>;

	void registerCallback( CB cb );

	void *recvCallback( int fd, void *arg );
        void *timerCallback( int fd, void *arg );

	void printTopicTable();

	void spin();

private:
	void subscribeTwoTopics();
	void connect2Server( int count );
	
	void sendHeartBeats( int clientFd );

	char sendBuff[4];
	char recvBuff[1024];
	
	void setTopicName( std::string topicName1, std::string topicName2 );

protected:
	Transport *transport;

        EpollEvent eventBase;

        TopicInfo TI[2];

        char topicName_[2][ TOPICNAMELEN ];

        Timer timer;

        int client_fd[2];	

	std::map<int, TopicInfo> subscribersTable;
	bool isConnected[2];

	//std::queue<D1> firstDataQueue;
	std::queue<D2> secondDataQueue;
	
	CB callback;
};

template<typename D1, typename D2>
MessageSync<D1, D2>::MessageSync()
{
	transport = new TransportTCP;
}

template<typename D1, typename D2>
void* MessageSync<D1, D2>::recvCallback( int fd, void *arg )
{
	memset( recvBuff, 0, sizeof( recvBuff ) );
        int ret = transport->read( fd, (unsigned char *)recvBuff, sizeof( recvBuff ) );
	if ( ret > 0 ){
		if( fd == client_fd[0] ){
			D1 d1;
			memcpy( &d1, recvBuff, sizeof(d1) );
			D2 d2;
			if(!secondDataQueue.empty()){
				d2 = secondDataQueue.back();
				while (!secondDataQueue.empty()) secondDataQueue.pop();
				callback( d1, d2 );	
			}
		}
		else if( fd == client_fd[1] ){
			D2 d2;
			memcpy( &d2, recvBuff, sizeof(d2) );
			secondDataQueue.push( d2 );
		}	
	}
}

template<typename D1, typename D2>
void* MessageSync<D1, D2>::timerCallback( int fd, void *arg )
{
	uint64_t uread;
        int ret = read( fd, &uread, sizeof( uint64_t ) );
        if( ret == 8 ){
                std::cout<<"timer ..."<<fd<<std::endl;
		if( !isConnected[0] ){
			connect2Server( 0 );
		}
		else{
			sendHeartBeats( client_fd[0] );
		}
		if( !isConnected[1] ){
			connect2Server( 1 );
		}
		else {
			sendHeartBeats( client_fd[1] );
		}
        }

}

template<typename D1, typename D2>
void MessageSync<D1, D2>::subscribeTwoTopics()
{
	for( int i = 0; i < 2; i ++ ){
		if( !transport->initSocketClient() ){
	                std::cerr<<"init the client failed ..."<<std::endl;
        	        exit(-1);
        	}
        	else std::cerr<<"------------Init The Tcp Client ------------"<<i<<std::endl;
		client_fd[i] = transport->getClientFd();
		if( client_fd[i] <= 0 ){
			std::cerr<<"client_fd <= 0, error: "<<client_fd[i]<<std::endl;
			exit(-1);
		}
		// bind a random tcp port
        	struct sockaddr_in localAddr;
	        localAddr.sin_family = AF_INET;
        	localAddr.sin_port = htons( 0 );
	        localAddr.sin_addr.s_addr = htonl( INADDR_ANY );
        	socklen_t len = sizeof( localAddr );

	        if( bind( client_fd[i], (struct sockaddr *)&localAddr, len ) == -1 ){
        	        std::cerr<<"bind failed ..."<<std::endl;
                	exit(-1);
        	}		
		Event recvEvent;
	        // initialize the epoll event of the tcp client
        	recvEvent.fd = client_fd[i];
	        recvEvent.event = EPOLLIN | EPOLLERR;
        	recvEvent.arg = NULL;

	        FUNC recvCb = std::bind( &MessageSync::recvCallback, this, _1, _2 );

        	recvEvent.callback = recvCb;
		eventBase.addEvent( recvEvent );
	}
	if( !timer.createTimer() ){
                std::cerr<<"create timer failed ..."<<std::endl;
                exit(-1);
        }
        int timer_fd = timer.getTimerFd();
        if( timer_fd <= 0 ){
                std::cerr<<"timer fd < 0, error: "<<timer_fd<<std::endl;
                exit(-1);
        }
	timer.setTimer( 1, 0, 1, 0 ); // 1s/per

	Event timerEvent;
	timerEvent.fd = timer_fd;
        timerEvent.event = EPOLLIN;
        timerEvent.arg = NULL;

        FUNC timerCb = std::bind( &MessageSync::timerCallback, this, _1, _2 );

        timerEvent.callback = timerCb;
	
	eventBase.addEvent( timerEvent );	

	for( int i = 0; i < 2; i ++ ){
		memset( &TI[i], 0, sizeof( TI[i] ) );

	        struct sockaddr_in clientInfo = transport->getLocalIp( client_fd[i] );
        	std::cout<<"ip ========== "<<inet_ntoa(clientInfo.sin_addr)<<std::endl;
	        std::cout<<"port ======== "<<ntohs( clientInfo.sin_port )<<std::endl;

	        TI[i].port = ntohs( clientInfo.sin_port );
        	strcpy( TI[i].ipAddr, inet_ntoa( clientInfo.sin_addr ) );
	        strcpy( TI[i].topicName, topicName_[i] );
        	TI[i].topicType = subscriber;

		subscribersTable.insert( std::make_pair( i, TI[i] ) );
		isConnected[i] = false;
	}
}

template<typename D1, typename D2>
void MessageSync<D1, D2>::subscribe( std::string topicName1 , std::string topicName2, PCS &pcs )
{
	std::cout<<"Create a subscribe ..."<<std::endl;
	memset( topicName_, 0, sizeof( topicName_ ) );
	setTopicName( topicName1, topicName2 );
	while( true ){
		if( getUdpType(pcs) == udpServer || getUdpType(pcs) == udpClient ){
			sleep(1);
			subscribeTwoTopics();
			for( int i = 0; i < 2; i ++ ){
				TI[i].udpPort = getNodeDiscoveryUdpPort( pcs );
				
				topicTable.push_back( TI[i] );
				sendTopicInfo( pcs, TI[i] );
				
			}
			break;
		}
	}
}

template<typename D1, typename D2>
void MessageSync<D1, D2>::connect2Server( int count )
{
        /*auto connectItem = isConnected.find(fd);
        for( auto it = topicTable.begin(); it != topicTable.end(); it ++ ){
                auto item = subscribersTable.find( fd );

                if( !strcmp( item->second.topicName, it->topicName ) && it->topicType == publisher ){
                        auto fdItem = fdMap.find(fd);

                        if( transport->connectSocket( fdItem->second, it->port, it->ipAddr ) )
                                connectItem->second = true;
                        else connectItem->second = false;
                }
        }*/
	for( auto it = topicTable.begin(); it != topicTable.end(); it ++ ){
		auto item = subscribersTable.find( count );
		if( !strcmp( item->second.topicName, it->topicName ) && it-> topicType == publisher ){
			if( transport->connectSocket( client_fd[count], it->port, it->ipAddr ) ){
				isConnected[count] = true;
			}
		}
	}
}

template<typename D1, typename D2>
void MessageSync<D1, D2>::printTopicTable()
{
        int i = 1;
        std::cout<<"--------Topic-Table----------"<<std::endl;
        for( auto it = topicTable.begin(); it != topicTable.end(); it ++, i ++ ){
                std::cout<<" topic name["<<i<<"] = "<<it->topicName<<std::endl;
                std::cout<<" port      ["<<i<<"] = "<<it->port<<std::endl;
                std::cout<<" ip address["<<i<<"] = "<<it->ipAddr<<std::endl;
                std::cout<<" topic type["<<i<<"] = "<<TOPICTYPE( it->topicType )<<std::endl;
                std::cout<<" udp   port["<<i<<"] = "<<it->udpPort<<std::endl;
                std::cout<<std::endl;
        }
        std::cout<<"---------------------------"<<std::endl;
}

template<typename D1, typename D2>
void MessageSync<D1, D2>::setTopicName( std::string topicName1, std::string topicName2 )
{
	if( topicName1.length() > TOPICNAMELEN || topicName2.length() > TOPICNAMELEN ){
                std::cout<<"Topic Name is too long, it must be less than 30 bytes ..."<<std::endl;
                exit(-1);
        }
        else{
                topicName1.copy( topicName_[0], topicName1.length(), 0 );
		topicName2.copy( topicName_[1], topicName2.length(), 0 );
        }	
}

template<typename D1, typename D2>
void MessageSync<D1, D2>::registerCallback( CB cb )
{
	callback = cb;	
}

template<typename D1, typename D2>
void MessageSync<D1, D2>::spin()
{
	while(1){
		eventBase.dispatcher();
	}
}

template<typename D1, typename D2>
void MessageSync<D1, D2>::sendHeartBeats( int clientFd )
{

}

}

#endif













