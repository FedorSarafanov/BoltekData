#include "logger.h"

Logger::Logger(const std::string &filename)
{
	m_log_fn = filename;
	if( access(m_log_fn.c_str(), W_OK ) == -1 ) {
		m_log_file_ptr = stdout;
		log("Error: Unable access to log file '%s', redirect to stdout",m_log_fn.c_str());
	}
	else
	{
		m_log_file_ptr = fopen(m_log_fn.c_str(),"a+");
	}
}


void Logger::log(const std::string &format, ...)
{
	va_list vl;
    va_start(vl, format);

    struct timeval ut_tv;
    struct tm *gtm;		
	char outtime[200];
    gettimeofday(&ut_tv, NULL);
	const time_t sec = (time_t)ut_tv.tv_sec;
	const time_t usec = (time_t)ut_tv.tv_usec;
	gtm = gmtime(&sec);
	strftime(outtime, sizeof(outtime), "%Y-%m-%d-%H:%M:%S", gtm);

	fprintf(m_log_file_ptr, "%s\t", outtime);
    vfprintf(m_log_file_ptr, format.c_str(), vl);
	fprintf(m_log_file_ptr, "\n");

    fflush(m_log_file_ptr);
    va_end(vl);
}

void Logger::log(struct tm* gtm, const std::string &format, ...)
{
	va_list vl;
    va_start(vl, format);
    char outtime[200];

	strftime(outtime, sizeof(outtime), "%Y-%m-%d-%H:%M:%S", gtm);

	fprintf(m_log_file_ptr, "%s\t", outtime);
    vfprintf(m_log_file_ptr, format.c_str(), vl);
	fprintf(m_log_file_ptr, "\n");

    fflush(m_log_file_ptr);
    va_end(vl);
}



Logger::~Logger()
{
	if (m_log_file_ptr){
		log("Close log file");
		fclose(m_log_file_ptr);
	}
}
