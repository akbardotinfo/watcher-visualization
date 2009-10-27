/* Copyright 2009 SPARTA, Inc., dba Cobham Analytic Solutions
 * 
 * This file is part of WATCHER.
 * 
 *     WATCHER is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU Affero General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 * 
 *     WATCHER is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU Affero General Public License for more details.
 * 
 *     You should have received a copy of the GNU Affero General Public License
 *     along with Watcher.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * Connects to a Watcher server and writes the event stream to a KML file suitable for use
 * with Google Earth.
 *
 * @author michael.elkins@cobham.com
 */

#include <unistd.h>
#include <getopt.h>
#include <stdint.h>

#include <cassert>
#include <iostream>
#include <cstdlib>

#include "logger.h"
#include "initConfig.h"
#include "singletonConfig.h"
#include "libwatcher/messageStream.h"
#include "libwatcher/watcherGraph.h"

using namespace watcher;

void write_kml(const WatcherGraph& graph, const std::string& outputFile); // kml.cc

namespace {

//arguments to getopt_long()
const option OPTIONS[] = {
    { "config", required_argument, 0, 'c' },
    { "help", no_argument, 0, 'h' },
    { "output-file", required_argument, 0, 'o' },
    { "refresh", required_argument, 0, 'r' },
    { "seek", required_argument, 0, 'S' },
    { 0, 0, 0, 0 }
};

#define TOOL_NAME "earthWatcher"
const char *CONFIG_FILE = TOOL_NAME ".cfg";
const char *OUTPUT_FILE = "watcher.kml";
const char *PROPERTY_FILE = TOOL_NAME ".log.properties";
const unsigned int DEFAULT_REFRESH = 1; // SECONDS

void usage()
{
    const char *SEP = "\t\t";   // separator for argument/description columns

    std::cout << "usage: earthWatcher [ -h ] [ -c FILE ]\n"
        "  -c, --config FILE" << SEP << "specify a configuration file (default: " << CONFIG_FILE << ")\n"
        "  -h, --help\t" << SEP << "display this help message\n"
        "  -o, --output-file FILE" << SEP << "specifies the output KML file (default: " << OUTPUT_FILE << ")\n"
        "  -r, --refresh SECS" << SEP << "write the the output every SECS seconds (default: " << DEFAULT_REFRESH << ")\n"
        "  -S, --seek POS" << SEP << "start event playback at timestamp POS (default: -1)\n"
        << std::endl;
}

} // end namespace

DECLARE_GLOBAL_LOGGER(TOOL_NAME);

int main(int argc, char **argv)
{
    TRACE_ENTER();

    const char *output_file = 0;
    Timestamp start_timestamp = -1 ; // default to EOF (live playback)
    unsigned int refresh = DEFAULT_REFRESH;

    for (int i; (i = getopt_long(argc, argv, "hc:o:r:S:", OPTIONS, 0)) != -1; ) {
        switch (i) {
            case 'c':
                break; //handled by initConfig()

            case 'o': // output-file
                output_file = optarg;
                break;

            case 'r':
                refresh = atoi(optarg);
                break;

            case 'S':
                start_timestamp = atol(optarg);
                break;

            case 'h':
            default:
                usage();
                TRACE_EXIT_RET(EXIT_SUCCESS);
                return EXIT_SUCCESS;
                break;
        }
    }

    libconfig::Config& config = SingletonConfig::instance();
    SingletonConfig::lock();
    std::string configFilename; //filled by initConfig()
    if (! watcher::initConfig(config, argc, argv, configFilename))
        std::cout << "Configuration file not found. Creating new configuration file and using default runtime values." << std::endl;
    SingletonConfig::unlock();

    std::string logConf(PROPERTY_FILE);
    if (!config.lookupValue("logPropertiesFile", logConf))
    {
        std::cout << "Unable to find logPropertiesFile setting in the configuration file, using default: " << logConf 
            << " and adding it to the configuration file." << std::endl;
        config.getRoot().add("logPropertiesFile", libconfig::Setting::TypeString) = logConf;
    }

    LOAD_LOG_PROPS(logConf); 
    LOG_INFO("Logger initialized from file \"" << logConf << "\"");

    std::string serverName("127.0.0.1");
    if (!config.lookupValue("server", serverName))
    {
        LOG_INFO("'server' not found in the configuration file, using default: " << serverName 
                << " and adding this to the configuration file.");
        config.getRoot().add("server", libconfig::Setting::TypeString) = serverName;
    }

    std::string service("watcherd");
    if (!config.lookupValue("service", service))
    {
        LOG_INFO("'service' not found in the configuration file, using default: " << service  
                << " and adding this to the configuration file.");
        config.getRoot().add("service", libconfig::Setting::TypeString) = service;
    }

    std::string outputFile(OUTPUT_FILE);
    if (output_file)
        outputFile = output_file; //initialize from user supplied argument
    else if (!config.lookupValue("outputFile", outputFile)) {
        LOG_INFO("'outputFile' not found in the configuration file, using default: " << outputFile  
                 << " and adding this to the configuration file.");
        config.getRoot().add("outputFile", libconfig::Setting::TypeString) = outputFile;
    }

    // open a message stream of live events for now
    MessageStreamPtr ms(MessageStream::createNewMessageStream(serverName, service, start_timestamp));
    if (!ms) {
        LOG_FATAL("Unable to create new message stream to server \"" << serverName << "\" using service (or port) \"" << service);
        TRACE_EXIT_RET(EXIT_FAILURE); 
        return EXIT_FAILURE;
    }

    LOG_INFO("Starting event playback");
    ms->startStream(); 

    srandom(time(0));//we use random() to select the icon below
    WatcherGraph graph;

    LOG_INFO("Waiting for events ");
    unsigned int messageNumber = 0;
    MessagePtr mp;
    time_t last_output = 0; // counter to allow for writing the kml file on a fixed time interval
    bool changed = false;
    while (ms->getNextMessage(mp)) {
        std::cout << "Message #" << (++messageNumber) << ": " << *mp << std::endl; 
        changed |= graph.updateGraph(mp);

        if (changed) {
            time_t now = time(0);
            if (now - last_output >= refresh) {
                graph.doMaintanence(); // expire stale links
                LOG_DEBUG("writing kml file");
                last_output = now;
                write_kml(graph, outputFile);
            }
            changed = false; // reset flag
        }
    }

    // Save any configuration changes made during the run.
    LOG_INFO("Saving last known configuration to " << configFilename); 
    SingletonConfig::lock();
    config.writeFile(configFilename.c_str());

    TRACE_EXIT_RET(0);
    return 0;
}