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

#include <iomanip>
#include <sstream>

#include <boost/lexical_cast.hpp>

#include "dataMarshaller.h"
#include "logger.h"

INIT_LOGGER(watcher::DataMarshaller, "DataMarshaller"); 

using namespace std;
using namespace watcher;
using namespace watcher::event;

#if 0
/**
 * GTL ERASE START
 * preinitailize the archiving mechanism. This is a hack and should be 
 * removed once we figure out how to do this without priming the 
 * archive pump
 */
#include "libwatcher/message.h"
#include "libwatcher/labelMessage.h"
#include "libwatcher/edgeMessage.h"
#include "libwatcher/colorMessage.h"
#include "libwatcher/gpsMessage.h"
#include "libwatcher/messageStatus.h"
#include "libwatcher/dataRequestMessage.h"
#include "libwatcher/testMessage.h"
#include "libwatcher/seekWatcherMessage.h"
#include "libwatcher/startWatcherMessage.h"
#include "libwatcher/stopWatcherMessage.h"
#include "libwatcher/speedWatcherMessage.h"
#include "libwatcher/nodeStatusMessage.h"
#include "libwatcher/connectivityMessage.h"
static MessagePtr unused_A_Ptr=MessagePtr(new LabelMessage); 
static MessagePtr unused_B_Ptr=MessagePtr(new EdgeMessage); 
static MessagePtr unused_C_Ptr=MessagePtr(new ColorMessage); 
static MessagePtr unused_D_Ptr=MessagePtr(new GPSMessage); 
static MessagePtr unused_E_Ptr=MessagePtr(new MessageStatus); 
static MessagePtr unused_F_Ptr=MessagePtr(new DataRequestMessage); 
static MessagePtr unused_G_Ptr=MessagePtr(new TestMessage); 
static MessagePtr unused_H_Ptr=MessagePtr(new SeekMessage); 
static MessagePtr unused_I_Ptr=MessagePtr(new StartMessage); 
static MessagePtr unused_J_Ptr=MessagePtr(new StopMessage); 
static MessagePtr unused_K_Ptr=MessagePtr(new SpeedMessage); 
static MessagePtr unused_L_Ptr=MessagePtr(new NodeStatusMessage); 
static MessagePtr unused_M_Ptr=MessagePtr(new ConnectivityMessage); 
#endif

/** 
 * GTL ERASE END 
 *
 **/

#define MARSHALSHORT(a,b) \
	do\
	{\
		*a++=((unsigned char)((b) >> 8)); \
		*a++=((unsigned char)(b)); \
	} while(0)

#define UNMARSHALSHORT(a,b)  {b=(*(a+0)<<8) | *(a+1); a+=2;}

#define MARSHALINT32(a,b) \
    do\
    {\
	*a++=((unsigned char)((b) >> 24));\
	*a++=((unsigned char)((b) >> 16));\
	*a++=((unsigned char)((b) >> 8)); \
	*a++=((unsigned char)(b)); \
    } while(0)

#define UNMARSHALINT32(a,b)  {b = (*(a+0)<<24) | (*(a+1)<<16) | (*(a+2)<<8) | *(a+3); a+=4;}

// static 
bool DataMarshaller::unmarshalHeader(const char *buffer, const size_t &bufferSize, size_t &payloadSize, unsigned short &messageNum)
{
    TRACE_ENTER();

    // logger is crashing if given anything other than a string! GTL - Test and see if this is still true.
    LOG_DEBUG("Unmarshalling a header of " << boost::lexical_cast<std::string>(bufferSize) << " bytes.");

    if (bufferSize < DataMarshaller::header_length)
    {
        LOG_ERROR("Not enought data in payload to unmarshal the packet or packet is corrupt"); 
        TRACE_EXIT_RET("false");
        return false;
    }

    if (bufferSize > 0xffffffff)
    {
	LOG_ERROR("buffer size " << bufferSize << " is too large to fit into an uint32.");
        TRACE_EXIT_RET("false");
        return false;
    }

    const unsigned char *bufPtr=(const unsigned char*)buffer; 
    UNMARSHALINT32(bufPtr, payloadSize);
    UNMARSHALSHORT(bufPtr, messageNum);

    LOG_DEBUG("Header data: payload size: " << payloadSize << " messageNum: " << messageNum); 

    return true;
}

