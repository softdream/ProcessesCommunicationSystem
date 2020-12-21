#include "pcs.h"
#include <unistd.h>

typedef struct{
	char arg1;
	char arg2;
	char arg3;
	char arg4;
}Arg;

void testCb( void *arg )
{
	std::cout<<"callback function ------------------"<<std::endl;

	Arg a = pcs::getData<Arg>( arg );
	
	std::cout<<"a.arg1 = "<<a.arg1<<std::endl;
	std::cout<<"a.arg2 = "<<a.arg2<<std::endl;
        std::cout<<"a.arg3 = "<<a.arg3<<std::endl;
        std::cout<<"a.arg4 = "<<a.arg4<<std::endl;
	
}

int main()
{
	std::cout<<"program started ..."<<std::endl;
	pcs::PCS n;
	//pcs::Publisher pub =  n.advertise ("node1");

	n.subscribe<int>( "test1", testCb );


	sleep(2);	

	n.printTopicTable();

	n.spin();

	return 0;
}
