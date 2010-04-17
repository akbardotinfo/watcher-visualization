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

#include "sharedStream.h"
#include "replayState.h"
#include "watcherd.h"
#include "database.h"

#include <libwatcher/seekWatcherMessage.h>
#include <libwatcher/speedWatcherMessage.h>
#include <libwatcher/playbackTimeRange.h>

#include <boost/weak_ptr.hpp>
#include <boost/foreach.hpp>

#include <list>

namespace {

uint32_t getNextUID()
{
    static uint32_t UID = 0;
    static boost::mutex uidlock;

    boost::mutex::scoped_lock l(uidlock);
    ++UID;
    return UID;
}

} // namespace

namespace watcher {

INIT_LOGGER(SharedStream, "SharedStream");

class SharedStreamImpl {
    public:
	Watcherd& watcher_;
	uint32_t uid_;
	boost::shared_ptr<ReplayState> replay_;

	boost::mutex lock_;
	std::list<ServerConnectionWeakPtr> clients_; // clients subscribed to this stream

	SharedStreamImpl(Watcherd& wd) : watcher_(wd), uid_(getNextUID()) {}
};

using namespace watcher::event;

uint32_t SharedStream::getUID() const
{
    return impl_->uid_;
}

SharedStream::SharedStream(Watcherd& wd) : isPlaying_(false), isLive_(true), impl_(new SharedStreamImpl(wd))
{
    TRACE_ENTER();
    TRACE_EXIT();
}

SharedStream::~SharedStream()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

void SharedStream::seek(const SeekMessagePtr& p)
{
    TRACE_ENTER();
    if (p->offset == SeekMessage::eof) {
	if (isLive_) {
	    // nothing to do
	} else if (impl_->replay_->speed() >= 0) {
	    // switch to live stream
	    isLive_ = true;
	    impl_->replay_->pause();
	    if (isPlaying_)
		impl_->watcher_.subscribe(shared_from_this());
	} else {
	    impl_->replay_->seek( SeekMessage::eof );
	}
    } else {
	impl_->replay_->seek(p->offset);

	// when switching from live to replay while playing, kick off the replay strand
	if (isLive_ && isPlaying_)
	    impl_->replay_->run();
	isLive_ = false;
    }
    TRACE_EXIT();
}

void SharedStream::start()
{
    TRACE_ENTER();
    LOG_DEBUG("in: isPlaying_=" << isPlaying_ << ", isLive_=" << isLive_);
    if (!isPlaying_) {
	isPlaying_ = true;
	if (isLive_)
	    impl_->watcher_.subscribe(shared_from_this());
	else
	    impl_->replay_->run();
    }
    LOG_DEBUG("out: isPlaying_=" << isPlaying_ << ", isLive_=" << isLive_);
    TRACE_EXIT();
}

void SharedStream::stop()
{
    TRACE_ENTER();
    LOG_DEBUG("isPlaying_=" << isPlaying_ << ", isLive_=" << isLive_);
    if (isPlaying_) {
	LOG_DEBUG("stopping playback");
	if (isLive_) {
	    impl_->watcher_.unsubscribe(shared_from_this());
	    /* save current Timestamp so we can resume from the database */
	    isLive_ = false;
	    impl_->replay_->seek( getCurrentTime() );
	} else
	    impl_->replay_->pause();
	isPlaying_ = false;
    } else
	LOG_DEBUG("stop message received, but playback is stopped");
    LOG_DEBUG("out: isPlaying_=" << isPlaying_ << ", isLive_=" << isLive_);
    TRACE_EXIT();
}

void SharedStream::speed(const SpeedMessagePtr& p)
{
    TRACE_ENTER();
    LOG_DEBUG("isPlaying_=" << isPlaying_ << ", isLive_=" << isLive_);

    if (p->speed == 0) {
	/* special case, speed==0 means StopMessage.  This is to avoid
	 * a Bad_arg exception from ReplayState. */
	LOG_INFO("got speed==0, emulating StopMessage");
	stop();
    } else if (isLive_ && p->speed >= 1.0f) {
	// ignore, can't predict the future
    } else {
	impl_->replay_->speed(p->speed);
	if (isLive_) {
	    isLive_ = false;
	    /* when transitioning from live playback, automatically
	     * seek to the end of the database.
	     */
	    impl_->replay_->seek( SeekMessage::eof );
	    if (isPlaying_)
		impl_->replay_->run();
	}
    }

    LOG_DEBUG("out: isPlaying_=" << isPlaying_ << ", isLive_=" << isLive_);
    TRACE_EXIT();
}

/** Returns a PlaybackTimeRangeMessage event to the sender with the timestamps of
 * the first and last event in the database.
 */
void SharedStream::range(PlaybackTimeRangeMessagePtr p)
{
    TRACE_ENTER();

    TimeRange r(event_range());
    p->min_ = r.first;
    p->max_ = r.second;
    sendMessage(p);

    TRACE_EXIT();
}

/** send a message to all clients subscribed to this shared stream. */
void SharedStream::sendMessage(MessagePtr m)
{
    TRACE_ENTER();
    boost::mutex::scoped_lock lck(impl_->lock_);

    /* weak_ptr doesn't have an operator== so we can't keep a dead list. */
    std::list<ServerConnectionWeakPtr> alive;

    bool dead = false;
    BOOST_FOREACH(ServerConnectionWeakPtr ptr, impl_->clients_) {
	ServerConnectionPtr conn = ptr.lock();
	if (conn) {
	    conn->sendMessage(m);
	    alive.push_front(ptr);
	} else
	    dead = true;
    }

    if (alive.empty()) {
	LOG_INFO("no more waiting clients");
	impl_->watcher_.unsubscribe(shared_from_this());
    } else if (dead)
	impl_->clients_ = alive;

    TRACE_EXIT();
}

void SharedStream::sendMessage(const std::vector<MessagePtr>& msgs)
{
    TRACE_ENTER();
    boost::mutex::scoped_lock lck(impl_->lock_);

    /* weak_ptr doesn't have an operator== so we can't keep a dead list. */
    std::list<ServerConnectionWeakPtr> alive;

    bool dead = false;
    BOOST_FOREACH(ServerConnectionWeakPtr ptr, impl_->clients_) {
	ServerConnectionPtr conn = ptr.lock();
	if (conn) {
	    conn->sendMessage(msgs);
	    alive.push_front(ptr);
	} else
	    dead = true;
    }

    if (alive.empty()) {
	LOG_INFO("no more waiting clients");
	impl_->watcher_.unsubscribe(shared_from_this());
    } else if (dead)
	impl_->clients_ = alive;

    TRACE_EXIT();
}

void SharedStream::subscribe(ServerConnectionPtr p)
{
    TRACE_ENTER();

    boost::mutex::scoped_lock lck(impl_->lock_);

    // shared_from_this() doesn't work in a ctor, so delay creation of the ReplayState until a client subscribes
    // FIXME: this assumes that all clients are using the same io_service.
    if (! impl_->replay_.get())
	impl_->replay_.reset(new ReplayState(p->io_service(), shared_from_this()));

    impl_->clients_.push_front(p);
    TRACE_EXIT();
}

/* list.remove() requires operator== which weak_ptr does not have */
template <typename T>
void weak_ptr_remove(std::list<boost::weak_ptr<T> > l, boost::weak_ptr<T> v)
{
    for (typename std::list<boost::weak_ptr<T> >::iterator it = l.begin(); it != l.end();)
    {
	if ( !(*it < v) && !(v < *it) )
	    it = l.erase(it);
	else
	    ++it;
    }
}
void SharedStream::unsubscribe(ServerConnectionPtr p)
{
    TRACE_ENTER();
    boost::mutex::scoped_lock lck(impl_->lock_);
    weak_ptr_remove(impl_->clients_, ServerConnectionWeakPtr(p));
    if (impl_->clients_.empty()) {
	LOG_INFO("no more waiting clients");
	impl_->watcher_.unsubscribe(shared_from_this());
    }
    TRACE_EXIT();
}

} // namespace