//static 
bool DataMarshaller::unmarshalPayload(MessagePtr &message, const char *buffer, const size_t &bufferSize)
{
    TRACE_ENTER(); 

    // Extract the Message from the payload
    istringstream s(string(buffer, bufferSize));
    message=Message::unpack(s);

    //LOG_DEBUG("Unmarshalled payload data: " << s.str()); 

    TRACE_EXIT_RET((message?"true":"false"));
    return static_cast<bool>(message);          // cast may be redundant 
}

//static
bool DataMarshaller::unmarshalPayload(std::vector<MessagePtr> &messages, unsigned short &numOfMessages, const char *buffer, const size_t &bufferSize)
{
    TRACE_ENTER();

    istringstream is(string(buffer, bufferSize)); 
    LOG_DEBUG("Unmarshalling payload data(size=" << bufferSize << "): " << is.str()); 
    unsigned short i=0;
    for(i=0; i<numOfMessages; i++)
    {
        MessagePtr m=Message::unpack(is);
        if(!m)
        {
            LOG_WARN("Error: Unable to unmarshal message number " << i+1 << " in the packet payload."); 
            numOfMessages=i;
            TRACE_EXIT_RET("false"); 
            return false;
        }
        messages.push_back(m);      // marshalled with push_front, so should get same order on unmarshalling.
    }

    //LOG_DEBUG("Unmarshalled payload data: " << is.str()); 
    LOG_DEBUG("Successfully unmarshalled " << i << " message" << (i>0?"s":"")); 

    TRACE_EXIT_RET("true"); 
    return true;
}

//static 
bool DataMarshaller::marshalPayload(const MessagePtr &message, NetworkMarshalBuffers &outBuffers)
{ 
    TRACE_ENTER();

    std::vector<MessagePtr> messVec;
    messVec.push_back(message);
    bool retVal=marshalPayload(messVec, outBuffers);

    TRACE_EXIT_RET((retVal?"true":"false")); 
    return retVal;
}

//static 
bool DataMarshaller::marshalPayload(const vector<MessagePtr> &messages, NetworkMarshalBuffers &outBuffers)
{
    TRACE_ENTER();

    // Add serialized Messages first, then prepend the header once we know 
    // how large the payload will be.
    uint32_t payloadSize=0;
    unsigned short messageNum=messages.size(); 

    // Putting each Message in a separate buffer may speed up sent/recv as
    // each buffer can be scatter-gather sent/recv'd.
    
    ostringstream os;
    std::string s(os.str());
    for(vector<MessagePtr>::const_iterator m=messages.begin(); m!=messages.end(); ++m)
    {
        (*m)->pack(os); 
        s = os.str();
        payloadSize += static_cast<uint32_t>(s.size()); 
        outBuffers.push_back(NetworkMarshalBuffer(s)); 
        LOG_DEBUG("Marshalled payload: " << s); 
        os.str(""); 
    }

    LOG_DEBUG("Serialized " << payloadSize << " bytes of message data from " << messageNum << " message" << (messageNum>1?"s":"")); 

    // Format the header.
    // GTL - may be nice to put the header itself into a class that supports archive/serialization...
    unsigned char header[header_length];
    unsigned char *bufPtr=header; 
    MARSHALINT32(bufPtr, payloadSize);
    MARSHALSHORT(bufPtr, messageNum);

    // Put the header on the front.
    outBuffers.push_front(NetworkMarshalBuffer(string((const char*)header, sizeof(header))));

    TRACE_EXIT_RET("true");
    return true;
}


