#pragma once
#include <string>
#include <cstring>
#include <cstdarg>
#include <ctime>
#include <unistd.h>
#include <sys/time.h>
#include <atomic>

#include <libusb-1.0/libusb.h> 
#include "logger.h"
#include "writer.h"


#define USB_VENDOR_ID       0x0403      /* USB vendor ID used by the device
                                         * 0x0483 is STMs ID
                                         */
#define USB_PRODUCT_ID      0xf245      /* USB product ID used by the device */
#define USB_ENDPOINT_IN     (LIBUSB_ENDPOINT_IN  | 1)   /* endpoint address */
#define USB_ENDPOINT_OUT    (LIBUSB_ENDPOINT_OUT | 2)   /* endpoint address */
#define USB_TIMEOUT         3000        /* Connection timeout (in ms) */
#define BUFFER_SIZE 1024

#define TERMINAL    "/dev/BOLTEK340_USBCOM"

class Boltek_tty
{
	private:
		std::string SID;
		Logger *m_logger;
		Writer *m_writer;

		libusb_context *ctx = NULL;
		libusb_device_handle *handle;
		libusb_device **devs;
		std::atomic<bool> *m_quit;

		char buf[BUFFER_SIZE];
		int tty_fd = 0;
		bool m_boltek_usb_connected = true;
		bool m_boltek_cable_connected = true;


		int usb_read(void);
		enum Init_res
		{
			DEFAULT = -2,
		    ERR_GET_DEV_LIST = -1,
		    INIT_SUCCESS = 0,
		    ERR_OPEN_BOLTEK = 1,
		    ERR_CLAIM_BOLTEK = 2,
		    ERR_NOTFOUND_BOLTEK = 3,
		};
		Init_res m_init_flag = DEFAULT;
		Init_res init(const unsigned PID);
	public:
		Boltek_tty(std::string SID, Logger *logger, Writer *writer, std::atomic<bool> *quit);
		~Boltek_tty();
		std::string read_data(void);
};