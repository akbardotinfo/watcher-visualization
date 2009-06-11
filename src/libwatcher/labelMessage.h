#ifndef WATCHER_LABEL_MESSAGE_DATA_H
#define WATCHER_LABEL_MESSAGE_DATA_H

#include <string>
#include <boost/asio.hpp>

#include "message.h"
#include "watcherColors.h"
#include "watcherGlobalFunctions.h"
#include "watcherTypes.h" 

namespace watcher {
    namespace event {
        class LabelMessage : public Message {
            public:
                // The data
                std::string label;
                float fontSize;
                Color foreground;
                Color background;
                Timestamp  expiration;    // this should be a float
                bool addLabel;              // add or remove the label depending on true or false here.
                GUILayer layer;             // which layer to put the label on 

                float lat, lng, alt; 

                // Attach a label to this node.
                LabelMessage(const std::string &label="", const int fontSize=10); 

                // Attach a label to an arbitrary node.
                LabelMessage(const std::string &label, const boost::asio::ip::address &address, const int fontSize=10);

                // Float a label in space
                LabelMessage(const std::string &label, const float &lat, const float &lng, const float &alt, const int fontSize=10);

                LabelMessage(const LabelMessage &other);

                bool operator==(const LabelMessage &other) const;
                LabelMessage &operator=(const LabelMessage &other);

                virtual std::ostream &toStream(std::ostream &out) const;
                std::ostream &operator<<(std::ostream &out) const { return toStream(out); }

            private:
                friend class boost::serialization::access;
                template <typename Archive> void serialize(Archive & ar, const unsigned int file_version);
                DECLARE_LOGGER();
        };

        typedef boost::shared_ptr<LabelMessage> LabelMessagePtr; 

        std::ostream &operator<<(std::ostream &out, const LabelMessage &mess);
    }
}
#endif // WATCHER_LABEL_MESSAGE_DATA_H
