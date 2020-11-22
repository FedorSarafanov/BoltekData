// #pragma once
#include <string>
#include <cstring>
#include <cstdarg>
// #include <ctime>
#include <unistd.h>
// #include <sys/time.h>
// #include <atomic>
#include <map>
#include <vector>
#include <algorithm>



class Argc_parser
{
private:
	std::string m_exec_name;
	std::map <std::string, std::string> m_map;
	std::map <std::string, std::string> m_vec;

public:
	Argc_parser(int a, char **argv);
	bool contains(std::string key);
	~Argc_parser();
};

Argc_parser::Argc_parser(int argc, char **argv){
	m_exec_name = argv[0]; // Name of the current exec program

	for (int i = 1; i < argc; ++i)
	{
		std::string str(argv[i]);
		std::string key("");
		std::string value("");
		bool is_key = true;
		for(auto sym : str){
			if (sym == '=')
			{
				is_key = false;
				continue;
			}
			if (is_key){
				key += sym;
			}
			else{
				value += sym;
			}
		}
		while(key[0] == '-'){
			key = key.substr(1);
		}
		if (str.find('=') != std::string::npos){
			m_map[key] = value;
			printf("MAP %s\t%s\n", key.c_str(), value.c_str());
		}
		else
		{
			m_vec[key] = "";
			printf("VEC %s\t%s\n", key.c_str());
		}
	}
}

bool Argc_parser::contains(std::string key){
	if (m_map.count(key)+m_vec.count(key) >0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

Argc_parser::~Argc_parser(){
	
}


int main(int argc, char *argv[]){
	Argc_parser aparser(argc, argv);
	printf("%d\n", aparser.contains("help"));
}