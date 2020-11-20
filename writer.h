#pragma once
#include <string>
#include <cstdarg>
#include <ctime>
#include <unistd.h>
#include <sys/time.h>
#include "logger.h"

class Writer
{
	private:
		std::string m_prefix;
		std::string m_fn;

		FILE *m_file_ptr;

		int m_hour_end_count;
		int m_error_count;

		Logger *m_logger;

	public:
		Writer(const std::string &prefix, Logger *logger);
		~Writer();
		void open(struct tm *gtm);
		void open();
		void write(const std::string &value);
};