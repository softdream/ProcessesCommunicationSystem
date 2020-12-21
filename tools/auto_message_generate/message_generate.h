#ifndef __MESSAGE_GENERATE_H_
#define __MESSAGE_GENERATE_H_

#include <iostream>
#include <fstream>
#include <cassert>
#include <string>
#include <string.h>
//#include <io.h>
#include <unistd.h>
#include <vector>
#include <map>
#include <algorithm> 

void readMessageInfo(std::string file);
void writeMessageInfo(std::string file);
void getMessageLists(std::string file);

#endif
