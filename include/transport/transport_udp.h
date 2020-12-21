#ifndef __TRANSPORT_UDP_H_
#define __TRANSPORT_UDP_H_

#include <iostream>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "transport.h"

namespace pcs
{

class TransportUDP : public Transport
{
public:
	TransportUDP() : transportType(0)
	{
	        bzero( &server_addr_, sizeof( server_addr_ ) );
	        server_recv_len = sizeof( server_recv_addr );
	}
	virtual ~TransportUDP()
	{
		close( serverFd );
		close( clientFd );
	}

        virtual int read( int fd, unsigned char *buffer, int size );
        virtual int write( int fd, unsigned char *buffer, int size );
	virtual int write( int fd, unsigned char *buffer, int size, int port );
	virtual int write( int fd, unsigned char *buffer, int size, struct sockaddr_in &clientAddr );	

        virtual void closeSocket( int fd );

	virtual bool connectSocket( const int port, const char *ipAddr )
	{

	}
	
	virtual bool connectSocket( const int sockFd, const int port, const char *ipAddr )
        {

        }
	

        virtual const int getTransportType();

	virtual bool initSocketServer();
	virtual bool initSocketClient();
	//bool setSocket();

	virtual int getClientFd()
	{
		return clientFd;
	}
	
	virtual int getServerFd()
	{
		return serverFd;
	}

	virtual struct sockaddr_in getClientInfo()
	{
		return server_recv_addr;
	}
		
	virtual struct sockaddr_in getServerInfo()
	{
		return server_addr_;
	}
	
	virtual int Accept()
	{

	}

private:
	int serverFd;
	int clientFd;
	struct sockaddr_in server_addr_;
	struct sockaddr_in client_addr_;
		
        struct sockaddr_in server_recv_addr;
        socklen_t server_recv_len;
	
	struct sockaddr_in client_dest_addr;
	//socklen_t client_dest_len;	
	int transportType;
};

}

#endif


