#define BOOST_TEST_MODULE testSpeedMessage
#include <boost/test/unit_test.hpp>
#if 0
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>
#endif
#include <sstream>

#include <libwatcher/speedWatcherMessage.h>

#include "logger.h"

using namespace std;
//using namespace watcher;
using namespace watcher::event;
using namespace boost;
//using namespace boost::archive;

BOOST_AUTO_TEST_CASE( ctor_test )
{
    LOAD_LOG_PROPS("log.properties"); 

    /* Ensure the class can be instantiated */
    SpeedMessage m1;

    SpeedMessage m2(42);

    SpeedMessage m3(-98);
}

BOOST_AUTO_TEST_CASE( pack_test )
{
    BOOST_TEST_MESSAGE("serializing shared pointer to base class");
    SpeedMessagePtr msg( new SpeedMessage(42) );
    ostringstream os;
    msg->pack(os);

    BOOST_TEST_MESSAGE("serialized: " << os.str());

    BOOST_TEST_MESSAGE("de-serializing shared pointer to base class");
    istringstream is(os.str());
    MessagePtr newmsg = Message::unpack(is);
    BOOST_REQUIRE(newmsg.get() != 0);

    SpeedMessagePtr pnewmsg = dynamic_pointer_cast<SpeedMessage>(newmsg);
    BOOST_REQUIRE(pnewmsg.get() != 0);

    BOOST_CHECK_EQUAL(*msg, *pnewmsg);
}

#if 0
BOOST_AUTO_TEST_CASE( serialize_test )
{
    SpeedMessage msg(37);
    ostringstream ofs;
    {
        boost::archive::text_oarchive oa(ofs);
        oa << msg;
    }

    istringstream ifs(ofs.str());
    SpeedMessage newmsg;
    {
        boost::archive::text_iarchive ia(ifs);
        ia >> newmsg;
    }

    BOOST_CHECK_EQUAL( msg, newmsg );
}

BOOST_AUTO_TEST_CASE ( shared_archive_test )
{
    SpeedMessagePtr msg(new SpeedMessage(37));
    ostringstream ofs;
    {
        boost::archive::text_oarchive oa(ofs);
        oa << msg;
    }

    istringstream ifs(ofs.str());
    SpeedMessagePtr newmsg;
    {
        boost::archive::text_iarchive ia(ifs);
        ia >> newmsg;
    }

    BOOST_CHECK_EQUAL( *msg, *newmsg );
}

BOOST_AUTO_TEST_CASE ( shared_archive_base_test )
{
    MessagePtr msg(new SpeedMessage(37));
    ostringstream ofs;
    {
        boost::archive::text_oarchive oa(ofs);
        oa << msg;
    }

    istringstream ifs(ofs.str());
    MessagePtr newmsg;
    {
        boost::archive::text_iarchive ia(ifs);
        ia >> newmsg;
    }

    SpeedMessagePtr dmsg = boost::dynamic_pointer_cast<SpeedMessage>(msg);
    BOOST_CHECK_NE( dmsg.get(), static_cast<SpeedMessage*>(0) );
    BOOST_TEST_MESSAGE( "dmsg->speed=" << dmsg->speed );
    SpeedMessagePtr dnewmsg = boost::dynamic_pointer_cast<SpeedMessage>(newmsg);
    BOOST_CHECK_NE( dnewmsg.get(), static_cast<SpeedMessage*>(0) );
    BOOST_TEST_MESSAGE( "dnewmsg->speed=" << dnewmsg->speed );

    BOOST_CHECK_EQUAL( *dmsg, *dnewmsg );

    // GTL - check that the pointers are pointing to correct vtables
    LOG_DEBUG("     *msg: " << *msg); 
    LOG_DEBUG("  *newmsg: " << *newmsg); 
    LOG_DEBUG("    *dmsg: " << *dmsg); 
    LOG_DEBUG(" *dnewmsg: " << *dnewmsg); 

    string dStr("SpeedMessage(speed=37)");  // This is the output of operator<< on derived speedTest
    ostringstream boutput, doutput;
    boutput << *newmsg;
    doutput << *dnewmsg; 

    BOOST_CHECK_NE( dStr, boutput.str() );
    BOOST_CHECK_EQUAL( dStr, doutput.str() );

    LOG_DEBUG("   base: " << boutput.str()); 
    LOG_DEBUG("derived: " << doutput.str()); 
}
#endif