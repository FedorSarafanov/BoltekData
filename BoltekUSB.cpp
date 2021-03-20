#include "BoltekUSB.hpp"

/**
 * Read a packet to `string` variable
 */
int BoltekUSB::usb_read(void)
{
    int nread, ret;
    ret = libusb_bulk_transfer(handle, USB_ENDPOINT_IN, receive_buf, sizeof(receive_buf),
                               &nread, USB_TIMEOUT);
    if (ret) {
        return -1;
    }
    else {
        memset(buf, 0, sizeof(buf));
        strncpy(buf, (const char*)receive_buf, nread);
        return 0;
    }
}


/**
 * write a few bytes to the device
 */
int BoltekUSB::usb_write(void)
{
    int n;
    int ret = libusb_bulk_transfer(handle, USB_ENDPOINT_OUT, transfer_buf, sizeof(transfer_buf), &n, USB_TIMEOUT);

    //Error handling
    switch(ret) {
        case 0:
            //send n bytes to device
            return 0;
        case LIBUSB_ERROR_TIMEOUT:
            break;
        case LIBUSB_ERROR_PIPE:
            break;
        case LIBUSB_ERROR_OVERFLOW:
            break;
        case LIBUSB_ERROR_NO_DEVICE:
            break;
        default:
            break;
    }
    return -1;

}

void BoltekUSB::usb_control_out(uint8_t bRequest, uint16_t wValue, uint16_t wIndex) {
    int rc = libusb_control_transfer(   
                                    handle,
                                    0x40, //uint8_t     bmRequestType,
                                    bRequest,
                                    wValue,
                                    wIndex,
                                    NULL, // unsigned char *    data,
                                    0, // uint16_t      wLength,
                                    USB_TIMEOUT //unsigned int      timeout
                                    );
}

void BoltekUSB::usb_control_in(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength) {
    uint8_t cr2[2];
    int rc2 = libusb_control_transfer(  
                                     handle,
                                     0xC0, //uint8_t     bmRequestType,
                                     bRequest,
                                     wValue,
                                     wIndex,
                                     cr2, // unsigned char *     data,
                                     wLength, // uint16_t    wLength,
                                     USB_TIMEOUT //unsigned int      timeout
                                     );
}



BoltekUSB::InitialisationStatus BoltekUSB::init(const unsigned PID)
{
    // signal(SIGINT, sighandler);
    libusb_init(&ctx);

    int devs_count = libusb_get_device_list(ctx, &devs);
    int boltekdevs_count = 0;

    if(devs_count < 0) {
        // m_logger->log("Error: Can't get USB devices list");
        return ERR_GET_DEV_LIST;
    } 
    else 
    {
        for(int i = 0; i < devs_count; i++) { 
            libusb_device *dev = devs[i];

            struct libusb_device_descriptor desc;
            struct libusb_config_descriptor *config;

            int r = libusb_get_device_descriptor(dev, &desc);
            if (r < 0) {
                // m_logger->log("Error: Can't get %d-th (from %d) device descriptor", i, devs_count);
                libusb_free_config_descriptor(config);
            } 
            else 
            {
                static libusb_device_handle *devHandle = NULL;
                libusb_get_config_descriptor(dev, 0, &config);
                unsigned char dev_detail[200];

                memset(dev_detail, 0, sizeof(dev_detail));


                int device_open_status = libusb_open(dev, &devHandle);

                if (device_open_status == 0) {
                    int returnStatus = libusb_get_string_descriptor_ascii(devHandle,
                               desc.iSerialNumber, dev_detail, sizeof(dev_detail));

                    if (returnStatus < 0) {
                        // m_logger->log("Error: Can't get %d-th (from %d) device SID", i, devs_count);
                    }
                    else {
                        if (USB_VENDOR_ID == desc.idVendor && PID == desc.idProduct && strcmp(SID.c_str(), (const char *)dev_detail) == 0) {
                            // m_logger->log("%d,%d", desc.idVendor,desc.idProduct);
                            boltekdevs_count++;
                            handle = devHandle;
                            break;
                        }
                        else
                        {
                            libusb_close(devHandle);
                        }
                    }
                }
                libusb_free_config_descriptor(config);
            }
        }
    }

    libusb_free_device_list(devs, 1);

    if (boltekdevs_count < 1){ 
        return ERR_NOTFOUND_BOLTEK;
    }

    int r = libusb_claim_interface(handle, 0);
    if (r < 0) {
        // libusb_close(handle);
        return ERR_CLAIM_BOLTEK;
    }

    usb_control_out(0, 0, 0);
    usb_control_in (5, 0, 0, 2);
    usb_control_in (10, 0, 0, 1);
    usb_control_out(9, 2, 0);
    usb_control_out(9, 0, 0);
    usb_control_out(0, 0, 0);
    usb_control_in (5, 0, 0, 2);
    usb_control_out(0, 0, 0);
    usb_control_out(0, 2, 0);
    usb_control_out(0, 1, 0);
    usb_control_out(0, 1, 0);
    usb_control_out(0, 1, 0);
    usb_control_out(0, 1, 0);
    usb_control_out(0, 1, 0);
    usb_control_out(0, 1, 0);
    usb_control_out(3, 0x4138, 0);
    usb_control_out(4, 8, 0);
    usb_control_out(0, 1, 0);
    usb_control_out(0, 1, 0);
    usb_control_out(0, 1, 0);
    usb_control_out(0, 1, 0);
    usb_control_out(0, 1, 0);
    usb_control_out(0, 1, 0);
    usb_control_out(0, 2, 0);

    return INIT_SUCCESS;
}

BoltekUSB::BoltekUSB(std::string SD, Logger *logger, Writer *writer, std::atomic<bool> *mquit)
{
    m_logger = logger;
    m_writer = writer;
    m_quit = mquit;
    SID = SD;
    m_init_flag = BoltekUSB::init(USB_PRODUCT_ID);
}

BoltekUSB::~BoltekUSB()
{
	if (m_init_flag == INIT_SUCCESS) { 
        libusb_release_interface (handle, 0);
        libusb_close(handle);
        if (m_boltek_usb_connected)
        {
            libusb_exit(NULL);
        }
    }
}

std::string BoltekUSB::read_data()
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
        libusb_exit(NULL);
        usleep(500000);        
        m_init_flag = BoltekUSB::init(USB_PRODUCT_ID);
    }
    if (!m_boltek_usb_connected){
        m_boltek_usb_connected = true;
        m_logger->log("Usb connected");
        m_writer->open();
    }

    int readRes = usb_read();

    std::string result("");
    if (readRes != 0)
    {
        m_init_flag = DEFAULT;
    }
    else
    {
        result = std::string(buf);
    }
    return result;
}