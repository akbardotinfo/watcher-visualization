#include "watcherSerialize.h"
#include "watcherGlobalFunctions.h" // for address serialization
#include "colorMessage.h"

using namespace std;
using namespace boost;

namespace watcher {
    namespace event {
        INIT_LOGGER(ColorMessage, "Message.ColorMessage");

        ColorMessage::ColorMessage() : 
            Message(COLOR_MESSAGE_TYPE, COLOR_MESSAGE_VERSION),
            color(Color::black),
            flashPeriod(0),
            expiration(0),
            layer(PHYSICAL_LAYER)
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }


        ColorMessage::ColorMessage(
                                   const Color &c, 
                                   const boost::asio::ip::address &address,
                                   const Timestamp &e,
                                   const Timestamp &f) : 
            Message(COLOR_MESSAGE_TYPE, COLOR_MESSAGE_VERSION),
            color(c), 
            flashPeriod(e),
            expiration(f),
            layer(PHYSICAL_LAYER)
        {
            TRACE_ENTER();
            fromNodeID=address;
            TRACE_EXIT();
        }

        ColorMessage::ColorMessage(const ColorMessage &other) : 
            Message(other.type, other.version),
            color(other.color), 
            flashPeriod(other.flashPeriod),
            expiration(other.expiration),
            layer(other.layer)
        {
            TRACE_ENTER();
            (*this)=other;
            TRACE_EXIT();
        }

        bool ColorMessage::operator==(const ColorMessage &other) const
        {
            TRACE_ENTER();

            bool retVal = 
                Message::operator==(other) && 
                color == other.color && 
                layer == other.layer;

            TRACE_EXIT_RET(retVal);
            return retVal;
        }

        ColorMessage &ColorMessage::operator=(const ColorMessage &other)
        {
            TRACE_ENTER();

            Message::operator=(other);
            color = other.color;
            flashPeriod = other.flashPeriod;
            expiration = other.expiration;
            layer = other.layer; 

            TRACE_EXIT();
            return *this;
        }

        // virtual 
        std::ostream &ColorMessage::toStream(std::ostream &out) const
        {
            TRACE_ENTER();

            Message::toStream(out);
            out << " color: " << color; 
            out << " flashPeriod: " << flashPeriod;
            out << " expiration: " << expiration;
            out << " layer: " << layer; 
            TRACE_EXIT();
            return out;
        }

        ostream& operator<<(ostream &out, const ColorMessage &mess)
        {
            TRACE_ENTER();
            mess.operator<<(out);
            TRACE_EXIT();
            return out;
        }

        template <typename Archive> void ColorMessage::serialize(Archive& ar, const unsigned int /* file_version */)
        {
            TRACE_ENTER();
            ar & boost::serialization::base_object<Message>(*this);
            ar & color;
            ar & flashPeriod;
            ar & expiration;
            ar & layer; 
            TRACE_EXIT();
        }
    }
}

BOOST_CLASS_EXPORT(watcher::event::ColorMessage);