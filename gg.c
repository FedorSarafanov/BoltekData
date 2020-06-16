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
#include <time.h>
#include <sys/time.h>



int main(int argc, char **argv)
{
//     char blacklist[] = "qwertyuiop[]asdfghjkl;'zxcvbnm/`=~!@#$%&(){}:<>?";
//     char numlist[] = "0123456789";
//     const char source[] = "asdasd43.20,01.23,0*CB";
//     int  startpoint = 32767;

//     char num1[64];
//     char num2[64];

//     // char *currentchar;
//     int j=0;
//     int k=0;
//     for (int i = 0; source[i] != 0; i++){
//         if (strchr(blacklist, source[i]) == NULL)
//         {
//             if (strchr("+-", source[i]) != NULL)
//             {
//                 startpoint = i;
//             }
//             if (source[i] == ',')
//             {
//                 startpoint = 32767;
//             }
//             if ((i>startpoint) || (strchr(numlist, source[i]) != NULL)){
//                 if (strlen(num1)<2){
//                     num1[j++]=source[i]; 
//                 }
//                 else{
//                     num2[k++]=source[i]; 
//                 }
//             }
//         }

//     }
//     printf(num1);
       // char i[]  = "s";
       // int f = i[0];
       // printf("%d",f);
//     printf(num2);
    // printf("asdass\n");





	gettimeofday(&ut_tv, NULL);
	const time_t sec = (time_t)ut_tv.tv_sec;
	const time_t usec = (time_t)ut_tv.tv_usec;


	gtm = gmtime(&sec);
	time_t GMT = mktime(gtm);

	strftime(outtime, sizeof(outtime), "%Y-%m-%d_%H:%M:%S", gtm);


	printf("%s.%d\n",outtime,usec);
	// struct tm* gmt;
	// gmt = gmtime(currtime);

	// struct timeval utime;
    return 0;
}

