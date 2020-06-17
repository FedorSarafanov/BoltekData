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

#define USLEEP_PERIOD 50000
#define BUFFER_SIZE 1024

static uint8_t receiveBuf[BUFFER_SIZE];
uint8_t transferBuf[BUFFER_SIZE];
char string[BUFFER_SIZE];
int lineAlarm = 0;
int usbAlarm = 0;

FILE *logfile = NULL;
char logname[15] = "boltek.log";


#define helpData  "Boltek fluxmeter client v0.1\n"\
                  "F.Kuterin, F.Sarafanov (c) 2020\n\n"\
                  "Use \t./get_data [PID],\t where [PID] can be obtained using \n"\
                  "command `lsusb  -d 0x0403: -v | grep idProduct`\n"\
                  "\nSee log in file 'boltek.log'"


/**
 * Read a packet to `string` variable
 */
static int usb_read(void)
{
    int nread, ret;
    ret = libusb_bulk_transfer(handle, USB_ENDPOINT_IN, receiveBuf, sizeof(receiveBuf),
                               &nread, USB_TIMEOUT);
    if (ret) {
        // logfile = fopen(logname,"a+");
        // fprintf(logfile, "ERROR in bulk read: %d\n", ret);
        // fclose(logfile);

        return -1;
    }
    else {
        memset(string, 0, sizeof(string));
        strncpy(string, (const char*)receiveBuf, nread);
        // logfile = fopen(logname,"a+");
        // fprintf(logfile, "%s\n", string);
        // fclose(logfile);

        return 0;
    }
}

uint16_t count=0;

/**
 * write a few bytes to the device
 */
