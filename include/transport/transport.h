#ifndef __TRANSPORT_H_
#define __TRANSPORT_H_

#include <iostream>
#include <functional>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if_ether.h>
#include <net/if.h>

#include <string.h>

#define ETH_NAME  "ens33"  

namespace pcs
{
typedef std::function<int( int, unsigned char, int )> Callback;

class Transport
{
public:
	Transport(){}
	virtual ~Transport(){};
	
	virtual int read( int fd, unsigned char *buffer, int size ) = 0;
	virtual int write( int fd, unsigned char *buffer, int size ) = 0;
	virtual int write( int fd, unsigned char *buffer, int size, int port ) = 0;
	virtual int write( int fd, unsigned char *buffer, int size, struct sockaddr_in &clientAddr ) = 0;	

	virtual void closeSocket( int fd ) = 0;

	virtual bool connectSocket( const int port, const char *ipAddr ) = 0;
	virtual bool connectSocket( const int sockFd, const int port, const char *ipAddr ) = 0;

	virtual const int getTransportType() = 0;

        virtual bool initSocketServer() = 0;
        virtual bool initSocketClient() = 0;
	
	virtual int getClientFd() = 0;
	virtual int getServerFd() = 0;


	virtual struct sockaddr_in getClientInfo() = 0;
	virtual struct sockaddr_in getServerInfo() = 0;

	void setReadCallback( const Callback &cb )
	{
		read_cb_ = cb;
	}

	void setWriteCallback( const Callback &cb )
	{
		write_cb_ = cb;
	}
	

	struct sockaddr_in getLocalIp( int sock_fd )
	{
		struct sockaddr_in  sin;
		struct ifreq ifr;
		
		struct sockaddr_in local;
		socklen_t Len = sizeof( local );		

	        if (getsockname( sock_fd, (struct sockaddr*)&local, &Len ) == -1){
        	        exit(-1);
        	}

        	strncpy(ifr.ifr_name,ETH_NAME,IFNAMSIZ);
	        ifr.ifr_name[IFNAMSIZ - 1] = 0;
        	if(ioctl(sock_fd,SIOCGIFADDR,&ifr) == 0) {
                	memcpy(&sin,&ifr.ifr_addr,sizeof(sin));
	        }
		
		struct sockaddr_in localInfo;
		localInfo.sin_port = local.sin_port;
		localInfo.sin_addr = sin.sin_addr;

		return localInfo;
	}
	
	virtual int Accept() = 0;

private:
	Callback read_cb_;
	Callback write_cb_;
	//Callback connection_cb_;
};	

}

#endif
