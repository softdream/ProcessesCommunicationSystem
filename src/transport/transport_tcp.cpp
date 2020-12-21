#include "transport_tcp.h"
#include <algorithm>

namespace pcs
{
	
bool TransportTCP::initSocketServer()	
{
	serverFd = socket( AF_INET, SOCK_STREAM, 0 );
	if( serverFd < 0 ){
		std::cerr<<"sock tcp server falied ..."<<serverFd<<std::endl;
		return false;
	}
	
	server_sockaddr.sin_family = AF_INET;
	server_sockaddr.sin_port = htons(0);
	server_sockaddr.sin_addr.s_addr = htonl( INADDR_ANY );
	
	server_len  = sizeof( server_sockaddr );
	
	int on;
	setsockopt( serverFd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof( on ) );

        if( bind( serverFd, ( struct sockaddr* )&server_sockaddr, server_len)  == -1 ){
                std::cout<<"tcp server bind failed ..."<<std::endl;
                return false; 
        }

	/* get the port of the tcp server */
	my_server_len = sizeof( my_server_addr );
	getsockname( serverFd, ( struct sockaddr* )&my_server_addr, &my_server_len );
	port = ntohs( my_server_addr.sin_port );
	std::cout<<"tcp server port = "<<port<<std::endl;

        if( listen( serverFd, 5 ) == -1 ){
                std::cout<<"tcp server listen failed ..."<<std::endl;
        }
	
	transportType = 3; // tcp server
	
	/* receive timeout */
	struct timeval tv_timeout;
	tv_timeout.tv_sec = 1;
    	tv_timeout.tv_usec = 0;
    	setsockopt(serverFd, SOL_SOCKET, SO_RCVTIMEO, &tv_timeout, sizeof(tv_timeout));
	
	std::cerr<<"initialize the tcp server successfully ..."<<std::endl;
	return true;
}

int TransportTCP::Accept()
{
	int connected_fd  = 0;
	connected_len = sizeof( connected_addr );
	connected_fd = accept( serverFd, ( struct sockaddr* )&connected_addr, & connected_len );
	if( connected_fd == -1 ){
		std::cerr<<"accepted error ..."<<std::endl;
		return false;
	}
	std::cerr<<"accepted successfully: "<<connected_fd<<std::endl;
	connectedFd.push_back( connected_fd );
	return connected_fd;
}

bool TransportTCP::initSocketClient()
{
	clientFd = socket( AF_INET, SOCK_STREAM, 0 );
	if( clientFd == -1 ){
		std::cerr<<"tcp client socket error ..."<<std::endl;
		return false;
	}
	transportType = 4; // tcp client
	std::cerr<<"initialize tcp client successfully ..."<<std::endl;
	return true;
}

bool TransportTCP::connectSocket( const int port, const char *ipAddr )
{
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons( port );
	server_addr.sin_addr.s_addr = inet_addr( ipAddr );
	bzero( &( server_addr.sin_zero ), sizeof( server_addr.sin_zero ) );
	
	if( connect( clientFd, (struct sockaddr *)&server_addr,sizeof(struct sockaddr_in ) ) == -1 ){
		std::cerr<<"connected error ..."<<std::endl;
		return false;
	}
	std::cerr<<"conneted ..."<<std::endl;
	return true;
}
	
bool TransportTCP::connectSocket( const int sockFd, const int port, const char *ipAddr )
{
        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons( port );
        server_addr.sin_addr.s_addr = inet_addr( ipAddr );
        bzero( &( server_addr.sin_zero ), sizeof( server_addr.sin_zero ) );

        if( connect( sockFd, (struct sockaddr *)&server_addr,sizeof(struct sockaddr_in ) ) == -1 ){
                std::cerr<<"connected error ..."<<std::endl;
                return false;
        }
        std::cerr<<"----------------------- TCP Link Connected ---------------------"<<std::endl;
        return true;
}


int TransportTCP::read( int fd, unsigned char *buffer, int size )
{
	memset( buffer, 0, size );
	int ret = recv( fd, buffer, size, 0 );
	if( ret > 0 ){
		std::cout<<"received "<<ret<<" bytes: "<<std::hex<<buffer<<std::endl;
	}	
	else{
		//std::cerr<<"receive data falied ..."<<std::endl;
		return ret;
	}
	return ret;
}

int TransportTCP::write( int fd, unsigned char *buffer, int size )
{
	int ret = send( fd, (char*)buffer, size, MSG_NOSIGNAL );
	if( ret > 0 ){
		std::cerr<<"send data: "<<ret<<std::endl;
	}
	else{
		std::cerr<<"send data failed ..."<<std::endl;
	}
	return ret;
}

void TransportTCP::closeSocket( int fd ) 
{
	transportType = 0;
	
	auto iter = std::find( connectedFd.begin(), connectedFd.end(), fd );
	if (iter != connectedFd.end())
		connectedFd.erase(iter);	

	close( fd );	
}

const int TransportTCP::getTransportType()
{
	return transportType;
}

struct sockaddr_in TransportTCP::getClientInfo()
{
	return connected_addr;
}

struct sockaddr_in TransportTCP::getServerInfo()
{

}


}

