#ifndef __MULTTIPLE_MESSAGES_SYNCHRONIZE_H_
#define __MULTTIPLE_MESSAGES_SYNCHRONIZE_H_

#include <map>
#include "timer.h"
#include "subscriber.h"
#include "pcs.h"
#include "transport_tcp.h"
#include <queue>

namespace pcs{

using namespace std::placeholders;

int count = 0;

template<typename... A> class MultiSync {};

template<> class MultiSync<> {};

template<typename Head, typename... Tail>
class MultiSync<Head, Tail...> : private MultiSync<Tail...>
{

	typedef MultiSync<Tail...> inherited;
protected:
	std::map<int, std::queue<Head>> queueMap;
	std::queue<Head> queue;

public:
	MultiSync();
	~MultiSync(){}

	void *recvCallback( int fd, void *arg );
	void *timerCallback( int fd, void *arg );

/*	void setTopicName()
	{
	
	}

	template<typename First, typename... Others>	
	void setTopicName( First first, Others... other )
	{
		memset( topicName_[nameCount], 0, TOPICNAMELEN );
		if( first.length() > TOPICNAMELEN ){
        	        std::cout<<"Topic Name is too long, it must be less than 30 bytes ..."<<std::endl;
                	exit(-1);
        	}
        	else{
                	first.copy( topicName_[nameCount], first.length(), 0 );
        	}
		nameCount ++;
		setTopicName( other... );
	}*/

	void spin();
 
	void subscribe( PCS &pcs );
       
	template<typename First, typename... Others>
        void subscribe( First first, Others... other, PCS &pcs )
	{
		memset( topicName_[nameCount], 0, TOPICNAMELEN );
                if( first.length() > TOPICNAMELEN ){
                        std::cout<<"Topic Name is too long, it must be less than 30 bytes ..."<<std::endl;
                        exit(-1);
                }
                else{
                        first.copy( topicName_[nameCount], first.length(), 0 );
                }
                nameCount ++;
		subscribe( other..., pcs );
	}


private:
	void subscribeMultiTopics();
	void connect2Server( int index );
	void printTopicTable();
	void sendHeartBeats( int clientFd );	

	Transport *transport;

        EpollEvent eventBase;

	char sendBuff[4];
        char recvBuff[1024];
	
	int *client_fd;
	TopicInfo *TI;

	Timer timer;
	char topicName_[10][ TOPICNAMELEN ];

	std::map<int, TopicInfo> subscribersTable;
        bool *isConnected;

	int nameCount;
};

template<typename Head, typename... Tail>
MultiSync<Head, Tail...>::MultiSync(): nameCount(0)
{
	transport = new TransportTCP;
	queueMap.insert( std::make_pair( count, queue) );
	count ++;
}

template<typename Head, typename... Tail>
void* MultiSync<Head, Tail...>::recvCallback( int fd, void *arg )
{
	memset( recvBuff, 0, sizeof( recvBuff ) );
        int ret = transport->read( fd, (unsigned char *)recvBuff, sizeof( recvBuff ) );
        if ( ret > 0 ){
		for( int i = 0; i < count; i ++ ){
			if( fd == client_fd[i] ){
				
			}
		}
	}
}

template<typename Head, typename... Tail>
void* MultiSync<Head, Tail...>::timerCallback( int fd, void *arg )
{
	uint64_t uread;
        int ret = read( fd, &uread, sizeof( uint64_t ) );
        if( ret == 8 ){
                std::cout<<"timer ..."<<fd<<std::endl;
        
		for( int i = 0; i < count; i ++ ){
			if ( !isConnected[i] ){
				connect2Server( i );
			}
			else {
				sendHeartBeats( client_fd[i] );
			}
		}
	}

}

template<typename Head, typename... Tail>
void MultiSync<Head, Tail...>::subscribeMultiTopics()
{
	client_fd = new int[count];
	TI = new TopicInfo[count];
	isConnected = new bool[count];	

	for( int i = 0; i < count; i ++ ){
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

                FUNC recvCb = std::bind( &MultiSync::recvCallback, this, _1, _2 );

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

        FUNC timerCb = std::bind( &MultiSync::timerCallback, this, _1, _2 );

        timerEvent.callback = timerCb;

        eventBase.addEvent( timerEvent );

	for( int i = 0; i < count; i ++ ){
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


template<typename Head, typename... Tail>
void MultiSync<Head, Tail...>::connect2Server( int index )
{
	for( auto it = topicTable.begin(); it != topicTable.end(); it ++ ){
                auto item = subscribersTable.find( index );
                if( !strcmp( item->second.topicName, it->topicName ) && it-> topicType == publisher ){
                        if( transport->connectSocket( client_fd[index], it->port, it->ipAddr ) ){
                                isConnected[index] = true;
                        }
                }
        }
}

template<typename Head, typename... Tail>
void MultiSync<Head, Tail...>::printTopicTable()
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


template<typename Head, typename... Tail>
void MultiSync<Head, Tail...>::spin()
{
	while(1){
                eventBase.dispatcher();
        }

}

template<typename Head, typename... Tail>
void MultiSync<Head, Tail...>::subscribe( PCS &pcs )
{
	while(true){
		if( getUdpType(pcs) == udpServer || getUdpType(pcs) == udpClient ){
                        sleep(1);
                        subscribeMultiTopics();
                        for( int i = 0; i < count; i ++ ){
                                TI[i].udpPort = getNodeDiscoveryUdpPort( pcs );

                                topicTable.push_back( TI[i] );
                                sendTopicInfo( pcs, TI[i] );

                        }
                        break;
                }
	}
}

template<typename Head, typename... Tail>
void MultiSync<Head, Tail...>::sendHeartBeats( int clientFd )
{

}


}
#endif	
