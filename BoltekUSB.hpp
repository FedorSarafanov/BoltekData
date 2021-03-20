#pragma once
#include <string>
#include <cstring>
#include <cstdarg>
#include <ctime>
#include <unistd.h>
#include <sys/time.h>
#include <atomic>

#include <libusb-1.0/libusb.h> 
#include "Logger.hpp"
#include "Writer.hpp"

#include "BoltekDevice.hpp"

#define USB_VENDOR_ID       0x0403      /* USB vendor ID used by the device
                                         * 0x0483 is STMs ID
                                         */
#define USB_PRODUCT_ID      0xf245      /* USB product ID used by the device */
#define USB_ENDPOINT_IN     (LIBUSB_ENDPOINT_IN  | 1)   /* endpoint address */
#define USB_ENDPOINT_OUT    (LIBUSB_ENDPOINT_OUT | 2)   /* endpoint address */
#define USB_TIMEOUT         3000        /* Connection timeout (in ms) */
#define BUFFER_SIZE 1024


class BoltekUSB : public BoltekDevice
{
	private:
		std::string SID;
		Logger *m_logger;
		Writer *m_writer;

		libusb_context *ctx = NULL;
		libusb_device_handle *handle;
		libusb_device **devs;
		std::atomic<bool> *m_quit;

		uint8_t receive_buf[BUFFER_SIZE];
		uint8_t transfer_buf[BUFFER_SIZE];
		char m_buf[BUFFER_SIZE];

		int usb_write(void);
		void usb_control_out(uint8_t bRequest, uint16_t wValue, uint16_t wIndex);
		void usb_control_in(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength);
		int usb_read(void);
		InitialisationStatus m_init_flag = DEFAULT;
		InitialisationStatus init(const unsigned PID);		
	public:
		BoltekUSB(std::string SID, Logger *logger, Writer *writer, std::atomic<bool> *quit);
		~BoltekUSB() override;
		virtual std::string read_data() override;
};