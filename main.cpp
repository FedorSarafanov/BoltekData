#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <atomic>

#include "Logger.hpp"
#include "Writer.hpp"
#include "BoltekDevice.hpp"
#include "BoltekTTY.hpp"
#include "BoltekUSB.hpp"
#include "Config.hpp"


#define USLEEP_PERIOD 1000

#define helpData  "Boltek fluxmeter client v1.0\n"\
                  "F. Kuterin, F. Sarafanov (c) IAPRAS 2020-2021\n\n"\
                  "Use \t./get_data --sid=[SID] --prefix=[PREFIX] --pid=[PID],\n where [PID] can be obtained using \n"\
                  "command `lsusb  -d 0x0403: -v | grep idProduct`,\n [PREFIX] -- location string (without whitespaces or dash)\n"\
                  "[SID] can be obtained using command `lsusb  -d 0x0403: -v | grep Serial`.\n"

std::atomic<bool> quit(false);

void got_signal(int)
{
    quit.store(true);
}


int main(int argc, char *argv[])
{

    Config config(argc, argv, "boltek.ini");
    if (config.get_value("help") || config.get_value("h")){
        printf(helpData);
        return EXIT_SUCCESS;
    }

    char SID[100]; 
    std::string S_SID = config.get_value("sid", "E2150533");
    sscanf(S_SID.c_str(),"%s",&SID);

    unsigned PID;
    std::string S_PID = config.get_value("pid", "0xf245");
    sscanf(S_PID.c_str(),"%x",&PID);

    std::string prefix = config.get_value("prefix", "default");
    std::string log_fn = config.get_value("log", "boltek.log");
    std::string data_folder = config.get_value("folder", "data");
    std::string tty_address = config.get_value("tty", "none");

    struct sigaction sa;
    memset( &sa, 0, sizeof(sa) );
    sa.sa_handler = got_signal;
    sigfillset(&sa.sa_mask);
    sigaction(SIGINT,&sa,NULL);

    Logger logger(log_fn);
    Writer writer(prefix, &logger, data_folder);
    BoltekDevice *boltek_device;

    if (tty_address != "none")
    {
        boltek_device = new BoltekTTY(tty_address, &logger, &writer, &quit);
    }
    else
    {
        boltek_device = new BoltekUSB(tty_address, &logger, &writer, &quit);
    }

    std::string line("");
    while(true){
        for (auto symbol : boltek_device->read_data())
        {
            switch (symbol)
            {
                case '$':
                    if (line.length() == 5 && !isdigit(line[0]))
                    {
                        writer.write(line);
                    }
                    line = "";
                    break;
                case '+':
                case '-':
                    line += symbol;
                    break;
                default:
                    if (isdigit(symbol) && line.length() < 5)
                    {
                        line += symbol;
                    }
            }
        }

        usleep(USLEEP_PERIOD);

        if ( quit.load() ) 
        {
            delete boltek_device;
            logger.log("Received SIGINT");
            break;
        }
    }       
    return EXIT_SUCCESS;
}