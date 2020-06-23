#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

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

static uint8_t receiveBuf[BUFFER_SIZE];
uint8_t transferBuf[BUFFER_SIZE];
char string[BUFFER_SIZE];
int lineAlarm = 0;
int usbAlarm = 0;

FILE *logfile = NULL;
char logname[15] = "boltek.log";
char SID[100] = "E2150531";


#define helpData  "Boltek fluxmeter client v0.1\n"\
                  "F.Kuterin, F.Sarafanov (c) IAPRAS 2020\n\n"\
                  "Use \t./get_data [SID] [PREFIX] [PID],\n where [PID] can be obtained using \n"\
                  "command `lsusb  -d 0x0403: -v | grep idProduct`,\n [PREFIX] -- location string (without whitespaces or dash)\n"\
                  "[SID] can be obtained using command `lsusb  -d 0x0403: -v | grep Serial`.\n"\
                  "\nYou can see log in file 'boltek.log'" 


/**
 * Read a packet to `string` variable
 */
static int usb_read(void)
{
    int nread, ret;
    ret = libusb_bulk_transfer(handle, USB_ENDPOINT_IN, receiveBuf, sizeof(receiveBuf),
                               &nread, USB_TIMEOUT);
    if (ret) {
        return -1;
    }
    else {
        memset(string, 0, sizeof(string));
        strncpy(string, (const char*)receiveBuf, nread);
        return 0;
    }
}

uint16_t count=0;

/**
 * write a few bytes to the device
 */
