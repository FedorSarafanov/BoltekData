#include "writer.h"  


Writer::Writer(const std::string &prefix, Logger *logger)
{
	m_prefix = prefix;
	m_hour_end_count = 0;
	m_fn = "";
	m_logger = logger;
	m_error_count = 0;
}

Writer::~Writer()
{
	if (m_file_ptr){
		fclose(m_file_ptr);
	}
}

void Writer::open(struct tm *gtm)
{
	char fn_woprefix[200];
	char filename[200];

    strftime(fn_woprefix, sizeof(fn_woprefix), "%Y-%m-%d-%H:%M:%S.txt", gtm);
    sprintf(filename, "data/%s-%s", m_prefix.c_str(), fn_woprefix);

    if (static_cast<std::string>(filename) != m_fn)
    {
		if (m_file_ptr){
			fclose(m_file_ptr);
		}    	
	    m_fn = filename;
		m_file_ptr = fopen(m_fn.c_str(),"a+");
	    if (m_file_ptr)
	    {
		    m_logger->log("Open new file %s",m_fn.c_str());
	    	m_error_count = 0;
	    }		
    }
    if (!m_file_ptr){
    	m_error_count++;
    	if (m_error_count == 1)
    	{
	    	m_logger->log("Error: Unable to open/create file %s",m_fn.c_str());
    	}
    }
}

void Writer::open()
{
	struct timeval ut_tv;
	time_t sec;
	struct tm *gtm;

	gettimeofday(&ut_tv, NULL);
	sec = (time_t)ut_tv.tv_sec;
	gtm = gmtime(&sec);

	Writer::open(gtm);
}

void Writer::write(const std::string &value) 
{
	struct timeval ut_tv;
	time_t sec;
	suseconds_t usec;
	struct tm *gtm;

	char outtime[200];

	gettimeofday(&ut_tv, NULL);
	sec = (time_t)ut_tv.tv_sec;
	usec = (suseconds_t)ut_tv.tv_usec;
	gtm = gmtime(&sec);

    if (gtm->tm_min == 0 && gtm->tm_sec == 0) // End of hour
    {
        Writer::open(gtm);
    }
	if (!m_file_ptr){
		Writer::open(gtm);
	}
	else
	{
	    strftime(outtime, sizeof(outtime), "%Y-%m-%d-%H:%M:%S", gtm);

		if( access(m_fn.c_str(), F_OK ) == -1 ) {
			m_logger->log("Error: Unable to write to file %s",m_fn.c_str());
		    Writer::open(gtm);
		}

		fprintf(m_file_ptr, "%s.%06ld\t%s\n", outtime, (long)usec, value.c_str());
	}
}