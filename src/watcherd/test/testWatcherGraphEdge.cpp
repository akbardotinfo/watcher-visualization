#define BOOST_TEST_MODULE watcher::watcherGraphEdge test
#include <boost/test/unit_test.hpp>

#include "logger.h"
#include "../watcherGraphEdge.h"

using namespace std;
using namespace boost;
using namespace watcher;
using namespace watcher::event;
using namespace boost::unit_test_framework;

BOOST_AUTO_TEST_CASE( ctor_test )
{
    // Do this in first test so we can log.
    LOAD_LOG_PROPS("test.log.properties"); 

    WatcherGraphEdge wge;
}

BOOST_AUTO_TEST_CASE( output_test )
{
    LOG_INFO("Checking Graph Edge output operator,"); 

    WatcherGraphEdge wge;

    cout << "Empty Edge:" << endl << wge << endl;

    wge.label="This is an edge. There are others like it, but this on is mine."; 
    wge.color=Color::yellow;
    wge.expiration=10000;
    wge.width=12.232412;

    struct 
    {
        char *label;
        char *layer;
    } data [] = {
        { "Hello" , "hello layer" }, 
        { "World" , "World layer" }, 
        { "doodoodoodoo doodoodoodoo" , "twilight zone" }, 
        { "Coffee Cups" , "Bed, Bath, and Beyond" }
    };

    for(unsigned int i=0; i< (sizeof(data)/sizeof(data[0])); i++)
    {
        LabelMessagePtr lmp(new LabelMessage(data[i].label));
        lmp->layer=data[i].layer;
        wge.attachedLabels.push_back(lmp);
    }

    cout << "Edge with stuff in it: " << wge << endl;
}


