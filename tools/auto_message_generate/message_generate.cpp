#include "message_generate.h"


std::vector<std::string> strVector;
std::map<std::string, int> arrayLengthMap;
std::map<std::string, std::string> namesMap;
std::map<std::string, std::string> arrayNamesMap;
std::map<std::string, std::string> typesMap;
std::map<std::string, std::string> arrayTypesMap;

// 逐行读取
void readMessageInfo(std::string file)
{

	std::ifstream infile;
	infile.open(file.data());   
	assert(infile.is_open());   

	std::ofstream outfile(file, std::ios::app);
	assert(outfile.is_open());

	std::string s;
	while (std::getline(infile, s)) {
		std::cout << s << std::endl;
		//outfile << s;
		//outfile << std::endl;

		/////////////// deal with the arrays //////////////////
		size_t start_pose = s.find("[");
		size_t end_pose = s.find("]");
		if (start_pose != std::string::npos && end_pose != std::string::npos) {
			//std::cout << "string length = " << s.length() << std::endl;
			//std::cout << "start_pose = " << start_pose << std::endl;
			//std::cout << "end_pose = " << end_pose << std::endl;

			std::string arrayLength = s.substr(start_pose + 1, end_pose - start_pose - 1);
			std::cout << "arrayLength = " << arrayLength << std::endl;

			int number = std::stoi(arrayLength);
			std::cout << "number = " << number << std::endl;
			arrayLengthMap.insert(std::make_pair(s, number));

			std::string arrayName = s.substr((s.find(" ") + 1), (s.find("[") - (s.find(" ") + 1)));
			std::cout << "array name = " << arrayName << std::endl;
			arrayNamesMap.insert(std::make_pair(s, arrayName));

			std::string arrayType = s.substr(0, s.find(" "));
			std::cout << "array type = " << arrayType << std::endl;
			arrayTypesMap.insert(std::make_pair(s, arrayType));
		}
		///////////////////////////////////////////////////////////////////
		else {
			strVector.push_back(s);

			std::string name = s.substr((s.find(" ") + 1));
			std::cout << "variable name = " << name << std::endl;
			namesMap.insert(std::make_pair(s, name));

			std::string type = s.substr(0, s.find(" "));
			std::cout << "variable type = " << type << std::endl;
			typesMap.insert(std::make_pair(s, type));
		}
		s.clear();
	}
	infile.close();             //关闭文件输入流 
	outfile.close();
}


