#include <iostream>

#include "initConfig.h"
#include "libconfig.h++"
#include "getopt.h"

using namespace watcher;
using namespace libconfig;
using namespace std;

bool watcher::initConfig(
            libconfig::Config &config, 
            int argc, 
            char **argv, 
            const char configFileChar,
            const char *configFileString)
{
    int c;
    int digit_optind = 0;

    while (true) 
    {
        int this_option_optind = optind ? optind : 1;
        int option_index = 0;
        static struct option long_options[] = {
            {configFileString, required_argument, 0, configFileChar},
            {0, 0, 0, 0}
        };

        char args[] = { configFileChar, ':', '\n' };
        c = getopt_long(argc, argv, args, long_options, &option_index);

        if (c == -1)
            break;

        if (c==configFileChar)
        {
            try
            {
                config.readFile(optarg);
                return true;
            }
            catch (ParseException &e)
            {
                cerr << "Error reading configuration file " << optarg << ": " << e.what() << endl;
                cerr << "Error: \"" << e.getError() << "\" on line: " << e.getLine() << endl;
            }
            catch (FileIOException &e)
            {
                cerr << "Unable to read file " << optarg << " given as configuration file on the command line." << endl;
            }
        }
        else
        {
            printf("?? getopt returned character code 0%o ??\n", c);
        }
    }
    return false;
}
