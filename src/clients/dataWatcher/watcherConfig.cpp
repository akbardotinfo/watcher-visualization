/* Copyright 2010 SPARTA, Inc., dba Cobham Analytic Solutions
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

#include <iostream>
#include <fstream>
#include <string>
#include <getopt.h>

#include <boost/filesystem.hpp>

#include "libconfig.h++"
#include "singletonConfig.h"
#include "logger.h"

namespace {

class singleton_config_lock {
    public:
    singleton_config_lock() { watcher::SingletonConfig::lock(); }
    ~singleton_config_lock() { watcher::SingletonConfig::unlock(); }
};

void usage(const char *prog)
{
    std::cout << "usage: " << prog << " [ -c CONFIG ] [ -h ] [ -n STREAMUID ]" << std::endl;
    exit(0);
}

}

DECLARE_GLOBAL_LOGGER("watcherGlobalLogger");

namespace watcher {
namespace config {
    std::string Server("localhost");
    int StreamUid = -1;

void initialize(int argc, char **argv)
{
    libconfig::Config& config = watcher::SingletonConfig::instance();
    singleton_config_lock lock;

    std::string configFilename(std::string(boost::filesystem::basename(argv[0])) + ".cfg");
    std::string logPropsFilename(std::string(boost::filesystem::basename(argv[0])) + ".log.properties");

    int i;
    while ((i = getopt(argc, argv, "c:n:h")) != -1) {
	switch(i) {
	    case 'c':
		configFilename = std::string(optarg);
		break;
	    case 'n':
		StreamUid = strtol(optarg, 0, 0);
		break;
	    case 'h':
		usage(argv[0]);
	}
    }

    try {
        // make sure it exists. 
        if (!boost::filesystem::exists(configFilename)) {
	    std::cerr << "Configuration file \"" << configFilename << "\", not found. Creating it." << std::endl;
	    std::ofstream f(configFilename.c_str(), std::ios_base::in | std::ios_base::out); 
            f.close();
        } else 
            config.readFile(configFilename.c_str());
	watcher::SingletonConfig::setConfigFile(configFilename);
    } catch (libconfig::ParseException &e) {
	std::cerr << "Error reading configuration file " << configFilename << ": " << e.what() << std::endl;
	std::cerr << "Error: \"" << e.getError() << "\" on line: " << e.getLine() << std::endl;
        exit(EXIT_FAILURE);  // !!!
    }

    libconfig::Setting& root = config.getRoot();
    if (!root.lookupValue("logProperties", logPropsFilename))
	root.add("logProperties", libconfig::Setting::TypeString) = logPropsFilename;
    if (!root.lookupValue("server", watcher::config::Server))
	root.add("server", libconfig::Setting::TypeString) = watcher::config::Server;

    if (!boost::filesystem::exists(logPropsFilename)) {
	std::cerr << "Log properties file not found - logging disabled." << std::endl;
	Logger::getRootLogger()->setLevel(Level::getOff());
    } else {
	LOAD_LOG_PROPS(logPropsFilename); 
	LOG_INFO("Logger initialized from file \"" << logPropsFilename << "\"");
    }
}

} // config
} // watcher

// vim:sw=4
