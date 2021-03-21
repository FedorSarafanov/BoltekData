#include "BoltekTTY.hpp"


BoltekTTY::InitialisationStatus BoltekTTY::init(const std::string &tty_address)
{
    m_tty_fd = open(tty_address.c_str(), O_RDWR);

    struct termios tty;

    if(tcgetattr(m_tty_fd, &tty) != 0) {
        // printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
        close(m_tty_fd);
        return ERR_NOTFOUND_BOLTEK;
    }

    tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
    tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
    tty.c_cflag &= ~CSIZE; // Clear all bits that set the data size 
    tty.c_cflag |= CS8; // 8 bits per byte (most common)
    tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)
    tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

    tty.c_lflag &= ~ICANON; // Non-canon mode (not line by line)
    tty.c_lflag &= ~ECHO; // Disable echo
    tty.c_lflag &= ~ECHOE; // Disable erasure
    tty.c_lflag &= ~ECHONL; // Disable new-line echo
    tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
    tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes

    tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
    tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed

    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 5;    // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.

    // Set in/out baud rate to be 9600
    cfsetispeed(&tty, B9600);
    cfsetospeed(&tty, B9600);

    // Save tty settings, also checking for error
    if (tcsetattr(m_tty_fd, TCSANOW, &tty) != 0) {
        // printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
        close(m_tty_fd);
        return ERR_OPEN_BOLTEK;
    }

    return INIT_SUCCESS;
}

BoltekTTY::BoltekTTY(const std::string &tty_address, Logger *logger, Writer *writer, std::atomic<bool> *mquit)
{
    m_logger = logger;
    m_writer = writer;
    m_quit = mquit;
    m_tty_address = tty_address;
    m_init_status = BoltekTTY::init(tty_address);
}

BoltekTTY::~BoltekTTY()
{
	if (m_init_status == INIT_SUCCESS) { 
        close(m_tty_fd);    
    }
}

std::string BoltekTTY::read_data()
{
    std::string result("");
    m_usb_is_connected = true;

    while (m_init_status != INIT_SUCCESS)
    {
        if( m_quit->load() )
        {
            return result;
        }

        if (m_usb_is_connected)
        {
            m_logger->log("Usb disconnected");
            m_writer->flush();
            m_usb_is_connected = false;
        }

        usleep(500000);
        m_init_status = BoltekTTY::init(m_tty_address);
    }
    if (!m_usb_is_connected)
    {
        m_usb_is_connected = true;
        m_logger->log("Usb connected");
        
        if (m_cable_is_connected)
        {
            m_writer->open();
        }
    }

    memset(m_buf, 0, sizeof(m_buf));
    int rdlen = read(m_tty_fd, m_buf, sizeof(m_buf)-1);
   
    if (isatty(m_tty_fd) == 0)
    {
        m_init_status = DEFAULT;
        return result;
    }

    if (rdlen > 0)
    {
        result = std::string(m_buf);
        
        if (!m_cable_is_connected)
        {
            m_cable_is_connected = true;
            m_logger->log("Signal cable connected");
            
            if (m_usb_is_connected)
            {
                m_writer->open();
            }
        }
    }
    else
    {
        if (m_cable_is_connected)
        {
            m_cable_is_connected = false;
            
            if( !m_quit->load() ) 
            {
                m_logger->log("Signal cable disconnected");
            }

            m_writer->flush();
        }
    }
    return result;
}