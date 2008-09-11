#ifndef SINGLETON_CONFIG_H
#define SINGLETON_CONFIG_H

#include "logger.h"
#include "libconfig.h++"
#include "pthread.h"

namespace watcher
{
    class singletonConfig
    {
        public:
            static libconfig::Config &instance();

            static void lock();
            static void unlock();

            DECLARE_LOGGER();

        private:
            singletonConfig();
            ~singletonConfig();

            static pthread_mutex_t accessMutex;
    };
}

#endif // SINGLETON_CONFIG_H