#pragma once
#include <string>
#include <cstring>
#include <cstdarg>
#include <ctime>
#include <unistd.h>
#include <sys/time.h>

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

class Boltek
{
	private:
		std::string SID;
		Logger *m_logger;
		Writer *m_writer;

		libusb_context *ctx = NULL;
		libusb_device_handle *handle;
		libusb_device **devs;

		uint8_t receive_buf[BUFFER_SIZE];
		uint8_t transfer_buf[BUFFER_SIZE];
		char buf[BUFFER_SIZE];

		bool m_boltek_usb_connected = true;


		int usb_write(void);
		void usb_control_out(uint8_t bRequest, uint16_t wValue, uint16_t wIndex);
		void usb_control_in(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength);
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
		Boltek(std::string SID, Logger *logger, Writer *writer);
		~Boltek();
		std::string read_data(void);
};