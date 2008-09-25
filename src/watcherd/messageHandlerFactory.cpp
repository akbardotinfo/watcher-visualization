#include "messageHandlerFactory.h"
#include "messageStatusHandler.h"
#include "testMessageHandler.h"
#include "gpsMessageHandler.h"
#include "labelMessageHandler.h"

using namespace watcher;

INIT_LOGGER(MessageHandlerFactory, "MessageHandlerFactory");

//
// If the messagehandlers stay just as a couple of fucntions, I may make them 
// all singletons and return pointers to static instances in order to save on
// memory and 'new'ing time. 
//
// static 
boost::shared_ptr<MessageHandler> MessageHandlerFactory::getMessageHandler(const MessageType &type)
{
    switch(type)
    {
        case UNKNOWN_MESSAGE_TYPE: 
            boost::shared_ptr<MessageHandler>(); // == NULL
            break;
        case MESSAGE_STATUS_TYPE: 
            return boost::shared_ptr<MessageStatusHandler>(new MessageStatusHandler);
            break;
        case TEST_MESSAGE_TYPE: 
            return boost::shared_ptr<TestMessageHandler>(new TestMessageHandler);
            break;
        case GPS_MESSAGE_TYPE: 
            return boost::shared_ptr<GPSMessageHandler>(new GPSMessageHandler);
            break;
        case LABEL_MESSAGE_TYPE: 
            return boost::shared_ptr<LabelMessageHandler>(new LabelMessageHandler);
            break;
    }

    return boost::shared_ptr<MessageHandler>(); // == NULL
}