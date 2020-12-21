#include "pcs.h"
#include <unistd.h>

unsigned char sendBuff[10] = {};

typedef struct{
	uint8_t h1;
	uint8_t h2;
	uint8_t h3;
	uint8_t h4;
	uint8_t h5;
}Type;
	

int main()
{
	Type t;
	t.h1 = 'H';
	t.h2 = 'e';
	t.h3 = 'l';
	t.h4 = 'l';
	t.h5 = 'o';


	std::cout<<"program started ..."<<std::endl;
	pcs::PCS n;
	pcs::Publisher pub =  n.advertise ("test1");

	//n.subscribe<int>( "test1" );

	//n.subscribe<char>( "node2" );

	sleep(2);	

	n.printTopicTable();

	while(1){
		memset( sendBuff, 0, sizeof( sendBuff ) );
				
		pub.publishMessage<Type>( t, sendBuff );
		sleep(1);
	}	

	return 0;
}
