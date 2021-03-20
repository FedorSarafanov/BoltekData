#pragma once
#include "BoltekDevice.hpp"
#include "Logger.hpp"
#include "Writer.hpp"
#define BUFFER_SIZE 1024

class BoltekTTY : public BoltekDevice
{
	private:
		std::string m_tty_address;

		Logger *m_logger;
		Writer *m_writer;

		std::atomic<bool> *m_quit;


		char m_buf[BUFFER_SIZE];
		int  m_tty_fd = 0;
		bool m_usb_is_connected = true;
		bool m_cable_is_connected = true;


		InitialisationStatus m_init_status = DEFAULT;
		InitialisationStatus init(std::string tty_address);

	public:
		BoltekTTY(std::string tty_address, Logger *logger, Writer *writer, std::atomic<bool> *quit);
		~BoltekTTY() override;
		virtual std::string read_data() override;
};
