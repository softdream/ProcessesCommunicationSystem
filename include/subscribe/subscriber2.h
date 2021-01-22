#ifndef __SUBSCRIBER2_H_
#define __SUBSCRIBER2_H_

#include "pcs.h"

namespace pcs{

using namespace std::placeholders;

template<typename T>
class Subscribe{
public:
	Subscribe();
	~Subscribe(){
		delete[] recvBuff;
	}
	
	Subscribe( const Subscribe &obj ) = delete ;
        Subscribe& operator=( const Subscribe &other ) = delete;

	Subscribe( const Subscribe &&obj ) noexcept  ;
	Subscribe& operator=( const Subscribe &&other )
	{
		if( this == &other )
			return *this;
	
		client_fd = other.client_fd;
		transport = other.transport;
		timer = other.timer;
		recvEvent = other.recvEvent;
		timerEvent = other.timerEvent;
		topicName_ = other.topicName_;
		TI = other.TI;
		isConnected = other.isConnected;
		sendBuff = other.sendBuff;
		messageLength = other.messageLength;
		recvBuff = other.recvBuff;
		callback = other.callback;
	
		return *this;
	}	

	void *recvCallback( int fd, void *arg );
	void *timerCallback( int fd, void *arg );
	
	using CbFunction = std::function<void( T& )>;

	void subscribe( std::string topicName, CbFunction cb, PCS &pcs );	

private:
	void createASubscriber( );
	void setTopicName( std::string topicName );
	void connect2Server();
	void sendHeartBeats( int client_fd );

	int client_fd;
	Transport *transport;
	Timer timer;	

	Event recvEvent;
	Event timerEvent;

	char topicName_[ TOPICNAMELEN ];
	TopicInfo TI;
	
	bool isConnected;

	unsigned char sendBuff[ 4 ];
	int messageLength;
	char *recvBuff;

