/*
 * Simple libusb-1.0 test programm
 * It openes an USB device, expects two Bulk endpoints,
 *   EP1 should be IN
 *   EP2 should be OUT
 * It alternates between reading and writing a packet to the Device.
 * It uses Synchronous device I/O
 *
 * Compile:
 *   gcc -lusb-1.0 -o test test.c
 * Run:
 *   ./test
 * Thanks to BertOS for the example:
 *   http://www.bertos.org/use/tutorial-front-page/drivers-usb-device
 *
 * For Documentation on libusb see:
 *   http://libusb.sourceforge.net/api-1.0/modules.html
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

//change if your libusb.h is located elswhere
#include <libusb-1.0/libusb.h>
//or uncomment this line:
//#include <libusb.h>
//and compile with:
//gcc -lusb-1.0 -o test -I/path/to/libusb-1.0/ test.c


#define USB_VENDOR_ID	    0x0403      /* USB vendor ID used by the device
                                         * 0x0483 is STMs ID
                                         */
#define USB_PRODUCT_ID	    0xf245      /* USB product ID used by the device */
#define USB_ENDPOINT_IN	    (LIBUSB_ENDPOINT_IN  | 1)   /* endpoint address */
#define USB_ENDPOINT_OUT	(LIBUSB_ENDPOINT_OUT | 2)   /* endpoint address */
#define USB_TIMEOUT	        3000        /* Connection timeout (in ms) */

static libusb_context *ctx = NULL;
static libusb_device_handle *handle;

#define BUFFER_SIZE 1024

static uint8_t receiveBuf[BUFFER_SIZE];
uint8_t transferBuf[BUFFER_SIZE];
char string[BUFFER_SIZE];
uint16_t counter=0;
char * ss;


char * hexdumpstr (const char *src, size_t size)
{
  static const char hex[] = "0123456789abcdef";
  char *result = (char *)malloc((size > 0)? size*3: 1),
    *p = result;

  if (result) {
    while (size-- > 0) {
      *p++ = hex[(*src >> 4) & 0x0f];
      *p++ = hex[*src++ & 0x0f];
      *p++ = ' ';
    }
    p[(p > result)? -1: 0] = 0;
  }

  return result;
}

const char * rmv(char *str){
    int i,j;
    i = 0;
    while(i<strlen(str))
    {
        if (str[i]==0x01)
        {
            for (j=i; j<strlen(str); j++)
                str[j]=str[j+1];
        }
        else i++;
    }
    return str;
}



/*
 * Read a packet
 */
static int usb_read(void)
{
	int nread, ret;
	ret = libusb_bulk_transfer(handle, USB_ENDPOINT_IN, receiveBuf, sizeof(receiveBuf),
			&nread, USB_TIMEOUT);
	if (ret){
		printf("ERROR in bulk read: %d\n", ret);
		return -1;
    }
	else{
		printf("%d receive %d bytes from device:", ++counter, nread);
        // for (size_t i=0;i<nread;++i) printf("%02X ", receiveBuf[i]);
        memset(string, 0, BUFFER_SIZE);
        strncpy(string, receiveBuf, nread);

        // ss = rmv(string);

        // printf("%d", strlen(string));

        // printf("%s", ss);
        printf("\n");
		// printf("%s", receiveBuf);  //Use this for benchmarking purposes
		return 0;
    }
}


/*
 * write a few bytes to the device
 *
 */
uint16_t count=0;
static int usb_write(void)
{
	int n, ret;
    //count up
    // n = sprintf(transferBuf, "%d\0",count++);
    //write transfer
    //probably unsafe to use n twice...
   
   	//ret = libusb_bulk_transfer(handle, USB_ENDPOINT_OUT, transferBuf, n,
	//		&n, USB_TIMEOUT);

    n = 0;
	ret = libusb_bulk_transfer(handle, USB_ENDPOINT_OUT, transferBuf, n, &n, USB_TIMEOUT);

    //Error handling
    switch(ret){
        case 0:
            printf("send %d bytes to device\n", n);
            return 0;
        case LIBUSB_ERROR_TIMEOUT:
            printf("ERROR in bulk write: %d Timeout\n", ret);
            break;
        case LIBUSB_ERROR_PIPE:
            printf("ERROR in bulk write: %d Pipe\n", ret);
            break;
        case LIBUSB_ERROR_OVERFLOW:
            printf("ERROR in bulk write: %d Overflow\n", ret);
            break;
        case LIBUSB_ERROR_NO_DEVICE:
            printf("ERROR in bulk write: %d No Device\n", ret);
            break;
        default:
            printf("ERROR in bulk write: %d\n", ret);
            break;
    }
    return -1;

}

void usb_control_out(uint8_t bRequest, uint16_t wValue, uint16_t wIndex) {
    int rc = libusb_control_transfer(handle,
		0x40, //uint8_t  	bmRequestType,
		bRequest,
		wValue,
		wIndex,
		NULL, // unsigned char *  	data,
		0, // uint16_t  	wLength,
		USB_TIMEOUT //unsigned int  	timeout 
	);

    printf("control result %d\n", rc);
}

void usb_control_in(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength) {
    if (wLength > 2) {
        printf("usb_control_in length arg must be 0,1,2, no more, not %u\n", (unsigned)wLength);
    }
    uint8_t cr2[2];
    int rc2 = libusb_control_transfer(handle,
		0xC0, //uint8_t  	bmRequestType,
		bRequest,
		wValue,
		wIndex,
		cr2, // unsigned char *  	data,
		wLength, // uint16_t  	wLength,
		USB_TIMEOUT //unsigned int  	timeout 
	);
    switch(wLength) {
    case 0: printf("control result %d\n", rc2); break;
    case 1: printf("control result %d: %02X\n", rc2, (unsigned)cr2[0]); break;
    case 2: printf("control result %d: %02X %02X\n", rc2, (unsigned)cr2[0], (unsigned)cr2[1]); break;
    }
}

/*
 * on SIGINT: close USB interface
 * This still leads to a segfault on my system...
 */
static void sighandler(int signum)
{
    printf( "\nInterrupt signal received\n" );
	if (handle){
        libusb_release_interface (handle, 0);
        printf( "\nInterrupt signal received1\n" );
        libusb_close(handle);
        printf( "\nInterrupt signal received2\n" );
	}
	printf( "\nInterrupt signal received3\n" );
	libusb_exit(NULL);
	printf( "\nInterrupt signal received4\n" );

	exit(0);
}

int main(int argc, char **argv)
{
    //Pass Interrupt Signal to our handler
	signal(SIGINT, sighandler);

	libusb_init(&ctx);
	// libusb_set_debug(ctx, 3);

    //Open Device with VendorID and ProductID
	handle = libusb_open_device_with_vid_pid(ctx,
				USB_VENDOR_ID, USB_PRODUCT_ID);
	if (!handle) {
		perror("device not found");
		return 1;
	}

	int r = 1;
	//Claim Interface 0 from the device
    r = libusb_claim_interface(handle, 0);
	if (r < 0) {
		fprintf(stderr, "usb_claim_interface error %d\n", r);
		return 2;
	}
	printf("Interface claimed\n");

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

    FILE *outfile;
    outfile = fopen("data.txt", "a");

	while (1){
		usb_read();
        fputs(string, outfile);
//		usb_write();
        usleep(50000);
    }
    fclose(outfile);
    //never reached
	libusb_close(handle);
	libusb_exit(NULL);

	return 0;
}

