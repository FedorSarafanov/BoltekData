#pragma once
#include <string>
#include <cstring>
#include <cstdarg>
#include <unistd.h>
#include <map>
#include <algorithm>
#include <fstream>
#include <streambuf>



class Config
{
private:
	std::string m_exec_name;
	std::map <std::string, std::string> m_map;
	std::map <std::string, std::string> m_vec;
	bool is_number(const std::string& s)
	{
		int j = static_cast<int>(s[0] == '+' || s[0] == '-');
	    return !s.empty() && std::find_if(s.begin()+j, 
	        s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
	}
	enum argtype
	{
		ARG_STRING,
		ARG_INT,
		ARG_FLOAT,
		ARG_ANY
	};
	bool contains(std::string key, argtype atype);
	void parse_cargs(int a, char **argv);
	void parse_ini(std::string);
public:
	Config(int a, char **argv, std::string);

	std::string get_value(std::string key, std::string bydef);

	int get_value(std::string key, int bydef);
	int get_value(std::string key, int, int, int);

	float get_value(std::string key, float bydef);
	float get_value(std::string key, float, float, float);

	double get_value(std::string key, double bydef);
	double get_value(std::string key, double, double, double);


	bool get_value(std::string key);

	~Config();
};