static int usb_write(void)
{
    int n;
    int ret = libusb_bulk_transfer(handle, USB_ENDPOINT_OUT, transferBuf, n, &n, USB_TIMEOUT);

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


/**
 * on SIGINT: fclose USB interface
 * This still leads to a segfault on my system...
 */
static void sighandler(int signum)
{
    struct timeval ut_tv;
    char outtime[25];
    struct tm *gtm;

    FILE *outfile;

    gettimeofday(&ut_tv, NULL);
    const time_t sec = (time_t)ut_tv.tv_sec;
    const time_t usec = (time_t)ut_tv.tv_usec;
    gtm = gmtime(&sec);
    strftime(outtime, sizeof(outtime), "%Y-%m-%d-%H:%M:%S", gtm);
    logfile = fopen(logname,"a+");
    fprintf(logfile, "%s exit\n",outtime);
    fclose(logfile);


    if (handle) {
        libusb_release_interface (handle, 0);
        libusb_close(handle);
    }

    if (!usbAlarm)
    {
        libusb_exit(NULL);
    }

    exit(0);
}


void printdev(libusb_device *dev) {

}

int init(const unsigned PID)
{
    struct timeval ut_tv;
    char outtime[25];
    struct tm *gtm;


    //Pass Interrupt Signal to our handler
    signal(SIGINT, sighandler);

    libusb_init(&ctx);
    // libusb_set_debug(ctx, 3);

    int cnt = libusb_get_device_list(ctx, &devs);
    int recnt = 0;
    if(cnt < 0) {
        gettimeofday(&ut_tv, NULL);
        const time_t sec = (time_t)ut_tv.tv_sec;
        gtm = gmtime(&sec);
        strftime(outtime, sizeof(outtime), "%Y-%m-%d-%H:%M:%S", gtm);
        logfile = fopen(logname,"a+");
        fprintf(logfile, "%s Error in libusb_get_device_list\n",outtime);
        fclose(logfile);
        printf("ERR1");
        return -1;
    } else {
        // printf("найдено устройств: %d\n", cnt);
        for(int i = 0; i < cnt; i++) {  //цикл перебора всех устройств
            // printdev(devs[i]);
            libusb_device *dev = devs[i];

            struct libusb_device_descriptor desc; //дескрипторустройства
            struct libusb_config_descriptor *config;//дескрипторконфигурацииобъекта
            const struct libusb_interface *inter;
            const struct libusb_interface_descriptor *interdesc;
            const struct libusb_endpoint_descriptor *epdesc;

            int r = libusb_get_device_descriptor(dev, &desc);

            if (r < 0) {
                // printf("ERR BY GET\n");
                libusb_free_config_descriptor(config);
            } else {
                static libusb_device_handle *devHandle = NULL;
                libusb_get_config_descriptor(dev, 0, &config);
                unsigned char dev_detail[200];
                int returnStatus = 0;
                memset(dev_detail, 0, sizeof(dev_detail));


                int device_open_status = libusb_open(dev, &devHandle);
                // printf("OPEN STATUS %d;\n",device_open_status);
                // printf("okay1\n");
                if (device_open_status == 0) {

                    returnStatus = libusb_get_string_descriptor_ascii(devHandle,
                                   desc.iSerialNumber, dev_detail, sizeof(dev_detail));
                    // printf("okay2\n");
                    if (returnStatus < 0) {
                        // printf("Error serial Number\n");
                    }
                    else {
                        // printf("Serial Number: %s\n",dev_detail);
                        // printf("Vendor:Device = %04x:%04x\n", desc.idVendor, desc.idProduct);
                        if (USB_VENDOR_ID == desc.idVendor && PID == desc.idProduct && strcmp(SID,dev_detail) == 0) {
                            // printf("OK\n");
                            recnt++;
                            handle = devHandle;
                            break;
                        }else{
                            // printf("CLOSE3\n");
                            libusb_close(devHandle);
                        }
                    }
                } else
                {
                    // libusb_close(devHandle);
                }
                // libusb_close(devHandle);
                libusb_free_config_descriptor(config);
            }
        }
    }
    libusb_free_device_list(devs, 1);


    // printf("MY DEVICES: %d\n", recnt);
    if (recnt<1){
        return 3;
    }


    //Open Device with VendorID and ProductID
    // handle = libusb_open_device_with_vid_pid(ctx,
    //          USB_VENDOR_ID, PID);
    if (!handle) {
        // perror("device not found");
        return 1;
    }

    int r = 1;
    //Claim Interface 0 from the device
    // printf("CLAIM\n");
    r = libusb_claim_interface(handle, 0);
    if (r < 0) {
        // usb_claim_interface error
        return 2;
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

    return EXIT_SUCCESS;
}


int main(int argc, char *argv[])
{
    char strBuf[6];
    struct timeval ut_tv;
    char outtime[25];
    struct tm *gtm;
    FILE *outfile = NULL;
    char fn_woprefix[100];
    char filename[100];
    char lastfn[100] = "";
    char prefix[100] = "default";

    unsigned PID = 0xf245;
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

    int initFlag = init(PID);


    gettimeofday(&ut_tv, NULL);
    const time_t sec = (time_t)ut_tv.tv_sec;
    const time_t usec = (time_t)ut_tv.tv_usec;
    gtm = gmtime(&sec);


    strftime(fn_woprefix, sizeof(fn_woprefix), "%Y-%m-%d-%H:%M:%S.txt", gtm);
    sprintf(filename, "%s-%s",prefix,fn_woprefix);
    strftime(outtime, sizeof(outtime), "%Y-%m-%d-%H:%M:%S", gtm);


    logfile = fopen(logname,"a+");
    fprintf(logfile, "%s run\n",outtime);
    fclose(logfile);


    int j = 0;
    int rushhour = 0;
    int readRes = 1;
    time_t lastTime = 0;


    while (1) {
        gettimeofday(&ut_tv, NULL);
        const time_t sec = (time_t)ut_tv.tv_sec;
        gtm = gmtime(&sec);

        if (initFlag == 0)
        {
            readRes = usb_read();
            if (usbAlarm)
            {
                usbAlarm = !usbAlarm;

                strftime(outtime, sizeof(outtime), "%Y-%m-%d-%H:%M:%S", gtm);
                logfile = fopen(logname,"a+");
                fprintf(logfile, "%s usb connected\n",outtime);
                fclose(logfile);

                lastTime = sec;
                strftime(fn_woprefix, sizeof(fn_woprefix), "%Y-%m-%d-%H:%M:%S.txt", gtm);
                sprintf(filename, "%s-%s",prefix,fn_woprefix);
            }
        } else {
            if (!usbAlarm)
            {
                usbAlarm = !usbAlarm;

                strftime(outtime, sizeof(outtime), "%Y-%m-%d-%H:%M:%S", gtm);
                logfile = fopen(logname,"a+");
                fprintf(logfile, "%s usb disconnected\n",outtime);
                fclose(logfile);

            }
        }
        if (readRes == 0 && initFlag == 0)
        {
            for (int i = 0; string[i] != '\0'; i++) {
                if (string[i] == '$')
                {
                    if (strlen(strBuf) > 0) {

                        gettimeofday(&ut_tv, NULL);
                        const time_t sec = (time_t)ut_tv.tv_sec;
                        const suseconds_t usec = (time_t)ut_tv.tv_usec;


                        gtm = gmtime(&sec);
                        // if (gtm->tm_sec==0) // End of min (debug)
                        if (gtm->tm_min==0 && gtm->tm_sec==0) // End of hour
                        {
                            if (rushhour == 0)
                            {
                                strftime(fn_woprefix, sizeof(fn_woprefix), "%Y-%m-%d-%H:%M:%S.txt", gtm);
                                sprintf(filename, "%s-%s",prefix,fn_woprefix);
                                strftime(outtime, sizeof(outtime), "%Y-%m-%d-%H:%M:%S", gtm);
                                rushhour = 1;
                            }
                        }
                        else
                        {
                            rushhour = 0;
                        }

                        if (strBuf[0] == '+' || strBuf[0] == '-') {
                            strftime(outtime, sizeof(outtime), "%Y-%m-%d-%H:%M:%S", gtm);
                            lastTime = sec;
                            if (strcmp(lastfn,filename) != 0)
                            {
                                strcpy(lastfn,filename);
                                logfile = fopen(logname,"a+");
                                fprintf(logfile, "%s start new file %s\n",outtime,filename);
                                fclose(logfile);

                                if (outfile != NULL)
                                {
                                    fclose(outfile);
                                }
                                outfile = fopen(filename, "a+");
                            }
                            fprintf(outfile, "%s.%06ld\t%s\n", outtime, (long)usec, strBuf);
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
                            if (!lineAlarm)
                            {
                                lineAlarm = 1;
                                struct tm *lastgtm = gmtime(&lastTime);
                                strftime(outtime, sizeof(outtime), "%Y-%m-%d-%H:%M:%S", lastgtm);
                                logfile = fopen(logname,"a+");
                                fprintf(logfile, "%s signal cable disconnected\n",outtime);
                                fclose(logfile);

                            }
                        }
                        else
                        {
                            if (lineAlarm)
                            {
                                lineAlarm = 0;
                                gettimeofday(&ut_tv, NULL);
                                const time_t sec = (time_t)ut_tv.tv_sec;
                                gtm = gmtime(&sec);
                                strftime(outtime, sizeof(outtime), "%Y-%m-%d-%H:%M:%S", gtm);
                                logfile = fopen(logname,"a+");
                                fprintf(logfile, "%s signal cable connected\n",outtime);
                                fclose(logfile);

                                strftime(fn_woprefix, sizeof(fn_woprefix), "%Y-%m-%d-%H:%M:%S.txt", gtm);
                                sprintf(filename, "%s-%s",prefix,fn_woprefix);
                            }
                        }
                    }
                }

            }
        }
        else
        {
            // printf("CLOSE1\n");
            // libusb_close(handle);
            libusb_exit(NULL);
            usleep(500000);
            initFlag = init(PID);

            gettimeofday(&ut_tv, NULL);
            const time_t sec = (time_t)ut_tv.tv_sec;
            gtm = gmtime(&sec);
            strftime(fn_woprefix, sizeof(fn_woprefix), "%Y-%m-%d-%H:%M:%S.txt", gtm);
            sprintf(filename, "%s-%s",prefix,fn_woprefix);
        }
        usleep(USLEEP_PERIOD);
    }
    fclose(outfile);

    printf("CLOSE2\n");
    libusb_close(handle);
    libusb_exit(NULL);

    return EXIT_SUCCESS;
}

