/* Copyright 2009 SPARTA, Inc., dba Cobham Analytic Solutions
 * 
 * This file is part of WATCHER.
 * 
 *     WATCHER is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 * 
 *     WATCHER is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 * 
 *     You should have received a copy of the GNU General Public License
 *     along with Watcher.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "singletonConfig.h"

using namespace watcher;
using namespace libconfig;

pthread_mutex_t SingletonConfig::accessMutex=PTHREAD_MUTEX_INITIALIZER;
std::string SingletonConfig::filename=""; 

INIT_LOGGER(SingletonConfig, "SingletonConfig");

// static
Config &SingletonConfig::instance()
{
    TRACE_ENTER();
    static Config theInstance;
    TRACE_EXIT();
    return theInstance;
}

void SingletonConfig::lock()
{
    TRACE_ENTER();
    pthread_mutex_lock(&accessMutex);
    TRACE_EXIT();
}

void SingletonConfig::unlock()
{
    TRACE_ENTER();
    pthread_mutex_unlock(&accessMutex);
    TRACE_EXIT();
}

void SingletonConfig::setConfigFile(const std::string &filename_)
{
    TRACE_ENTER();
    filename=filename_;
    TRACE_EXIT();
}

void SingletonConfig::saveConfig()
{
    TRACE_ENTER();
    instance().writeFile(filename.data());
    TRACE_EXIT();
}

