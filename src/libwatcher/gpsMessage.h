#ifndef WATCHER_GPS__MESSAGE_DATA_H
#define WATCHER_GPS__MESSAGE_DATA_H

#include <string>
#include <boost/serialization/base_object.hpp>

#include "message.h"

namespace watcher {
    namespace event {
        class GPSMessage : public Message {
            public:
                // The data
                float lat;
                float lng;
                float alt;

                GPSMessage(const float &lat=0.0, const float &lng=0.0, const float &alt=0.0);
                GPSMessage(const GPSMessage &other);

                bool operator==(const GPSMessage &other) const;
                GPSMessage &operator=(const GPSMessage &other);

                virtual std::ostream &toStream(std::ostream &out) const;
                std::ostream &operator<<(std::ostream &out) const { return toStream(out); }

                template <typename Archive>
                void serialize(Archive & ar, const unsigned int file_version)
                {
                    TRACE_ENTER();
                    ar & boost::serialization::base_object<Message>(*this);
                    ar & lat;
                    ar & lng;
                    ar & alt;
                    TRACE_EXIT();
                }
            private:
                DECLARE_LOGGER();
        };

        typedef boost::shared_ptr<GPSMessage> GPSMessagePtr; 

        std::ostream &operator<<(std::ostream &out, const GPSMessage &mess);
    }
}

#endif // WATCHER_GPS__MESSAGE_DATA_H
