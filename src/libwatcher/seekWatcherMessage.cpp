/** @file
 * @author Michael Elkins <michael.elkins@sparta.com>
 * @date 2009-03-20
 */
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/export.hpp>

#include "seekWatcherMessage.h"

namespace watcher {
    namespace event {
        INIT_LOGGER(SeekMessage, "Message.SeekMessage");

        /**
         * Seek to a particular point in time in the event stream.
         * @param offset_ time at which to seek to
         * @param w how to interpret the time offset
         */
        SeekMessage::SeekMessage(float offset_, whence w)
            : Message(SEEK_MESSAGE_TYPE, SEEK_MESSAGE_VERSION), offset(offset_), rel(w)
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }

        std::ostream& operator<< (std::ostream& o, const SeekMessage& m)
        {
            TRACE_ENTER();
            TRACE_EXIT();
            return o << "SeekMessage(offset=" << m.offset << " , rel=" << m.rel << ')';
        }

        template <typename Archive>
        void SeekMessage::serialize(Archive& ar, const unsigned int /* version */)
        {
            TRACE_ENTER();
            ar & boost::serialization::base_object<Message>(*this);
            ar & offset;
            ar & rel;
            TRACE_EXIT();
        }

    }
}

BOOST_CLASS_EXPORT(watcher::event::SeekMessage);
