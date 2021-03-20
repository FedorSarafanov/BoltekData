#pragma once
#include <string>

class BoltekDevice
{
	public:
	    virtual ~BoltekDevice() = default;
	    virtual std::string read_data() = 0;

	protected:
		enum InitialisationStatus
		{
			DEFAULT = -2,
		    ERR_GET_DEV_LIST = -1,
		    INIT_SUCCESS = 0,
		    ERR_OPEN_BOLTEK = 1,
		    ERR_CLAIM_BOLTEK = 2,
		    ERR_NOTFOUND_BOLTEK = 3,
		};
		bool m_usb_is_connected = true;
		bool m_cable_is_connected = true;
};
