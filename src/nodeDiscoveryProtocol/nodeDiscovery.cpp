#include "nodeDiscovery.h"
#include <string.h>
#include <algorithm>
#include <unistd.h>


namespace pcs{

using namespace std::placeholders;


std::vector<TopicInfo> topicTable;

NodeDiscovery::NodeDiscovery() : client_fd(0),
				 isRunning(false),
				 isFirstNode(true),
				 findCount(0),
				 received(false),
				 timerCount(0),
				 timeOfServer(0),
				 timeOfClient(0),
				 isServerOffLine(false),
				 updateCount(0),
				 topicUpdateCount(0),
				 needHeart(true),
				 port(0),
				 topicUpdateStatus(false)
{
	finder = new TransportUDP;
}

void *NodeDiscovery::clientCallback( int fd, void *arg )
{
	memset( recvBuff, 0, sizeof( recvBuff ) );
	int ret = finder->read( fd, recvBuff, sizeof( recvBuff ) );
	
	if( ret > 0 ){
		std::cout<<"client received "<<ret<<" bytes data ..."<< std::endl;
		// this is a heartbeat message from the server
		if( recvBuff[0] == 'c' && recvBuff[1] == 'c' ){
			std::cout<<"received a heart from udp server ..."<<std::endl;
			isServerOffLine = false;
			timeOfServer = timerCount;
		}

		if( recvBuff[0] == 'b' && recvBuff[1] == 'b' && recvBuff[2] == 'b' ){
			std::cout<<"recevied a link table ... "<<std::endl;
			Node node;
			memcpy( &node, recvBuff, sizeof( node ) );
			nodeInfo nI;
			strcpy( nI.ipAddr, node.ipAddr );
			nI.port = node.port;
			nI.type = node.type;
			nI.timeOfClient = 0;
			linkTable.push_back( nI );
	
			std::cout<<" ----------------------------------------- "<<std::endl;
			/* print for testing */
			for( auto it = linkTable.begin(); it != linkTable.end(); it ++ ){
				std::cout<<"ip = "<<it->ipAddr<<std::endl;
				std::cout<<"port = "<<it->port<<std::endl;
				std::cout<<"type = "<<it->type<<std::endl;
			}
			std::cout<<" ----------------------------------------- "<<std::endl;
		}

		if( recvBuff[0] == 'a' && recvBuff[1] == 'a' ){
			std::cout<<"should reset the link table ..."<<std::endl;
			linkTable.clear();

	                Notify notify;
                        notify.head1 = 'a';
                        notify.head2 = 'a';

			/* send a response to the server */
                        memset(sendBuff, 0, sizeof( sendBuff ));
                        memcpy( sendBuff, &notify, sizeof( notify ) );
                        finder->write( client_fd, sendBuff, sizeof( notify ) );
                        std::cout<<"Notity message Response -----------------------------"<<std::endl;

		}
		
		if( recvBuff[0] == 'h' && recvBuff[1] == 'h' ){
			std::cout<<"should reset the topic information table ..."<<std::endl;
			topicTable.clear();// reset the topic information table

			Notify notify;
                        notify.head1 = 'h';
                        notify.head2 = 'h';

                        /* send a response to the server */
                        memset(sendBuff, 0, sizeof( sendBuff ));
                        memcpy( sendBuff, &notify, sizeof( notify ) );
                        finder->write( client_fd, sendBuff, sizeof( notify ) );
                        std::cout<<"Topics Update Notification Response -----------------------------"<<std::endl;

		}
		
		if( recvBuff[0] == 'j' && recvBuff[1] == 'j' ){
			std::cout<<"received a topic info ..............................."<<std::endl;
			TopicInfo TI;
			memcpy( &TI, recvBuff, sizeof( TI ) );
			topicTable.push_back( TI );
			
			std::cout<<"---------------------------------------"<<std::endl;
			std::cout<<" topicName = "<<TI.topicName<<std::endl;
			std::cout<<" ipAddr    = "<<TI.ipAddr<<std::endl;
			std::cout<<" port      = "<<TI.port<<std::endl;
			std::cout<<" topicType = "<<TI.topicType<<std::endl;
			std::cout<<" udp  port = "<<TI.udpPort<<std::endl;
			std::cout<<"---------------------------------------"<<std::endl;
		}
	}
}

void *NodeDiscovery::serverCallback( int fd, void *arg )
{
	memset( recvBuff, 0, sizeof( recvBuff ) ) ;
	int ret = finder->read( fd, recvBuff, sizeof( recvBuff ) );

	if( ret > 0 ){
		//std::cout<<"server recevied "<<ret<<" bytes data ..."<<std::endl;
		// this is a heartbeat message	from the client
		if( recvBuff[0] == 'c' && recvBuff[1] == 'c' ){
			std::cout<<"received a heartbeat, and send back ..."<<std::endl;
	
			memset( sendBuff, 0, sizeof( sendBuff ) );
			HeartBeats heart;
			heart.heart1 = 'c';
			heart.heart2 = 'c';
			memcpy( sendBuff, &heart, sizeof( heart ) );

			struct sockaddr_in clientInfo = finder->getClientInfo();
                        int ret = finder->write( server_fd, sendBuff, sizeof( heart ), clientInfo );			
			if( ret > 0 ){

			}
			else {
				exit(-1);
			}
			
			std::string s1 = inet_ntoa( clientInfo.sin_addr );
			for( auto it = linkTable.begin(); it != linkTable.end(); it ++ ){
				//std::string s2 = it->ipAddr;
				//if( s2.compare( s1 ) == 0 && it->port == ntohs( clientInfo.sin_port ) ){
				if( !strcmp( it->ipAddr, inet_ntoa( clientInfo.sin_addr ) ) && it->port == ntohs( clientInfo.sin_port ) ){
					it->timeOfClient = timerCount;
					std::cout<<"it->timeOfClient = "<<it->timeOfClient<<std::endl;
				}
			}

		}
		
		if( recvBuff[0] == 'd'&& recvBuff[1] == 'd'&& recvBuff[2] == 'd'&& recvBuff[3] == 'd' ){
			struct sockaddr_in clientInfo = finder->getClientInfo();			
			nodeInfo nI;
			char *ip = inet_ntoa( clientInfo.sin_addr );
			strcpy( nI.ipAddr, ip );
			nI.port = ntohs( clientInfo.sin_port );			
			nI.type = 1;	
			
			std::cout<<"ip = "<<nI.ipAddr<<std::endl;
			std::cout<<"port = "<<nI.port<<std::endl;
						
			linkTable.push_back( nI );
			
			sendNotifyMessage();
		}
		
		if( recvBuff[0] == 'a' && recvBuff[1] == 'b' && recvBuff[2] == 'c' && recvBuff[3] == 'd' ){
			memset( sendBuff, 0, sizeof(sendBuff) );
                        std::cout<<"send discoveried message ..."<<std::endl;
                        Header head;
                     	head.head1 = 'a';
               	        head.head2 = 'b';
                        head.head3 = 'c';
                        head.head4 = 'd';
                        memcpy( sendBuff, &head, sizeof( head ) );  
			
			struct sockaddr_in clientInfo = finder->getClientInfo();
			finder->write( server_fd, sendBuff, sizeof( head ), clientInfo );
		}
		
		if( recvBuff[0] == 'a' && recvBuff[1] == 'a' ){
			/*int i = 0;
			updateCount ++;
			for( auto it = linkTable.begin() + 1; it != linkTable.end(); it++, i ++ );
			
			std::cout<<"i ========================================================= "<<i<<std::endl;;
			if( updateCount == i ){*/
			updateCount ++;
			if( updateCount == ( linkTable.size() - 1 ) ){
				updateLinkTable();
				//-----------Added-----------//
				updateTopicTable();
				//---------------------------//	
				updateCount = 0;
			}
		}
	
		/* received a topic information from the udp client */
		if( recvBuff[0] == 'f' && recvBuff[1] == 'f' ){
			TopicInfo TI;
			memcpy( &TI, recvBuff, sizeof( TI ) );
			topicTable.push_back( TI );
			
			topicUpdateNotify();
		}
		
		/* when recieved the topics update notification response */
		if( recvBuff[0] == 'h' && recvBuff[1] == 'h' ){
			topicUpdateCount ++;
			std::cout<<"topicUpdateCount ======================== "<<topicUpdateCount<<std::endl;
			if( topicUpdateCount == ( linkTable.size() - 1 ) ){
				updateTopicTable();
				topicUpdateCount = 0;
			}		
		}
	}
}


void *NodeDiscovery::timerCallback( int fd, void *arg )
{
	uint64_t uread;
	int ret = read( fd, &uread, sizeof( uint64_t ) );
	if( ret == 8 ){

		if( findCount <= 5 && isFirstNode ){
				memset( sendBuff, 0, sizeof(sendBuff) );
				std::cout<<"send discovery message ..."<<findCount<<std::endl;
				Header head;
				head.head1 = 'a';
				head.head2 = 'b';
				head.head3 = 'c';
				head.head4 = 'd';
				memcpy( sendBuff, &head, sizeof( head ) );		
	
				if( finder->write( client_fd, (unsigned char*)sendBuff, sizeof( head ) ) ){
					findCount ++;				
				}
				else{
					exit(-1);
				}
		}
		// if there is not a server found
		else if( findCount == 6 && isFirstNode ){
			// create a new udp server
			std::cout<<" -----------------create a new server----------------- "<<std::endl;
			findCount = 7; // don't add anymore further
			
			eventBase.delEvent( timerEvent );
	                eventBase.delEvent( finderEvent );

        	        finder->closeSocket( client_fd ); // close the udp client
             		timer.closeTimerFd(  ); // close the timer
			
			client_fd = 0;
			timer_fd = 0;
			
			/***********************************************************************/
	                if( !finder->initSocketServer()  ){
        	                std::cerr<<"init the udp server failed ..."<<std::endl;
                	        exit(-1);
                	}
                	else std::cerr<<"init the udp server ... "<<std::endl;
			
			server_fd = finder->getServerFd();
			
                        if( server_fd <= 0 ) {
        	                std::cerr<<"server fd < 0, error: "<<server_fd<<std::endl;
                                exit(-1);
                        }

			serverEvent.fd = server_fd;
	                serverEvent.event = EPOLLIN;
        	        serverEvent.arg = NULL;
                	FUNC serverCb = std::bind( &NodeDiscovery::serverCallback, this, _1, _2 );
			serverEvent.callback = serverCb;
			/************************************************************************/

                        /************************************************************************/
                        if( !timer.createTimer() ){
	                        std::cerr<<"create timer failed ..."<<std::endl;
                                exit( -1 );
                        }
                        timer_fd = timer.getTimerFd();
                        if( timer_fd <= 0 ) {
        	                std::cerr<<"timer fd < 0, error: "<<timer_fd<<std::endl;
                                exit(-1);
                        }
                        timer.setTimer( 1, 0, 1, 0 );   // 1s/per

                        timerEvent.fd = timer_fd;
                        timerEvent.event = EPOLLIN;
                        timerEvent.arg = NULL;

                        FUNC timerCb = std::bind( &NodeDiscovery::timer3Callback, this, _1, _2 );
                        timerEvent.callback = timerCb;
                        /*************************************************************************/

			// add a new server event to the epoll
        		eventBase.addEvent( serverEvent );
			eventBase.addEvent( timerEvent );
	
	                //struct sockaddr_in serverInfo = finder->getServerInfo();
			struct sockaddr_in serverInfo = finder->getLocalIp( server_fd );		
			
			/* update the link table */	
       	 	        nodeInfo nI;
               	 	char *ip = inet_ntoa( serverInfo.sin_addr );
	                strcpy( nI.ipAddr, ip );
        	        nI.port = ntohs( serverInfo.sin_port );
	                nI.type = 2;;
	
        	        linkTable.push_back( nI );

			/* reset the port */
			port = ntohs( serverInfo.sin_port );
			
			/* reset the udp type */			
			udpType = udpServer;
		}
		else {
			std::cout<<"----------------------Timer Test"<<std::endl;
		}
	}
}

void *NodeDiscovery::timer2Callback( int fd, void *arg )
{
        uint64_t uread;
        int ret = read( fd, &uread, sizeof( uint64_t ) );
        if( ret == 8 ){
		timerCount ++;
		if( timerCount > 999 ) timerCount = 0;
		
		if( !isServerOffLine ){
			if( ( timerCount - timeOfServer ) > 3 ){
				std::cout<<"----------- the udp server is out of net ----------"<<std::endl;
			
				/* delete the server in the linkTable */
				auto item = linkTable.begin();
				//------------------------ Added -----------------------------//
				auto it2 = topicTable.begin();
                                for( ; it2 != topicTable.end(); ){
                                        // firstly, find which one should be deleted from the topic info table
                                        if( !strcmp( it2->ipAddr, item->ipAddr ) && it2->udpPort == item->port ){
                                                it2 = topicTable.erase( it2 );  //and then, get the it's iterator and erase it
                                        }
                                        else it2 ++;
                                }
				//------------------------------------------------------------//		
				linkTable.erase( item ); // delete the first node info inthe link table
		
				/* search the link table and find the first node */
				struct sockaddr_in localAddr = finder->getLocalIp( client_fd );
				auto it = linkTable.begin();
	
				if( !strcmp(it->ipAddr, inet_ntoa( localAddr.sin_addr ) ) && it->port == ntohs( localAddr.sin_port ) ){
					//reinit a udp server
					eventBase.delEvent( timerEvent );
					eventBase.delEvent( clientEvent );
					
					finder->closeSocket( client_fd );
					timer.closeTimerFd(); 
				
					client_fd = 0;
					timer_fd = 0;

					/*********************************************************************************/	
					if( !finder->initSocketServer() ){
						std::cerr<<"reInit the udp server failed ..."<<std::endl;
						exit(-1);
					}	
					else std::cerr<<"------------- reInit the udp server -------------"<<std::endl;
					server_fd = finder->getServerFd();
			
					if( server_fd <= 0 ){
						std::cerr<<"server fd < 0, error: "<<server_fd <<std::endl;
						exit( -1 );
					}
				
					serverEvent.fd = server_fd;
					serverEvent.event = EPOLLIN;
					serverEvent.arg = NULL;
					FUNC serverCb = std::bind( &NodeDiscovery::serverCallback, this, _1, _2 );
					serverEvent.callback = serverCb;
					/*********************************************************************************/
		
					/*********************************************************************************/
					if( !timer.createTimer() ){
						std::cerr<<"reCreate timer failed ..."<<std::endl;
						exit(-1);
					}	

					timer_fd = timer.getTimerFd();
					if( timer_fd <= 0 ){
						std::cerr<<"timer fd < 0, error: "<<timer_fd<<std::endl;
						exit(-1);
					}
					timer.setTimer( 1, 0, 1, 0 );
			
					timerEvent.fd = timer_fd;
					timerEvent.event = EPOLLIN;
					timerEvent.arg = NULL;
				
					FUNC timerCb = std::bind( &NodeDiscovery::timer3Callback, this, _1, _2 );
					timerEvent.callback = timerCb;

					eventBase.addEvent( serverEvent );
					eventBase.addEvent( timerEvent );
					/*********************************************************************************/

					// change the client info to the server in linkTable
					//struct sockaddr_in serverInfo = finder->getServerInfo();
					struct sockaddr_in serverInfo = finder->getLocalIp( server_fd );
					
					char *ip = inet_ntoa( serverInfo.sin_addr );
					strcpy( it->ipAddr, ip );
					it->port = ntohs( serverInfo.sin_port );
					it->type = 2;	
			
					/* reset the port */
					port = ntohs( serverInfo.sin_port );
		
					/* reset the udp type */
					udpType = udpServer;	
	
					sleep(1);
					// firstly, send the notify message to the remain clients
					sendNotifyMessage();
					// when a node is exit, update the link table to all the clients
					//sleep(1);
					//updateLinkTable();
					needHeart = false;
				}
				else isServerOffLine = true;
			}
		}

		if( needHeart ){
			//std::cout<<"timer2 ......................."<<std::endl;	
        	        // mantain a heartbeat mechanic
			memset( sendBuff, 0, sizeof( sendBuff ) );
	                HeartBeats heart;
        	        heart.heart1 = 'c';
                	heart.heart2 = 'c';
	                memcpy( sendBuff, &heart, sizeof( heart ) );

        	        int ret = finder->write( client_fd, ( unsigned char* )sendBuff, sizeof( heart ) );
                	if( ret > 0 ){
	
        	        }
                	else {
       				exit(-1);
	                }
		}	
	}
}

void *NodeDiscovery::timer3Callback( int fd, void *arg )
{
        uint64_t uread;
        int ret = read( fd, &uread, sizeof( uint64_t ) );
        if( ret == 8 ){
		timerCount ++;
	//	std::cout<<"timer3 -----------------------"<<timerCount<<std::endl;
                if( timerCount > 999 ) timerCount = 0;
		
		auto item = linkTable.begin();
		for( auto it = linkTable.begin(); it != linkTable.end(); it ++ ){
		//	std::cout<<"port------------------ : "<<it->port<<std::endl;
			if( ( timerCount - it->timeOfClient ) > 3 && it->type == 1 ){
				std::cout<<it->ipAddr<<": "<<it->port<<" client is out of the net ..."<<std::endl;
			
				// find the iterator  from the link table
				item = find_if( linkTable.begin(), linkTable.end(), \
											[it]( nodeInfo found ){ \
										    		return !strcmp(it->ipAddr, found.ipAddr) && it->port == found.port; \
											} );
				/* delete the topic info in the topic table */
				//------------------------------- Added ---------------------------------------//
				auto it2 = topicTable.begin();
				for( ; it2 != topicTable.end(); ){
					// firstly, find which one should be deleted from the topic info table
					if( !strcmp( it2->ipAddr, it->ipAddr ) && it2->udpPort == it->port ){
						it2 = topicTable.erase( it2 );  //and then, get the it's iterator and erase it
					}
					else it2 ++;
				}
				//----------------------------------------------------------------------------//
			}
		}

		/* then delete it from the link table */
		if( item != linkTable.end() && item != linkTable.begin() ){
			linkTable.erase( item );
			sendNotifyMessage();
		}
	}
}


void *NodeDiscovery::finderCallback( int fd, void *arg )
{
	std::cout<<"--------------------------finderCallback"<<std::endl;
	
	//std::cout<<"udp client start receiving ... "<<std::endl;
		memset( recvBuff, 0, sizeof( recvBuff ) );
		int ret = finder->read( fd, recvBuff, sizeof( recvBuff ) );
		if( ret > 0 ){
			if( recvBuff[0] == 'a'&& recvBuff[1] == 'b'&& recvBuff[2] == 'c'&& recvBuff[3] == 'd' ){ // if received a verify from a server 
				
				isFirstNode = false;
				findCount = 7;
						
				// delete the old client event and create a new one
				eventBase.delEvent( finderEvent );
				eventBase.delEvent( timerEvent );				
				
				// close the old file descriptor
				finder->closeSocket( client_fd );
				timer.closeTimerFd();
				
				client_fd = 0;
				timer_fd = 0;
	
				/***********************************************************************/
				if( !finder->initSocketClient() ){
					std::cerr<<"reInit the udp client failed ..."<<std::endl;
					exit(-1);
				}
				else std::cerr<<"reInit the udp client ... "<<std::endl;

				client_fd = finder->getClientFd();
                                if( client_fd <= 0 ) {
                                        std::cerr<<"client fd < 0, error: "<<client_fd<<std::endl;
                                        exit(-1);
                                }
							
				clientEvent.fd = client_fd;
				clientEvent.event  = EPOLLIN;
				clientEvent.arg = NULL;
				FUNC clientCb = std::bind( &NodeDiscovery::clientCallback, this, _1, _2 );
				clientEvent.callback = clientCb;
				/*************************************************************************/

				/************************************************************************/	
				if( !timer.createTimer() ){
					std::cerr<<"reCreate timer failed ..."<<std::endl;
					exit( -1 );
				}
				timer_fd = timer.getTimerFd();
				if( timer_fd <= 0 ) {
					std::cerr<<"timer fd < 0, error: "<<timer_fd<<std::endl;
					exit(-1);
				}
				timer.setTimer( 1, 0, 1, 0 );   // 1s/per
				
				timerEvent.fd = timer_fd;
			        timerEvent.event = EPOLLIN;
			        timerEvent.arg = NULL;

			        FUNC timerCb = std::bind( &NodeDiscovery::timer2Callback, this, _1, _2 );
        			timerEvent.callback = timerCb;
				/*************************************************************************/
				
				// add new event to the epoll	
				eventBase.addEvent( clientEvent );
				eventBase.addEvent( timerEvent );
			
				// when create a new udp client, have to send a verify information to help the server establish a link information table
				memset( sendBuff, 0, sizeof( sendBuff ) );
				sendBuff[0] = 'd';
				sendBuff[1] = 'd';
				sendBuff[2] = 'd';
				sendBuff[3] = 'd';
				int ret = finder->write( client_fd, sendBuff, 4 );
		
				/* reset the port */
				struct sockaddr_in clientInfo = finder->getLocalIp( client_fd )  ;
				port = ntohs( clientInfo.sin_port );

				/* reset the udp type */
				udpType = udpClient;
			}
		}
}


void NodeDiscovery::updateLinkTable()
{
	/* update the link table to all the clients */
        std::cout<<"should update the link table of the client ..................."<<std::endl;
        for( auto it = linkTable.begin() + 1; it != linkTable.end(); it ++ ){
		struct sockaddr_in client;
        	client.sin_family = AF_INET;
        	client.sin_addr.s_addr = inet_addr( it->ipAddr );
                client.sin_port = htons( it->port );
        
	        for( auto it2 = linkTable.begin(); it2 != linkTable.end(); it2 ++ ){
                	Node node;
                        node.head1 = 'b';
                        node.head2 = 'b';
                        node.head3 = 'b';
                        strcpy( node.ipAddr, it2->ipAddr );
                        node.port = it2->port;
                        node.type = it2->type;

                        std::cout<<"ip ================== "<<node.ipAddr<<std::endl;
                        std::cout<<"port ================ "<<node.port<<std::endl;
                        std::cout<<"type ================ "<<node.type<<std::endl;

                        memset(sendBuff, 0, sizeof( sendBuff ));
                        memcpy( sendBuff, &node, sizeof( node ) );
                        int ret = finder->write( server_fd, sendBuff, sizeof( node ), client );
                        if( ret <= 0 ) exit(-1);
                }
	}

}

void NodeDiscovery::updateTopicTable()
{
	/* update the topic table to all the clients */
	std::cout<<" Should update the topic tables of the clients ...................... "<<std::endl;
	for( auto it = linkTable.begin() + 1; it != linkTable.end(); it ++ ){
                struct sockaddr_in client;
                client.sin_family = AF_INET;
                client.sin_addr.s_addr = inet_addr( it->ipAddr );
                client.sin_port = htons( it->port );

                for( auto it2 = topicTable.begin(); it2 != topicTable.end(); it2 ++ ){
			TopicInfo TI;
			TI.head1 = 'j';
			TI.head2 = 'j';
			strcpy( TI.topicName, it2->topicName );
			TI.port = it2->port;
			strcpy( TI.ipAddr, it2->ipAddr );
			TI.topicType = it2->topicType;
			TI.udpPort = it2->udpPort;	
			
                        memset(sendBuff, 0, sizeof( sendBuff ));
                        memcpy( sendBuff, &TI, sizeof( TI ) );
                        int ret = finder->write( server_fd, sendBuff, sizeof( TI ), client );
                        if( ret <= 0 ) exit(-1);
                }
        }
}

void NodeDiscovery::sendNotifyMessage()
{
	for( auto it = linkTable.begin() + 1; it != linkTable.end(); it ++ ){
        	struct sockaddr_in client;
                client.sin_family = AF_INET;
                client.sin_addr.s_addr = inet_addr( it->ipAddr );
                client.sin_port = htons( it->port );
                                
                Notify notify;
                notify.head1 = 'a';
                notify.head2 = 'a';
        
                memset(sendBuff, 0, sizeof( sendBuff ));
                memcpy( sendBuff, &notify, sizeof( notify ) );
                finder->write( server_fd, sendBuff, sizeof( notify ), client );
                
		std::cout<<"send notity message -----------------------------"<<std::endl;
	}

}

void NodeDiscovery::topicUpdateNotify()
{
        for( auto it = linkTable.begin() + 1; it != linkTable.end(); it ++ ){
                struct sockaddr_in client;
                client.sin_family = AF_INET;
                client.sin_addr.s_addr = inet_addr( it->ipAddr );
                client.sin_port = htons( it->port );

                Notify notify;
                notify.head1 = 'h';
                notify.head2 = 'h';

                memset(sendBuff, 0, sizeof( sendBuff ));
                memcpy( sendBuff, &notify, sizeof( notify ) );
                finder->write( server_fd, sendBuff, sizeof( notify ), client );

                std::cout<<"topics update notification -----------------------------"<<std::endl;
        }
	
	topicUpdateStatus = true;
}

void NodeDiscovery::initFinderClient()
{
	if( !timer.createTimer() ){
		std::cerr<<"create timer failed ... "<<std::endl;
		exit(-1);
	}
	timer_fd = timer.getTimerFd();
	if( timer_fd <= 0 ){
		std::cout<<"timer_fd < 0, error: "<<timer_fd<<std::endl;
		exit(-1);
	}	

	timer.setTimer( 1, 0, 0, 100000000 );	// 100ms/per
	
	timerEvent.fd = timer_fd;
	timerEvent.event = EPOLLIN;
	timerEvent.arg = NULL;
	
	FUNC timerCb = std::bind( &NodeDiscovery::timerCallback, this, _1, _2 );
	timerEvent.callback = timerCb;


	if( !finder->initSocketClient() ){
		std::cerr<<"init the udp client failed ..."<<std::endl;
		exit(-1);
	}
	else std::cerr<<"init the udp client ..."<<std::endl;

	client_fd = finder->getClientFd();
	if( client_fd <= 0 ){
		std::cerr<<"client_fd < 0, error: "<<client_fd <<std::endl;
		exit(-1);
	}

	finderEvent.fd = client_fd;
	finderEvent.event = EPOLLIN;
	finderEvent.arg = NULL;

	FUNC finderCb = std::bind( &NodeDiscovery::finderCallback, this, _1, _2 );
	finderEvent.callback = finderCb;
	
}

void NodeDiscovery::startDiscovery()
{

	isRunning = true;
	
	initFinderClient();

	eventBase.addEvent( timerEvent );
	eventBase.addEvent( finderEvent );
	
	while( isRunning ){
		eventBase.dispatcher();
	}
}

void NodeDiscovery::stopDiscovery()
{
	if( isRunning ){
		isRunning = false;
	}
}

void NodeDiscovery::sendTopicInfo( TopicInfo &topicInfo )
{
	std::cout<<"send a topic info ..............."<<std::endl;
	topicInfo.head1 = 'f';
	topicInfo.head2 = 'f';
	
	memset( sendBuff, 0, sizeof( sendBuff ) );
	memcpy( sendBuff, &topicInfo, sizeof( topicInfo ) );
	uint8_t ret = finder->write( client_fd, sendBuff, sizeof( topicInfo ) );
	
//	if( ret <= 0 ) exit(-1);
}

void NodeDiscovery::printTopicTable()
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

void NodeDiscovery::setTopicUpdateStatus( bool status )
{
	topicUpdateStatus = status;
}

bool NodeDiscovery::getTopicUpdateStatus()
{
	return topicUpdateStatus;
}

}












