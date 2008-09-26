#ifndef WATCHER_LABEL_MESSAGE_DATA_H
#define WATCHER_LABEL_MESSAGE_DATA_H

#include <string>
#include <boost/asio.hpp>

#include "logger.h"
#include "message.h"
#include "watcherColors.h"

namespace boost {
    namespace archive {
        class polymorphic_iarchive;
        class polymorphic_oarchive;
    }
}

namespace watcher 
{
    class LabelMessage : public Message
    {
        public:
            // The data
            std::string label;
            int fontSize;
            boost::asio::ip::address address;
            watcher::Color foreground;
            watcher::Color background;
            unsigned int expiration;
            bool addLabel;              // add or remove the label depending on true or false here.

            // Attach a label to this node.
            LabelMessage(const std::string &label="", const int fontSize=10); 

            // Attach a label to an arbitrary node.
            LabelMessage(const std::string &label, const boost::asio::ip::address &address, const int fontSize=10);

            LabelMessage(const LabelMessage &other);

            bool operator==(const LabelMessage &other) const;
            LabelMessage &operator=(const LabelMessage &other);

            virtual std::ostream &toStream(std::ostream &out) const;
            std::ostream &operator<<(std::ostream &out) const { return toStream(out); }

            virtual void serialize(boost::archive::polymorphic_iarchive & ar, const unsigned int file_version);
            virtual void serialize(boost::archive::polymorphic_oarchive & ar, const unsigned int file_version);

        private:
            DECLARE_LOGGER();
    };

    typedef boost::shared_ptr<LabelMessage> LabelMessagePtr; 

    std::ostream &operator<<(std::ostream &out, const LabelMessage &mess);
}

#endif // WATCHER_LABEL_MESSAGE_DATA_H
