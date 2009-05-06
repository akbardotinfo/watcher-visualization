#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/export.hpp>

#include "testMessage.h"

using namespace std;

namespace watcher {
    namespace event {

        INIT_LOGGER(TestMessage, "Message.TestMessage");

        TestMessage::TestMessage() : 
            Message(TEST_MESSAGE_TYPE, MESSAGE_TEST_VERSION), 
            stringData(),
            intsData()
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }

        TestMessage::TestMessage(const string &str, const vector<int>& ints) :
            Message(TEST_MESSAGE_TYPE, MESSAGE_TEST_VERSION),
            stringData(str),
            intsData(ints)
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }

#if 0
        TestMessage::TestMessage(const TestMessage &other) :
            Message(other.type, other.version), 
            stringData(other.stringData),
            intsData(other.intsData)
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }
#endif

        bool TestMessage::operator==(const TestMessage &other) const
        {
            TRACE_ENTER();

            bool retVal = 
                Message::operator==(other) && 
                stringData == other.stringData &&
                intsData == other.intsData;

            TRACE_EXIT_RET(retVal);
            return retVal;
        }

#if 0
        TestMessage &TestMessage::operator=(const TestMessage &other)
        {
            TRACE_ENTER();

            Message::operator=(other);
            stringData=other.stringData;
            intsData=other.intsData;

            TRACE_EXIT();
            return *this;
        }
#endif

        // virtual 
        std::ostream &TestMessage::toStream(std::ostream &out) const
        {
            TRACE_ENTER();

            Message::toStream(out);
            out << " stringData:\"" << stringData << "\""; 
            out << " intsData:[";
            // Compiler cant' find this: copy(intsData.begin(), intsData.end(), ostream_iterator<int>(out,","));
            for (vector<int>::const_iterator i=intsData.begin(); i != intsData.end(); ++i)
                out << *i << ",";
            out << "] ";

            TRACE_EXIT();
            return out;
        }

        ostream &operator<<(ostream &out, const TestMessage &mess)
        {
            TRACE_ENTER();
            mess.operator<<(out);
            TRACE_EXIT();
            return out;
        }

        template <typename Archive> void TestMessage::serialize(Archive & ar, const unsigned int /* file_version */)
        {
            TRACE_ENTER();
            ar & boost::serialization::base_object<Message>(*this);
            ar & stringData;
            ar & intsData;
            TRACE_EXIT();
        }
    }
}

BOOST_CLASS_EXPORT(watcher::event::TestMessage);
