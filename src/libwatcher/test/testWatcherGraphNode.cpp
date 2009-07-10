#define BOOST_TEST_MODULE watcher::watcherGraphNode test
#include <boost/test/unit_test.hpp>

#include "logger.h"
#include "../watcherGraphNode.h"

using namespace std;
using namespace boost;
using namespace watcher;
using namespace watcher::event;
using namespace boost::unit_test_framework;

BOOST_AUTO_TEST_CASE( ctor_test )
{
    // Do this in first test so we can log.
    LOAD_LOG_PROPS("test.log.properties"); 

    WatcherGraphNode wgn;
}

BOOST_AUTO_TEST_CASE( output_test )
{
    LOG_INFO("Checking Graph Node output operator,"); 

    WatcherGraphNode wgn;

    cout << "Empty Node:" << endl << wgn << endl;

    wgn.nodeId.from_string("192.168.1.123");
    wgn.gpsData=GPSMessagePtr(new GPSMessage(0.1234, 0.2345, 0.3456));
    wgn.displayInfo->label="This is a label there are others like it, but this one is mine."; 
    wgn.connected=true;

    // Create from label message
    LabelMessagePtr lmp(new LabelMessage("Hello"));
    lmp->layer="twilight zone";
    LabelDisplayInfoPtr ldip(new LabelDisplayInfo); 
    ldip->loadConfiguration(lmp); 
    wgn.labels.push_back(ldip);

    // create from label display info
    ldip.reset(new LabelDisplayInfo);
    ldip->layer="chuck e. cheeze";
    ldip->labelText="From the FUN zone!"; 
    wgn.labels.push_back(ldip);

    cout << "Node with stuff in it: " << endl << wgn << endl;
}