void writeMessageInfo(std::string file)
{
	std::string rmseFile = file;
	if (access(rmseFile.c_str(), 0) == 0) {//文件存在
		if (remove(rmseFile.c_str()) == 0) {
			std::cout << "delete the existed file" << std::endl;
		}
		else {
			std::cout << "delte failed" << std::endl;
		}
	}

	std::ofstream outfile(file, std::ios::app);
	assert(outfile.is_open());

	std::string s1;

	std::string head = file.substr(0, file.find(".h"));
	std::transform(head.begin(), head.end(), head.begin(), ::toupper);
	std::cout << "head = " << head << std::endl;

	outfile << "#ifndef __" << head << "_H_" << std::endl;
	outfile << "#define __" << head << "_H_" << std::endl << std::endl;

	///////////////////////////////// Some Defines ////////////////////////////////////
	outfile <<"#include <stdint.h>"<<std::endl;
	s1.clear();
	s1 = "#include <string.h>";
	outfile << s1;
	outfile << std::endl << std::endl;

	/*outfile << "#ifndef unsigned char uint8_t" << std::endl;
	s1.clear();
	s1 = "\t#define unsigned char uint8_t";
	outfile << s1;
	outfile << std::endl;
	outfile <<"#endif"<<std::endl<<std::endl;

	outfile << "#ifndef char int8_t" << std::endl;
	s1.clear();
	s1 = "\t#define char int8_t";
	outfile << s1;
	outfile << std::endl;
	outfile <<"#endif"<<std::endl<<std::endl;

	outfile << "#ifndef unsigned int uint32_t" << std::endl;
	s1.clear();
	s1 = "\t#define unsigned int uint32_t";
	outfile << s1;
	outfile << std::endl;
	outfile <<"#endif"<<std::endl<<std::endl;

	outfile << "#ifndef int int32_t" << std::endl;
	s1.clear();
	s1 = "\t#define int int32_t";
	outfile << s1;
	outfile << std::endl;
	outfile <<"#endif"<<std::endl<<std::endl;

	outfile << "#ifndef float float32_t" << std::endl;
	s1.clear();
	s1 = "\t#define float float32_t";
	outfile << s1;
	outfile << std::endl;
	outfile <<"#endif"<<std::endl<<std::endl;

	outfile << "#ifndef unsigned float ufloat32_t" << std::endl;
	s1.clear();
	s1 = "\t#define unsigned float ufloat32_t";
	outfile << s1;
	outfile << std::endl;
	outfile <<"#endif"<<std::endl<<std::endl;

	outfile << "#ifndef long int64_t" << std::endl;
	s1.clear();
	s1 = "\t#define long int64_t";
	outfile << s1;
	outfile << std::endl;
	outfile <<"#endif"<<std::endl<<std::endl;

	outfile << "#ifndef unsigned long uint64_t" << std::endl;
	s1.clear();
	s1 = "\t#define unsigned long uint64_t";
	outfile << s1;
	outfile << std::endl;
	outfile <<"#endif"<<std::endl<<std::endl;

	outfile << "#ifndef double double64_t" << std::endl;
	s1.clear();
	s1 = "\t#define double double64_t";
	outfile << s1;
	outfile << std::endl;
	outfile <<"#endif"<<std::endl<<std::endl;

	outfile << "#ifndef unsigned double udouble64_t" << std::endl;
	s1.clear();
	s1 = "\t#define unsigned double udouble64_t";
	outfile << s1;
	outfile << std::endl << std::endl;
	outfile <<"#endif"<<std::endl<<std::endl;*/
	/////////////////////////////////////////////////////////////////////////////

	////////////////////////////////// Start /////////////////////////////////////////
	outfile << "namespace pcs {" << std::endl << std::endl;
	s1.clear();
	s1 = "struct ";
	outfile << s1;

	std::string sub = file.substr(0, file.find(".h"));
	std::cout << "sub = " << sub << std::endl;
	char *a = new char[sub.length() + 1];
	strcpy(a, sub.c_str());
	a[0] = a[0] - 32;
	std::string temp = a;
	outfile << temp << "{";
	outfile << std::endl;

	//////////////////// Generate Default Constructor and Deconstructor ///////////////////
	if (!strVector.empty()) {
		for (auto it = strVector.begin(); it != strVector.end(); it++) {
			outfile << "\t" << *it << "; ";
			outfile << std::endl;
		}
	}
	//strVector.clear();
	outfile << std::endl;

	if (!arrayLengthMap.empty()) {
		for (auto it = arrayLengthMap.begin(); it != arrayLengthMap.end(); it++) {
			outfile << "\t" << it->first << ";" << std::endl;
		}
	}
	outfile << std::endl;

	outfile << "\t" << temp << "( )" << std::endl << "\t{" << std::endl << std::endl << "\t}" << std::endl << std::endl;

	outfile << "\t~" << temp << "( )" << std::endl << "\t{" << std::endl << std::endl << "\t}" << std::endl << std::endl;

	//////////////////////////// Generate Copy Constructor /////////////////////////////
	outfile << "\t" << temp << "( const " << temp << " &obj ): ";

	int i = 0;
	for (auto it = namesMap.begin(); it != namesMap.end(); it++, i++) {
		if (i != namesMap.size() - 1) {
			outfile << it->second << "( obj." << it->second << " ), " << std::endl << "\t\t\t";
		}
		else {
			outfile << it->second << "( obj." << it->second << " ) ";
		}
	}
	outfile << std::endl;
	outfile << "\t{" << std::endl;

	outfile << "\t\t";
	if (!arrayNamesMap.empty()) {
		for (auto it = arrayNamesMap.begin(); it != arrayNamesMap.end(); it++) {
			auto lengthItem = arrayLengthMap.find(it->first);
			outfile << "memcpy( " << it->second << ", obj." << it->second << ", " << lengthItem->second << " );" << std::endl;
		}
	}
	outfile << std::endl;
	outfile << "\t}" << std::endl << std::endl;
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////// Generata Assign Copy Constructor ////////////////////////////
	outfile << "\t" << temp << "& operator=( const " << temp << " &other )" << std::endl;
	outfile << "\t{" << std::endl;
	outfile << "\t\tif( this == &other )" << std::endl;
	outfile << "\t\t\treturn *this;" << std::endl;
	for (auto it = namesMap.begin(); it != namesMap.end(); it++, i++) {
		outfile << "\t\t" << it->second << " = other." << it->second << ";" << std::endl;
	}
	if (!arrayNamesMap.empty()) {
		for (auto it = arrayNamesMap.begin(); it != arrayNamesMap.end(); it++) {
			auto lengthItem = arrayLengthMap.find(it->first);
			outfile << "\t\tmemcpy( " << it->second << ", other." << it->second << ", " << lengthItem->second << " );" << std::endl;
		}
	}
	outfile << std::endl;
	outfile << "\t\treturn *this;" << std::endl;
	outfile << "\t}" << std::endl << std::endl;
	///////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////// OverLoading the Constructor ///////////////////////
	outfile << "\t" << temp << "( ";
	i = 0;
	for (auto it = namesMap.begin(); it != namesMap.end(); it++, i++) {
		auto typeItem = typesMap.find( it->first );
		if (i != namesMap.size() - 1) {
			outfile << "const " << typeItem->second <<" "<< it->second << "_, ";
		}
		else {
			outfile << "const " << typeItem->second <<" "<< it->second << "_ ";
		}
	}
	outfile << " ): ";
	i = 0;
	for (auto it = namesMap.begin(); it != namesMap.end(); it++, i++) {
		if (i != namesMap.size() - 1) {
			outfile << it->second << "( " << it->second << "_ )," << std::endl << "\t\t\t";
		}
		else {
			outfile << it->second << "( " << it->second << "_ )";
		}
	}
	outfile << std::endl;
	outfile << "\t{" << std::endl << std::endl;
	outfile << "\t}" << std::endl << std::endl;

	///////////////// Function of Getting the size of the arrays ///////////////
	for (auto it = arrayNamesMap.begin(); it != arrayNamesMap.end(); it++) {
		outfile << "\tinline int " << it->second << "_size()" << std::endl;
		outfile << "\t{" << std::endl;
		auto lengthItem = arrayLengthMap.find( it->first );
		outfile << "\t\tint size = " << lengthItem->second << ";" << std::endl;
		outfile << "\t\treturn size; " << std::endl;
		outfile << "\t}" << std::endl;
	}

	/////////////////////////////// End //////////////////////////////////
	outfile << "};" << std::endl << std::endl;
	outfile << "typedef struct " << temp << " " << temp <<";"<< std::endl;

	outfile << "}" << std::endl;
	outfile << "#endif" << std::endl;

	outfile.close();
}

void getMessageLists(std::string file)
{
	std::ifstream infile;
	infile.open(file.data());
	assert(infile.is_open());

	std::cout << "message: " << std::endl;
	std::string s;
	while (std::getline(infile, s)) {
		//std::cout << s << std::endl;
		for (auto it = namesMap.begin(); it != namesMap.end(); it++) {
			size_t pose = s.find( it->first );
			
		//	std::cout << "pose == " << pose << std::endl;
			if (pose == 1) {
				std::cout<<"\t"<<it->first<<std::endl;
				//std::cout << s << std::endl;
			}
		}
		for (auto it = arrayNamesMap.begin(); it != arrayNamesMap.end(); it++) {
			size_t pose = s.find( it->first );

			if (pose == 1) {
				std::cout<<"\t"<<it->first<<std::endl;
			//	std::cout << s << std::endl;
			}
		}
	}
	infile.close();
}
