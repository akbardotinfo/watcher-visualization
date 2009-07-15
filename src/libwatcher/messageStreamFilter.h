/**
 * @file messageStreamFilter.h
 * @author Geoff Lawler <geoff.lawler@cobham.com> 
 * @date 2009-07-15
 */
#ifndef THIS_ONE_TIME_AT_BAND_CAMP____H
#define THIS_ONE_TIME_AT_BAND_CAMP____H

#include <ostream>
#include <boost/shared_ptr.hpp>
#include "watcherRegion.h"
#include "libwatcher/messageTypesAndVersions.h"  // for GUILayer
#include "logger.h"

namespace watcher
{
    using namespace event; 
    
    /** 
     * @class MessageStreamFilter
     * @author Geoff Lawler <geoff.lawler@sparta.com>
     * @date 2009-04-03
     */
    class MessageStreamFilter
    {
        public:
            /**
             * MessageStreamFilter
             * Create a filter which filters a message stream.
             */
            MessageStreamFilter();

            /**
             * Death to all humans!
             */
            virtual ~MessageStreamFilter();

            /**
             m getLayer()
             * @return Returns the current layer of this filter
             */
            GUILayer getLayer() const; 

            /**
             * setLayer()
             * @param layer - set the layer of this filter to be the value passed in.
             */
            void setLayer(const GUILayer &layer); 

            /**
             * getMessageType()
             * @return Returns the current message type of this filter
             */
            unsigned int getMessageType() const; 

            /**
             * setMessageType()
             * @param type - set the messageType of this filter to be the value passed in.
             */
            void setMessageType(const unsigned int &type); 

            /**
             * getRegion()
             * @return Returns the current region of this filter
             */
            WatcherRegion getRegion() const; 

            /**
             * setRegion()
             * @param region - set the region of this filter to be the value passed in.
             */
            void setRegion(const WatcherRegion &layer); 

            /**
             * Write an instance of this class as a human readable stream to the otream given
             */
            virtual std::ostream &toStream(std::ostream &out) const;

            /**
             * Write an instance of this class as a human readable stream to the otream given.
             * Just calls MessageStream::toStream().
             */
            std::ostream &operator<<(std::ostream &out) const { return toStream(out); }

        protected:

            // noop

        private:
            DECLARE_LOGGER();

            std::string layer;
            unsigned int messageType;  
            WatcherRegion region;

    }; // like a graduating senior

    /**
     * @typedef a MessageStream shared pointer type
     */
    typedef boost::shared_ptr<MessageStreamFilter> MessageStreamFilterPtr;

    /** write a human readable version of the MessageStreamFilter class to the ostream given
     */
    std::ostream &operator<<(std::ostream &out, const MessageStreamFilter &messStreamFilter);

}

#endif //  THIS_ONE_TIME_AT_BAND_CAMP____H