static int usb_write(void)
{
    int n, ret;
    //count up
    // n = sprintf(transferBuf, "%d\0",count++);
    //write transfer
    //probably unsafe to use n twice...

    //ret = libusb_bulk_transfer(handle, USB_ENDPOINT_OUT, transferBuf, n,
    //      &n, USB_TIMEOUT);

    n = 0;
    ret = libusb_bulk_transfer(handle, USB_ENDPOINT_OUT, transferBuf, n, &n, USB_TIMEOUT);

    //Error handling
    switch(ret) {
    case 0:
        // logfile = fopen(logname,"a+");
        // fprintf(logfile, "send %d bytes to device\n", n);
        // fclose(logfile);

        return 0;
    case LIBUSB_ERROR_TIMEOUT:
        // logfile = fopen(logname,"a+");
        // fprintf(logfile, "ERROR in bulk write: %d Timeout\n", ret);
        // fclose(logfile);

        break;
    case LIBUSB_ERROR_PIPE:
        // logfile = fopen(logname,"a+");
        // fprintf(logfile, "ERROR in bulk write: %d Pipe\n", ret);
        // fclose(logfile);

        break;
    case LIBUSB_ERROR_OVERFLOW:
        // logfile = fopen(logname,"a+");
        // fprintf(logfile, "ERROR in bulk write: %d Overflow\n", ret);
        // fclose(logfile);

        break;
    case LIBUSB_ERROR_NO_DEVICE:
        // logfile = fopen(logname,"a+");
        // fprintf(logfile, "ERROR in bulk write: %d No Device\n", ret);
        // fclose(logfile);

        break;
    default:
        // logfile = fopen(logname,"a+");
        // fprintf(logfile, "ERROR in bulk write: %d\n", ret);
        // fclose(logfile);

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

    // logfile = fopen(logname,"a+");
    // fprintf(logfile, "control result %d\n", rc);
    // fclose(logfile);

}

void usb_control_in(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength) {
    if (wLength > 2) {
        // logfile = fopen(logname,"a+");
        // fprintf(logfile, "usb_control_in length arg must be 0,1,2, no more, not %u\n", (unsigned)wLength);
        // fclose(logfile);

    }
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
    switch (wLength) {
    case 0:
        // logfile = fopen(logname,"a+");
        // fprintf(logfile, "control result %d\n", rc2);
        // fclose(logfile);

        break;
    case 1:
        // logfile = fopen(logname,"a+");
        // fprintf(logfile, "control result %d: %02X\n", rc2, (unsigned)cr2[0]);
        // fclose(logfile);

        break;
    case 2:
        // logfile = fopen(logname,"a+");
        // fprintf(logfile, "control result %d: %02X %02X\n", rc2, (unsigned)cr2[0], (unsigned)cr2[1]);
        // fclose(logfile);

        break;
    }
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

    // logfile = fopen(logname,"a+");
    // fprintf(logfile,  "\nInterrupt signal received\n" );
    // fclose(logfile);

    if (handle) {
        libusb_release_interface (handle, 0);
        // logfile = fopen(logname,"a+");
        // fprintf(logfile,  "\nInterrupt signal received1\n" );
        // fclose(logfile);

        libusb_close(handle);
        // logfile = fopen(logname,"a+");
        // fprintf(logfile,  "\nInterrupt signal received2\n" );
        // fclose(logfile);

    }
    // logfile = fopen(logname,"a+");
    // fprintf(logfile,  "\nInterrupt signal received3\n" );
    // fclose(logfile);

    if (!usbAlarm)
    {
        libusb_exit(NULL);
    }
    // logfile = fopen(logname,"a+");
    // fprintf(logfile,  "\nInterrupt signal received4\n" );
    // fclose(logfile);


    exit(0);
}

// unused function
/*
char *del_char(const char * src, char * res, char c)
{
    char *tmp = res;
    do {
        if (*src != c) {
            *res++ = *src;
        }
    } while (*src++);
    return tmp;
}
*/

int init(const unsigned PID)
{
    //Pass Interrupt Signal to our handler
    signal(SIGINT, sighandler);

    libusb_init(&ctx);
    // libusb_set_debug(ctx, 3);

    //Open Device with VendorID and ProductID
    handle = libusb_open_device_with_vid_pid(ctx,
             USB_VENDOR_ID, PID);
    if (!handle) {
        // perror("device not found");
        return 1;
    }

    int r = 1;
    //Claim Interface 0 from the device
    r = libusb_claim_interface(handle, 0);
    if (r < 0) {
        // fprintf(stderr, "usb_claim_interface error %d\n", r);
        return 2;
    }
    // logfile = fopen(logname,"a+");
    // fprintf(logfile, "Interface claimed\n");
    // fclose(logfile);


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
    printf("%s\n", helpData);
    unsigned PID = 0xf245;
    if (argc>1)
    {
        sscanf(argv[1],"%x",&PID);
    }

    int initFlag = init(PID);
    // logfile = fopen(logname,"a+");
    // fprintf(logfile, "%d\n", initFlag);
    // fclose(logfile);


    char strBuf[6];
    // char datetime[50];
    struct timeval ut_tv;
    char outtime[25];
    struct tm *gtm;


    FILE *outfile = NULL;

    gettimeofday(&ut_tv, NULL);
    const time_t sec = (time_t)ut_tv.tv_sec;
    const time_t usec = (time_t)ut_tv.tv_usec;
    gtm = gmtime(&sec);

    char filename[50];
    char lastfn[50] = "";

    strftime(filename, sizeof(filename), "%Y-%m-%d-%H:%M:%S.txt", gtm);
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
                strftime(filename, sizeof(filename), "%Y-%m-%d-%H:%M:%S.txt", gtm);
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
                        // if (gtm->tm_min==0 && gtm->tm_sec==0) // Истек час
                        if (gtm->tm_sec==0)
                        {
                            if (rushhour == 0)
                            {
                                // strcpy(lastfn, filename);
                                strftime(filename, sizeof(filename), "%Y-%m-%d-%H:%M:%S.txt", gtm);
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

                    // Очистка буфера
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
                        // Не $, не +-, не цифра
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

                                strftime(filename, sizeof(filename), "%Y-%m-%d-%H:%M:%S.txt", gtm);
                            }
                        }
                    }
                }

            }
        }
        else
        {
            libusb_close(handle);
            libusb_exit(NULL);
            usleep(500000);
            initFlag = init(PID);

            gettimeofday(&ut_tv, NULL);
            const time_t sec = (time_t)ut_tv.tv_sec;
            gtm = gmtime(&sec);
            strftime(filename, sizeof(filename), "%Y-%m-%d-%H:%M:%S.txt", gtm);
        }
        usleep(USLEEP_PERIOD);
    }
    fclose(outfile);

    libusb_close(handle);
    libusb_exit(NULL);

    return EXIT_SUCCESS;
}

