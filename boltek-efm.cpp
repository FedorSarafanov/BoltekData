#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <atomic>

#include "Config.hpp"
#include "Logger.hpp"
#include "Writer.hpp"

#include "BoltekDevice.hpp"
#include "BoltekTTY.hpp"
#include "BoltekUSB.hpp"


constexpr int READ_DATA_DELAY_US(1000);
constexpr const char* HELP_TEXT(\
    "Boltek EFM-100 fluxmeter client v1.1\n"\
    "F. Kuterin, F. Sarafanov (c) IAPRAS Jun 16 2020 - " __DATE__ "\n\n"\
    "Use: boltek-efm [arguments]\n\n"\
    "Arguments:\n"\
    "  -config, --config   Configuration file\n"\
    "  -folder, --folder   Folder for save data files\n"\
    "  -log, --log         Log file. Pass \"none\" for redirect to stdout\n"\
    "  -prefix, --prefix   Data files prefix\n"\
    "  -pid, --pid         Boltek (USB) product id. For example, 0xf245\n"\
    "  -sid, --sid         Boltek (USB) serial id. For example, E2150533\n"\
    "  -tty, --tty         Boltek (TTY) address, for example /dev/ttyUSB0\n"\
    "  -h,   --help        Show this help and exit\n");


std::atomic<bool> quit(false);

void got_signal(int)
{
    quit.store(true);
}


int main(int argc, char *argv[])
{

    Config config(argc, argv, "boltek-efm.ini");
    if (config.get_value("help") || config.get_value("h"))
    {
        printf(HELP_TEXT);
        return EXIT_SUCCESS;
    }

    char SID[100]; 
    std::string S_SID = config.get_value("sid", "E2150533");
    sscanf(S_SID.c_str(),"%s",&SID);

    unsigned PID;
    std::string S_PID = config.get_value("pid", "0xf245");
    sscanf(S_PID.c_str(),"%x",&PID);

    std::string prefix = config.get_value("prefix", "default");
    std::string log_fn = config.get_value("log", "boltek-efm.log");
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

        usleep(READ_DATA_DELAY_US);

        if ( quit.load() ) 
        {
            delete boltek_device;
            logger.log("Received SIGINT");
            break;
        }
    }
    
    return EXIT_SUCCESS;
}