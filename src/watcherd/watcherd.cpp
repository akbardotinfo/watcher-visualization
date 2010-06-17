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

#include <algorithm>

#include "watcherd.h"
#include "singletonConfig.h"
#include "logger.h"
#include "sharedStream.h"
#include <libwatcher/listStreamsMessage.h>

#include <boost/foreach.hpp>

using namespace watcher;
using namespace watcher::event;
using namespace std;
using namespace boost;

INIT_LOGGER(Watcherd, "Watcherd"); 

Watcherd::Watcherd(bool ro) : 
    config_(SingletonConfig::instance()),
    serverMessageHandlerPtr(new ServerMessageHandler),
    readOnly_(ro)
{
    TRACE_ENTER();
    pthread_rwlock_init(&messageRequestorsLock, 0);
    TRACE_EXIT();
}

Watcherd::~Watcherd()
{
    TRACE_ENTER();
    pthread_rwlock_destroy(&messageRequestorsLock);
    TRACE_EXIT();
}

void Watcherd::run(const std::string &address, const std::string &port, const int &threadNum)
{
    TRACE_ENTER(); 

    // Block all signals for background thread.
    sigset_t new_mask;
    sigfillset(&new_mask);
    sigset_t old_mask;
    pthread_sigmask(SIG_BLOCK, &new_mask, &old_mask);

    // Run server in background thread.
    serverConnection.reset(new Server(*this, address, port, (size_t)threadNum, serverMessageHandlerPtr));
    connectionThread = boost::thread(boost::bind(&watcher::Server::run, serverConnection));

    // Restore previous signals.
    pthread_sigmask(SIG_SETMASK, &old_mask, 0);

    // Wait for signal indicating time to shut down.
    sigset_t wait_mask;
    sigemptyset(&wait_mask);
    sigaddset(&wait_mask, SIGINT);
    sigaddset(&wait_mask, SIGQUIT);
    sigaddset(&wait_mask, SIGTERM);
    pthread_sigmask(SIG_BLOCK, &wait_mask, 0);
    int sig = 0;
    sigwait(&wait_mask, &sig);

    // Stop the server.
    serverConnection->stop();
    connectionThread.join();
    TRACE_EXIT();
}

void Watcherd::subscribe(SharedStreamPtr client)
{
    TRACE_ENTER();

    pthread_rwlock_wrlock(&messageRequestorsLock);
    shared_ptr<pthread_rwlock_t> lock(&messageRequestorsLock, pthread_rwlock_unlock);

    messageRequestors.push_front(client);

    TRACE_EXIT();
}

void Watcherd::unsubscribe(SharedStreamPtr client)
{
    TRACE_ENTER();
    pthread_rwlock_wrlock(&messageRequestorsLock);
    shared_ptr<pthread_rwlock_t> lock(&messageRequestorsLock, pthread_rwlock_unlock);

    messageRequestors.remove(client);
    TRACE_EXIT();
}

/** Send a single message to all clients subscribed to the live stream */
void Watcherd::sendMessage(MessagePtr msg)
{
    TRACE_ENTER();
    pthread_rwlock_rdlock(&messageRequestorsLock);
    shared_ptr<pthread_rwlock_t> lock(&messageRequestorsLock, pthread_rwlock_unlock);

    // bind can't handle overloaded functions.  use member function pointer to help
    void (SharedStream::*ptr)(MessagePtr) = &SharedStream::sendMessage;
    for_each(messageRequestors.begin(), messageRequestors.end(), bind(ptr, _1, msg));
    TRACE_EXIT();
}

/** Send a set of messages to all clients subscribed to the live stream */
void Watcherd::sendMessage(const std::vector<MessagePtr>& msg)
{
    TRACE_ENTER();
    pthread_rwlock_rdlock(&messageRequestorsLock);
    shared_ptr<pthread_rwlock_t> lock(&messageRequestorsLock, pthread_rwlock_unlock);

    // bind can't handle overloaded functions.  use member function pointer to help
    void (SharedStream::*ptr)(const std::vector<MessagePtr>&) = &SharedStream::sendMessage;
    for_each(messageRequestors.begin(), messageRequestors.end(), bind(ptr, _1, msg));
    TRACE_EXIT();
}

void Watcherd::listStreams(ServerConnectionPtr conn)
{
    TRACE_ENTER();
    boost::shared_lock<boost::shared_mutex> lock(allStreamsLock);

    ListStreamsMessagePtr msg(new ListStreamsMessage());
    BOOST_FOREACH(SharedStreamPtr stream, allStreams) {
	msg->evstreams.push_back( EventStreamInfoPtr(new EventStreamInfo( stream->getUID(), stream->getDescription() )) );
    }
    conn->sendMessage(msg);

    TRACE_EXIT();
}

SharedStreamPtr Watcherd::getStream(uint32_t uid)
{
    TRACE_ENTER();
    boost::shared_lock<boost::shared_mutex> lock(allStreamsLock);
    BOOST_FOREACH(SharedStreamPtr stream, allStreams) {
	if (stream->getUID() == uid) {
	    TRACE_EXIT_RET(true);
	    return stream;
	}
    }
    TRACE_EXIT_RET(false);
    return SharedStreamPtr(static_cast<SharedStream*>(0));
}

/** add a stream to the list of all known streams */
void Watcherd::addStream(SharedStreamPtr p)
{
    boost::unique_lock<boost::shared_mutex> lock(allStreamsLock);
    allStreams.push_front(p);
}

/** remove a stream from the list of all known streams.  also removes from the subscribed list. */
void Watcherd::removeStream(SharedStreamPtr p)
{
    unsubscribe(p);

    boost::unique_lock<boost::shared_mutex> lock(allStreamsLock);
    allStreams.remove(p);
}