	CbFunction callback;
};

/*template<typename T>
Subscribe<T>::Subscribe( const Subscribe &obj ):client_fd( obj.client_fd ),
                                                transport( obj.transport ),
                                                timer( obj.timer ),
                                                recvEvent( obj.recvEvent ),
                                                timerEvent( obj.timerEvent ),
                                                TI( obj.TI )

{
        memcpy( topicName_, obj.topicName_, sizeof( topicName_ ) );
        memcpy( recvBuff, obj.recvBuff, sizeof( recvBuff ) );
}*/

template<typename T>
Subscribe<T>::Subscribe( const Subscribe &&obj ) noexcept
						: client_fd( obj.client_fd ),
                                                transport( obj.transport ),
                                                timer( obj.timer ),
                                                recvEvent( obj.recvEvent ),
                                                timerEvent( obj.timerEvent ),
						topicName_( obj.topicName_ ),
                                                TI( obj.TI ),
						isConnected( obj.isConnected ),
						sendBuff( obj.sendBuff ),
						messageLength( obj.messageLength ),
						recvBuff( obj.recvBuff ),
						callback( obj.callback )
{

}


template<typename T>
Subscribe<T>::Subscribe() : client_fd( 0 ),
			    isConnected( false ),
			    messageLength( 0 )
{
	transport = new TransportTCP;
}

template<typename T>
void* Subscribe<T>::recvCallback( int fd, void *arg )
{
	memset( recvBuff, 0, sizeof( recvBuff ) );
        int ret = transport->read( fd, (unsigned char *)recvBuff, sizeof( recvBuff ) );
        if ( ret > 0 ){
		T data;
		memcpy( &data, recvBuff, sizeof( data ) );
		callback( data );
	}
}

template<typename T>
void* Subscribe<T>::timerCallback( int fd, void *arg )
{
	uint64_t uread; 
        int ret = read( fd, &uread, sizeof( uint64_t ) );
	if( ret == 8 ){
		std::cout<<"timer ..."<<std::endl;
		if( !isConnected ){
			connect2Server(  );
		}
		else {
			sendHeartBeats( client_fd );
		}
	}
}

template<typename T>
void Subscribe<T>::createASubscriber()
{
	/*--------------------- Initialize a tcp client  ----------------------*/
	if( !transport->initSocketClient() ){
		std::cerr<<"init the client failed ..."<<std::endl;
		exit(-1);
	}
	else std::cerr<<"---------- Init the TCP Client ------------"<<std::endl;

	client_fd = transport->getClientFd();
	if( client_fd <= 0 ){
		std::cerr<<"client_fd < 0, error: "<<client_fd<<std::endl;
		exit(-1);
	}

	struct sockaddr_in localAddr;
	localAddr.sin_family = AF_INET;
	localAddr.sin_port = htons( 0 );
	localAddr.sin_addr.s_addr = htonl( INADDR_ANY );
	socklen_t len = sizeof( localAddr );

	if( bind( client_fd, (struct sockaddr *)&localAddr, len ) == -1 ){
		std::cerr<<"bind failed ..."<<std::endl;
		exit(-1);
	}	
	
	recvEvent.fd = client_fd;
	recvEvent.event = EPOLLIN | EPOLLERR;
	recvEvent.arg = NULL;
	
	FUNC recvCb = std::bind( &Subscribe::recvCallback, this, _1, _2 );
	recvEvent.callback = recvCb;
	
	/*----------------------- Create A Timer -----------------------------*/
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

	timerEvent.fd = timer_fd;
        timerEvent.event = EPOLLIN;
        timerEvent.arg = NULL;

        FUNC timerCb = std::bind( &Subscribe::timerCallback, this, _1, _2 );

        timerEvent.callback = timerCb;

	memset( &TI, 0, sizeof( TI ) );
	struct sockaddr_in clientInfo = transport->getLocalIp( client_fd );
        std::cout<<"ip ========== "<<inet_ntoa(clientInfo.sin_addr)<<std::endl;
        std::cout<<"port ======== "<<ntohs( clientInfo.sin_port )<<std::endl;

	TI.port = ntohs( clientInfo.sin_port );
        strcpy( TI.ipAddr, inet_ntoa( clientInfo.sin_addr ) );
        strcpy( TI.topicName, topicName_ );
        TI.topicType = subscriber;

}

template<typename T>
void Subscribe<T>::subscribe( std::string topicName, CbFunction cb, PCS &pcs )
{
	std::cout<<"Create A Subscriber ..."<<std::endl;
	memset( topicName_, 0, sizeof( topicName_ ) );
	setTopicName( topicName );
	
	// allocate the memory for receiving the message
	int messageLength = sizeof( T );
	std::cout <<"message size = "<<messageLength<<std::endl;
	recvBuff = new char[ messageLength + 1 ];

	callback = cb;

	while( true ){
		if( getUdpType(pcs) == udpServer || getUdpType(pcs) == udpClient ){
			sleep(1);
			createASubscriber();
			pcs.addEvents( recvEvent );
			pcs.addEvents( timerEvent );			

			TI.udpPort = getNodeDiscoveryUdpPort( pcs );
					
			topicTable.push_back( TI );
			sendTopicInfo( pcs, TI );
			break;
		}
	}
}

template<typename T>
void Subscribe<T>::setTopicName( std::string topicName )
{
	if( topicName.length() > TOPICNAMELEN ){
                std::cout<<"Topic Name is too long, it must be less than 30 bytes ..."<<std::endl;
                exit(-1);
        }
        else{
                topicName.copy( topicName_, topicName.length(), 0 );
        }
}

template<typename T>
void Subscribe<T>::connect2Server(  )
{
	for( auto it = topicTable.begin(); it != topicTable.end(); it ++ ){
		if( !strcmp( topicName_, it->topicName ) && it->topicType = publisher ){
			if( transport->connectSocket( client_fd, it->port, it->ipAddr ) ){
				isConnected = true;
			}
		}
	}
}

template<typename T>
void Subscribe<T>::sendHeartBeats( int client_fd )
{
	HeartBeats heart;
	heart.heart1 = 'c';
	heart.heart2 = 'c';
	heart.nodeNum = 1;
	
	memset( sendBuff, 0, sizeof( sendBuff ) );
	
	int ret = transport->write( client_fd, sendBuff, sizeof( heart ) );
	if( ret > 0 ){
		
	}
	else {
		std::cout<<"tcp server is offline ..."<<ret<<std::endl;
		isConnected = false;
	}
}

}

#endif
