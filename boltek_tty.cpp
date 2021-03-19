#include "boltek_tty.h"
#include <termios.h>
#include <fcntl.h>    /* For O_RDWR */
#include <unistd.h>   /* For open(), create() */


Boltek_tty::Init_res Boltek_tty::init(std::string tty_address)
{
    tty_fd = open(tty_address.c_str(), O_RDWR);

    struct termios tty;

    if(tcgetattr(tty_fd, &tty) != 0) {
        // printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
        close(tty_fd);
        return ERR_NOTFOUND_BOLTEK;
    }

    tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
    tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
    tty.c_cflag &= ~CSIZE; // Clear all bits that set the data size 
    tty.c_cflag |= CS8; // 8 bits per byte (most common)
    tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)
    tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

    // tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~ICANON; //NB!!!
    tty.c_lflag &= ~ECHO; // Disable echo
    tty.c_lflag &= ~ECHOE; // Disable erasure
    tty.c_lflag &= ~ECHONL; // Disable new-line echo
    tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
    tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes

    tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
    tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
    // tty.c_oflag &= ~OXTABS; // Prevent conversion of tabs to spaces (NOT PRESENT ON LINUX)
    // tty.c_oflag &= ~ONOEOT; // Prevent removal of C-d chars (0x004) in output (NOT PRESENT ON LINUX)

    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 5;    // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.

    // Set in/out baud rate to be 9600
    cfsetispeed(&tty, B9600);
    cfsetospeed(&tty, B9600);

    // Save tty settings, also checking for error
    if (tcsetattr(tty_fd, TCSANOW, &tty) != 0) {
        // printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
        close(tty_fd);
        return ERR_OPEN_BOLTEK;
    }

    return INIT_SUCCESS;
}

Boltek_tty::Boltek_tty(std::string tty_address, Logger *logger, Writer *writer, std::atomic<bool> *mquit)
{
    m_logger = logger;
    m_writer = writer;
    m_quit = mquit;
    m_tty_address = tty_address;
    // m_logger->log("%s",tty_address.c_str());
    m_init_flag = Boltek_tty::init(tty_address);
}

Boltek_tty::~Boltek_tty()
{
	if (m_init_flag == INIT_SUCCESS) { 
        close(tty_fd);    
    }
}

std::string Boltek_tty::read_data()
{
    m_boltek_usb_connected = true;
    while (m_init_flag != INIT_SUCCESS)
    {
        if( m_quit->load() ) {
            return std::string("");
        }
        if (m_boltek_usb_connected)
        {
            m_logger->log("Usb disconnected");
            m_writer->flush();
            m_boltek_usb_connected = false;
        }
        usleep(500000);
        m_init_flag = Boltek_tty::init(m_tty_address);
    }
    if (!m_boltek_usb_connected){
        m_boltek_usb_connected = true;
        m_logger->log("Usb connected");
        if (m_boltek_cable_connected){
            m_writer->open();
        }
    }

    memset(&buf[0], 0, sizeof(buf));
    int rdlen = read(tty_fd, buf, sizeof(buf)-1);

    std::string result("");
    if (isatty(tty_fd) == 0)
    {
        m_init_flag = DEFAULT;
        return result;
    }
    if (rdlen > 0)
    {
        result = std::string(buf);
        if (!m_boltek_cable_connected){
            m_boltek_cable_connected = true;
            m_logger->log("Signal cable connected");
            if (m_boltek_usb_connected){
                m_writer->open();
            }
        }
    }
    else
    {
        if (m_boltek_cable_connected){
            m_boltek_cable_connected = false;
            if( !m_quit->load() ) {
                m_logger->log("Signal cable disconnected");
            }
            m_writer->flush();
        }
    }
    return result;
}