#include "pcs.h"
#include <unistd.h>

namespace pcs{

using namespace std::placeholders;

PCS::PCS() : nodeDiscoveryThread_( std::bind( &PCS::initANodeDiscovery, this ) ), recvSize(0)	
{
	nodeDiscoveryThread_.detach();	
	transport = new TransportTCP;
}

void PCS::initANodeDiscovery()
{
	nodeDiscovery.startDiscovery();
}

Publisher PCS::advertise( std::string topicName  )
{
	std::cout<<"create a advertise ..."<<std::endl;
	Publisher pub;
	
	pub.setTopicName( topicName );
		
	//publihsersTable.insert( std::make_pair( topicName, pub ) );
	
	while( true  ){
		if( nodeDiscovery.getUdpType() == udpServer || nodeDiscovery.getUdpType() == udpClient ){
		//	auto it = publihsersTable.find( topicName );
		//	if( it != publihsersTable.end() ){
				std::thread publishThread_( &Publisher::createAPublisher, &pub, std::ref( topicTable ) ) ;

				sleep(1);
		
				TopicInfo TI = pub.getTopicInfo();
			       	TI.udpPort = nodeDiscovery.getPort();
			
				topicTable.push_back( TI );
				// send the topic information to the udp server
	       			nodeDiscovery.sendTopicInfo( TI );
			
				publishThread_.detach();	
		//	}	
			break;
		}
	}				
	
	return pub;
}

void* PCS::recvCallback( int fd, void *arg )
{
	memset( recvBuff, 0, sizeof( recvBuff ) );
	int ret = transport->read( fd, recvBuff, sizeof( recvBuff ) );
	
	if( ret > 0 ){
		std::cout<<"received                        :  "<<recvBuff<<std::endl;
		auto it = cbMap.find( fd );
		
		it->second( (void*)recvBuff );
	}
}


void* PCS::timerCallback( int fd, void *arg )
{
        uint64_t uread;
        int ret = read( fd, &uread, sizeof( uint64_t ) );
        if( ret == 8 ){
                std::cout<<"timer ..."<<fd<<std::endl;
		auto it = isConnected.find( fd );
        	if( it->second )      sendHeartBeats(fd);       
        	else {
			std::cout<<"ready to connect to the tcp server ..."<<std::endl;
			connect2Server(fd);
		}
	}

}

void PCS::createASubscriber()
{
	client_fd = 0;
	
	/***************** initialize a tcp client for a subscriber *************/
	if( !transport->initSocketClient() ){
                std::cerr<<"init the client failed ..."<<std::endl;
                exit(-1);
        }
        else std::cerr<<"------------Init The Tcp Client ------------"<<std::endl;

        // get the port of the tcp client
  //      int client_fd = transport->getClientFd();
    	client_fd = transport->getClientFd();
	if( client_fd <= 0 ){
                std::cerr<<"client_fd <0, error: "<<client_fd<<std::endl;
                exit(-1);
        }
	
	// bind a random tcp port
	struct sockaddr_in localAddr;
	localAddr.sin_family = AF_INET;
	localAddr.sin_port = htons( 0 );
	localAddr.sin_addr.s_addr = htonl( INADDR_ANY );
	socklen_t len = sizeof( localAddr );
	
	if( bind( client_fd, (struct sockaddr *)&localAddr, len ) == -1 ){
		std::cerr<<"bind failed ..."<<std::endl;
		exit(-1);
	}	

	Event recvEvent;
        // initialize the epoll event of the tcp client
        recvEvent.fd = client_fd;
        recvEvent.event = EPOLLIN | EPOLLERR;
        recvEvent.arg = NULL;

        FUNC recvCb = std::bind( &PCS::recvCallback, this, _1, _2 );

        recvEvent.callback = recvCb;
	/***********************************************************************/
	/*************************** create a timer ****************************/
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

        FUNC timerCb = std::bind( &PCS::timerCallback, this, _1, _2 );

        timerEvent.callback = timerCb;
	/**********************************************************************/
        eventBase.addEvent( timerEvent );
	eventBase.addEvent( recvEvent );
		
	memset( &TI, 0, sizeof( TI ) );

	struct sockaddr_in clientInfo = transport->getLocalIp( client_fd );
        std::cout<<"ip ========== "<<inet_ntoa(clientInfo.sin_addr)<<std::endl;
        std::cout<<"port ======== "<<ntohs( clientInfo.sin_port )<<std::endl;

        TI.port = ntohs( clientInfo.sin_port );
        strcpy( TI.ipAddr, inet_ntoa( clientInfo.sin_addr ) );
        strcpy( TI.topicName, topicName_ );
        TI.topicType = subscriber;

	subscribersTable.insert( std::make_pair( timer_fd, TI ) );

	isConnected.insert( std::make_pair( timer_fd, false ) );

	fdMap.insert( std::make_pair( timer_fd, client_fd ) );
}

void PCS::setTopicName( std::string topicName )
{
        if( topicName.length() > TOPICNAMELEN ){
                std::cout<<"Topic Name is too long, it must be less than 30 bytes ..."<<std::endl;
                exit(-1);
        }
        else{
                topicName.copy( topicName_, topicName.length(), 0 );
        }

}

void PCS::sendHeartBeats( int fd )
{
	HeartBeats heart;
        heart.heart1 = 'c';
        heart.heart2 = 'c';
        heart.nodeNum = 1;

        memset( sendBuff, 0, sizeof( sendBuff ) );
	
	auto fdItem = fdMap.find(fd);
	int ret = transport->write( fdItem->second, sendBuff, sizeof( heart ) );
	if( ret > 0 )
		std::cout<<"Send heartbeat to the server ..."<<ret<<std::endl;
	else {
		std::cout<<"the tcp server is offline, and disconecet ..."<<ret<<std::endl;
		auto connectItem = isConnected.find(fd);
		connectItem->second = false;
	}
}

void PCS::connect2Server( int fd )
{
	auto connectItem = isConnected.find(fd);
	for( auto it = topicTable.begin(); it != topicTable.end(); it ++ ){
		auto item = subscribersTable.find( fd );
		
		if( !strcmp( item->second.topicName, it->topicName ) && it->topicType == publisher ){
			auto fdItem = fdMap.find(fd);

			if( transport->connectSocket( fdItem->second, it->port, it->ipAddr ) )
				connectItem->second = true;
			else connectItem->second = false;
		}		
	}
}

void PCS::spin()
{
	while(1){
		eventBase.dispatcher();
	}
}

void PCS::printTopicTable()
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


UdpType getUdpType( PCS &p )
{
	return p.nodeDiscovery.getUdpType();
}

 
void sendTopicInfo( PCS &p, TopicInfo &topicInfo )
{
	p.nodeDiscovery.sendTopicInfo( topicInfo );
}

int getNodeDiscoveryUdpPort( PCS &p )
{
	return p.nodeDiscovery.getPort();
}

void PCS::addEvents( Event &event )
{
	eventBase.addEvent( event );
}

void PCS::sleep_for( int millseconds )
{
	std::this_thread::sleep_for( std::chrono::milliseconds( millseconds ) );
}

}
