#pragma once
#include <string>
#include <cstdarg>
#include <ctime>
#include <unistd.h>
#include <sys/time.h>

class Boltek
{
	private:
		std::string m_prefix;
		std::string m_fn;

	public:
		Boltek();
		~Boltek();
};


Writer::Boltek()
{
}

Writer::~Boltek()
{
}
