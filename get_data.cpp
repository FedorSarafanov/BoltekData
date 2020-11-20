#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#include "logger.h"
#include "writer.h"
 
#include <libusb-1.0/libusb.h> 

#define USB_VENDOR_ID       0x0403      /* USB vendor ID used by the device
                                         * 0x0483 is STMs ID
                                         */
#define USB_PRODUCT_ID      0xf245      /* USB product ID used by the device */
#define USB_ENDPOINT_IN     (LIBUSB_ENDPOINT_IN  | 1)   /* endpoint address */
#define USB_ENDPOINT_OUT    (LIBUSB_ENDPOINT_OUT | 2)   /* endpoint address */
#define USB_TIMEOUT         3000        /* Connection timeout (in ms) */

static libusb_context *ctx = NULL;
static libusb_device_handle *handle;
libusb_device **devs;

#define USLEEP_PERIOD 50000
#define BUFFER_SIZE 1024

static uint8_t receive_buf[BUFFER_SIZE];
uint8_t transfer_buf[BUFFER_SIZE];
char string[BUFFER_SIZE];

bool boltek_signal_connected = true;
bool boltek_usb_connected = true;

FILE *logfile = NULL;
char logname[15] = "boltek.log";
char SID[100] = "E2150533"; 


#define helpData  "Boltek fluxmeter client v0.1\n"\
                  "F. Kuterin, F. Sarafanov (c) IAPRAS 2020\n\n"\
                  "Use \t./get_data [SID] [PREFIX] [PID],\n where [PID] can be obtained using \n"\
                  "command `lsusb  -d 0x0403: -v | grep idProduct`,\n [PREFIX] -- location string (without whitespaces or dash)\n"\
                  "[SID] can be obtained using command `lsusb  -d 0x0403: -v | grep Serial`.\n"\
                  "\nYou can see log in file 'boltek.log'" 


Logger logger("boltek.log");
Writer writer(std::string("prefix"), &logger);

/**
 * Read a packet to `string` variable
 */
static int usb_read(void)
{
    int nread, ret;
    ret = libusb_bulk_transfer(handle, USB_ENDPOINT_IN, receive_buf, sizeof(receive_buf),
                               &nread, USB_TIMEOUT);
    if (ret) {
        return -1;
    }
    else {
        memset(string, 0, sizeof(string));
        strncpy(string, (const char*)receive_buf, nread);
        return 0;
    }
}


/**
 * write a few bytes to the device
 */
static int usb_write(void)
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

void usb_control_out(uint8_t bRequest, uint16_t wValue, uint16_t wIndex) {
    int rc = libusb_control_transfer(handle,
                                     0x40, //uint8_t     bmRequestType,
                                     bRequest,
                                     wValue,
                                     wIndex,
                                     NULL, // unsigned char *    data,
                                     0, // uint16_t      wLength,
                                     USB_TIMEOUT //unsigned int      timeout
                                    );
}

void usb_control_in(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength) {
    uint8_t cr2[2];
    int rc2 = libusb_control_transfer(handle,
                                      0xC0, //uint8_t     bmRequestType,
                                      bRequest,
                                      wValue,
                                      wIndex,
                                      cr2, // unsigned char *     data,
                                      wLength, // uint16_t    wLength,
                                      USB_TIMEOUT //unsigned int      timeout
                                     );
}


static void sighandler(int signum)
{
    if (handle) {
        libusb_release_interface (handle, 0);
        libusb_close(handle);
    }

    if (boltek_usb_connected)
    {
        libusb_exit(NULL);
    }

    exit(0);
}

enum Init_res
{
    ERR_GET_DEV_LIST = -1,
    INIT_SUCCESS = 0,
    ERR_OPEN_BOLTEK = 1,
    ERR_CLAIM_BOLTEK = 2,
    ERR_NOTFOUND_BOLTEK = 3,
};


