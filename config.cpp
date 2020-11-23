#include "config.h"

void Config::parse_cargs(int argc, char **argv)
{
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
		}
		else
		{
			m_vec[key] = "";
		}
	}
}

void Config::parse_ini(std::string ini_fn)
{
	std::ifstream ini_file(ini_fn);
	std::string line;
	while (std::getline(ini_file, line))
	{
		if (line.find('=') != -1)
		{
			std::string key = line.substr(0, line.find('='));
			while(key[0] == ' '){
				key = key.substr(1);
			}
			while(key[key.size()-1] == ' '){
				key = key.substr(0, key.size()-1);
			}
			std::string value = line.substr(line.find('=')+1);
			while(value[0] == ' '){
				value = value.substr(1);
			}
			while(value[value.size()-1] == ' '){
				value = value.substr(0, value.size()-1);
			}
			m_map[key] = value;
		}
	}
}


Config::Config(int argc, char **argv, std::string ini_fn)
{
	parse_cargs(argc, argv);
	std::string fn = get_value("ini",ini_fn);
	parse_ini(fn);
	parse_cargs(argc, argv);
	if (contains("ignore-ini", ARG_STRING))
	{
		m_map.clear();
		m_vec.clear();
		parse_cargs(argc, argv);
	}
}

bool Config::contains(std::string key, argtype atype){
	switch (atype){
		case ARG_STRING:
	    case ARG_ANY:
	        return m_map.count(key)+m_vec.count(key) > 0;
	    case ARG_INT:
	    	if (m_map.count(key) > 0)
	    	{
	    		if (is_number(m_map[key]))
	    		{
	    			return true;
	    		}
	    	}
	    	return false;
	    case ARG_FLOAT:
	    {
	    	std::string s = m_map[key];
	     	size_t count = std::count_if( s.begin(), s.end(), []( char c ){if (c == '.') return true; else return false; });
	     	int j = static_cast<int>(s[0] == '+' || s[0] == '-');
	     	bool check = std::find_if(s.begin()+j, s.end(), [](unsigned char c) { return !std::isdigit(c) && c != '.'; }) == s.end();
	    	return !s.empty() && check && count <=1 && count >=0;
	    }
	    default:
	    	return	false;
	}
}

bool Config::get_value(std::string key)
{
	return contains(key, ARG_ANY);
}

std::string Config::get_value(std::string key, std::string bydef)
{
	if (contains(key, ARG_STRING))
	{
		if (m_map[key] != ""){
			return m_map[key];
		}
	}
	return bydef;
}

int Config::get_value(std::string key, int bydef)
{
	if (contains(key, ARG_INT))
	{
		return std::stoi(m_map[key]);
	}
	return bydef;
}


int Config::get_value(std::string key, int bydef, int min, int max)
{
	if (contains(key, ARG_INT))
	{
		int res = std::stoi(m_map[key]);
		if (res >= min && res <= max){
			return res;
		}
	}
	return bydef;
}

float Config::get_value(std::string key, float bydef)
{
	float f = bydef;
	if (contains(key, ARG_FLOAT))
	{
		f = std::stof(m_map[key]);
	}
	return f;
}

double Config::get_value(std::string key, double bydef)
{
	auto f = bydef;
	if (contains(key, ARG_FLOAT))
	{
		f = std::stof(m_map[key]);
	}
	return f;
}

double Config::get_value(std::string key, double bydef, double min, double max)
{
	auto res = get_value(key, bydef);
	if ( res >= min && res <= max)
	{
		return res;
	}
	return bydef;
}

float Config::get_value(std::string key, float bydef, float min, float max)
{
	auto res = get_value(key, bydef);
	if ( res >= min && res <= max)
	{
		return res;
	}
	return bydef;
}


Config::~Config(){
	
}
