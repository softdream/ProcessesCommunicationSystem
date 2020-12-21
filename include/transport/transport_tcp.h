#ifndef __TRANSPORT_TCP_H_
#define __TRANSPORT_TCP_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>

#include <vector>

#include "transport.h"

namespace pcs
{
	
class TransportTCP : public Transport
{
public:
	TransportTCP():port(0), transportType(0)
	{

	}
	
	TransportTCP( const TransportTCP &obj ): serverFd( obj.serverFd ),
						 clientFd( obj.clientFd ),
						 connectedFd( obj.connectedFd ),
						 port( obj.port ),
						 transportType( obj.transportType )
	{	
		memcpy( &server_sockaddr, &obj.server_sockaddr, sizeof( server_sockaddr ) );
		memcpy( &client_sockaddr, &obj.client_sockaddr, sizeof( client_sockaddr ) );
		memcpy( &my_server_addr, &obj.my_server_addr, sizeof( my_server_addr ) );
		memcpy( &connected_addr, &obj.connected_addr, sizeof( connected_addr ) );

	}

	TransportTCP& operator=( const TransportTCP &other )
	{
		if( this == &other ) return *this;

		serverFd = other.serverFd;
		clientFd = other.clientFd;
		connectedFd = other.connectedFd;
		port = other.port;
		transportType = other.transportType;
		memcpy( &server_sockaddr, &other.server_sockaddr, sizeof( server_sockaddr ) );
                memcpy( &client_sockaddr, &other.client_sockaddr, sizeof( client_sockaddr ) );
                memcpy( &my_server_addr, &other.my_server_addr, sizeof( my_server_addr ) );
                memcpy( &connected_addr, &other.connected_addr, sizeof( connected_addr ) );
		return *this;
	}

	virtual ~TransportTCP()
	{
		close( serverFd );
		close( clientFd );
	}

        virtual int read( int fd, unsigned char *buffer, int size );
        virtual int write( int fd, unsigned char *buffer, int size );
	virtual int write( int fd, unsigned char *buffer, int size, int port )
	{

	}
	virtual int write( int fd, unsigned char *buffer, int size, struct sockaddr_in &clientAddr )
	{

	}
        virtual void closeSocket( int fd );

	virtual bool connectSocket( const int port, const char *ipAddr );
	virtual bool connectSocket( const int sockFd, const int port, const char *ipAddr );

        virtual const int getTransportType();

        virtual bool initSocketServer();
        virtual bool initSocketClient();


	virtual struct sockaddr_in getClientInfo();
	virtual struct sockaddr_in getServerInfo();

	virtual int Accept();

	virtual int getServerFd()
	{
		return serverFd;
	}
	
	virtual int getClientFd()
	{
		return clientFd;
	}
	
	
	const int getPort()
	{
		return port;
	}

	const std::vector<int> getConnectedFd()
	{
		return connectedFd;
	}

private:
	int serverFd;
	int clientFd;
	
	std::vector<int> connectedFd;

	struct sockaddr_in server_sockaddr;
	struct sockaddr_in client_sockaddr;
	struct sockaddr_in my_server_addr;
        struct sockaddr_in connected_addr;
	socklen_t server_len, client_len, my_server_len, connected_len;
	
	int port;

	int transportType;

};

}


#endif
