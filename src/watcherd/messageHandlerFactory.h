#ifndef MESSAGE_HANDLER_FACTORY_H
#define MESSAGE_HANDLER_FACTORY_H

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include "messageTypesAndVersions.h"
#include "messageHandler.h"

namespace watcher 
{
    class MessageHandlerFactory : public boost::noncopyable
    {
        public:

            // static function that returns a pointer to a MessageHandler class based on the 
            // message type you want to handle. 
            typedef boost::shared_ptr<MessageHandler> MessageHandlerPtr
            static MessageHandlerPtr getMessageHandler(const MessageType &type);

            DECLARE_LOGGER(); 

        protected:
        private:
    };
}
#endif // MESSAGE_HANDLER_FACTORY_H
