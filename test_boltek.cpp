#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <ctime>
#include <atomic>

#include "logger.h"
#include "writer.h"
#include "boltek.h"

#include <libusb-1.0/libusb.h> 

#define USLEEP_PERIOD 50000
char logname[15] = "boltek.log";
char SID[100] = "E2150533"; 

#define helpData  "Boltek fluxmeter client v0.1\n"\
                  "F. Kuterin, F. Sarafanov (c) IAPRAS 2020\n\n"\
                  "Use \t./get_data [SID] [PREFIX] [PID],\n where [PID] can be obtained using \n"\
                  "command `lsusb  -d 0x0403: -v | grep idProduct`,\n [PREFIX] -- location string (without whitespaces or dash)\n"\
                  "[SID] can be obtained using command `lsusb  -d 0x0403: -v | grep Serial`.\n"\
                  "\nYou can see log in file 'boltek.log'" 

std::atomic<bool> quit(false);

void got_signal(int)
{
    quit.store(true);
}


int main(int argc, char *argv[])
{

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

    // SIGINT sighandler
    struct sigaction sa;
    memset( &sa, 0, sizeof(sa) );
    sa.sa_handler = got_signal;
    sigfillset(&sa.sa_mask);
    sigaction(SIGINT,&sa,NULL);


    Logger logger("boltek.log");
    Writer writer(std::string("test"), &logger);
    Boltek boltek(SID, &logger, &writer, &quit);

    std::string line("");

    bool boltek_signal_connected = true;

    long int empty_counter = 0;
    struct tm *last_write_time;

    while(true){
        empty_counter++;
        for (auto symbol : boltek.read_data())
        {
            switch (symbol){
                case '$':
                    if (line.length() == 5 && !isdigit(line[0]))
                    {
                        empty_counter = 0;
                        last_write_time = writer.write(line);
                    }
                    line = "";
                    break;
                case '+':
                case '-':
                    line += symbol;
                    break;
                default:
                    if (isdigit(symbol) && line.length()<5){
                        line += symbol;
                    }
            }
        }
        if (empty_counter > 1000) { empty_counter = 10; }
        if (empty_counter > 5) {
            if (boltek_signal_connected)
            {
                boltek_signal_connected = false;
                logger.log(last_write_time, "Signal cable disconnected");
            }
        }
        if (empty_counter == 0)
        {
            if (!boltek_signal_connected)
            {
                boltek_signal_connected = true;
                logger.log(last_write_time, "Signal cable connected");
                writer.open();
            }
        }
        usleep(USLEEP_PERIOD);
        if( quit.load() ) {
            logger.log("Received SIGINT");
            break;
        }
    }
}