Init_res init(const unsigned PID)
{
    signal(SIGINT, sighandler);

    libusb_init(&ctx);

    int devs_count = libusb_get_device_list(ctx, &devs);
    int boltekdevs_count = 0;

    if(devs_count < 0) {
        // logger.log("Error: Can't get USB devices list");
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
                // logger.log("Error: Can't get %d-th (from %d) device descriptor", i, devs_count);
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
                        // logger.log("Error: Can't get %d-th (from %d) device SID", i, devs_count);
                    }
                    else {
                        if (USB_VENDOR_ID == desc.idVendor && PID == desc.idProduct && strcmp(SID, (const char *)dev_detail) == 0) {
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


int main(int argc, char *argv[])
{
    char strBuf[6];
    struct timeval ut_tv;
    struct tm *gtm;
    char prefix[100] = "default";

    unsigned PID = USB_PRODUCT_ID;
    if (argc>1)
    {
        if (strcmp(argv[1],"-h")==0)
        {
            printf("%s\n", helpData);
            return EXIT_SUCCESS;
        }
        sscanf(argv[1],"%s",&SID);
    }
    if (argc>2)
    {
        sscanf(argv[2],"%s",&prefix);
    }
    if (argc>3)
    {
        sscanf(argv[3],"%x",&PID);
    }

    auto initFlag = init(PID);

    gettimeofday(&ut_tv, NULL);
    const time_t sec = (time_t)ut_tv.tv_sec;
    const time_t usec = (time_t)ut_tv.tv_usec;
    gtm = gmtime(&sec);

    int j = 0;
    int readRes = 1;
    time_t lastTime = 0;


    while (1) {
        gettimeofday(&ut_tv, NULL);
        const time_t sec = (time_t)ut_tv.tv_sec;
        gtm = gmtime(&sec);

        if (initFlag == INIT_SUCCESS)
        {
            readRes = usb_read();
            if (!boltek_usb_connected)
            {
                boltek_usb_connected = true;
                logger.log(gtm, "Usb connected");
                lastTime = sec;
                writer.open();
            }
        } 
        else 
        {
            if (boltek_usb_connected)
            {
                boltek_usb_connected = false;
                logger.log(gtm, "Usb disconnected");
            }
        }


        if (readRes == 0 && initFlag == INIT_SUCCESS)
        {
            for (int i = 0; string[i] != '\0'; i++) {
                if (string[i] == '$')
                {
                    if (strlen(strBuf) > 0) {
                        gettimeofday(&ut_tv, NULL);
   
                        if (strBuf[0] == '+' || strBuf[0] == '-') {
                            writer.write(std::string(strBuf));
                            lastTime = (time_t)ut_tv.tv_sec;
                        }

                    }

                    j = 0;
                    memset(strBuf, 0, sizeof(strBuf));

                }
                else
                {
                    if (string[i] == '+' || string[i] == '-' || isdigit(string[i]))
                    {
                        if (j<=4)
                        {
                            strBuf[j] = string[i];
                            j++;
                        }
                    } else {
                        if (sec-lastTime > 1 && lastTime != 0)
                        {
                            if (boltek_signal_connected)
                            {
                                boltek_signal_connected = false;
                                struct tm *lastgtm = gmtime(&lastTime);
                                logger.log(lastgtm, "Signal cable disconnected");
                            }
                        }
                        else
                        {
                            if (!boltek_signal_connected)
                            {
                                boltek_signal_connected = true;
                                // gettimeofday(&ut_tv, NULL);
                                // const time_t sec = (time_t)ut_tv.tv_sec;
                                // gtm = gmtime(&sec);
                                // logger.log(gtm, "Signal cable connected");
                                logger.log("Signal cable connected");
                                writer.open();
                            }
                        }
                    }
                }
            }
        }
        else
        {
            libusb_exit(NULL);
            usleep(500000);
            initFlag = init(PID);

        }
        usleep(USLEEP_PERIOD);
    }

    libusb_close(handle);
    libusb_exit(NULL);

    return EXIT_SUCCESS;
}