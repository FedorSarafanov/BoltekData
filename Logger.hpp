#pragma once

#include <string>
#include <cstdarg>
#include <ctime>
#include <sys/time.h>
#include <unistd.h>

class Logger
{
private:
	std::string m_log_fn;
	FILE *m_log_file_ptr;

public:
	Logger(const std::string &filename);
	~Logger();
	void log(struct tm* gtm, const std::string &format, ...);
	void log(const std::string &format, ...);
};