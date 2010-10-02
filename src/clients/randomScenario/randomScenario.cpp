#include <iostream>

#include <libwatcher/client.h>
#include <libwatcher/labelMessage.h>
#include <libwatcher/edgeMessage.h>
#include <libwatcher/gpsMessage.h>
#include <libwatcher/colorMessage.h>
#include <libwatcher/colors.h>
#include <libwatcher/watcherColors.h>
#include <libwatcher/connectivityMessage.h>
#include <libwatcher/sendMessageHandler.h>

#include <logger.h>

#include "configuration.h"

using namespace std;
using namespace watcher;
using namespace watcher::event;
namespace po=boost::program_options;
namespace rs=randomScenario;

typedef struct {
    double x, y, z;
    double speed;
    double theta;
    double phi;
} NodePos;

void computeEdges(unsigned int *edges, const NodePos *nodes, const unsigned int nodeNum);
void doMobility(NodePos *nodes, const unsigned int nodeNum);

static bool debug=false;
static unsigned int maxWidth=0;
static unsigned int maxHeight=0;
static unsigned int maxDepth=0; 
static unsigned int radius=0;

int main(int argc, char **argv) 
{
    // Load config and bail on error. 
    if (!rs::loadConfiguration(argc, argv))
        return EXIT_FAILURE;

    rs::printConfiguration(cout); 

    srand(time(NULL));

    po::variables_map &config=rs::getConfig();
    unsigned int nodeNum=config["nodeNum"].as<unsigned int>(); 
    float nodeDegreePercentage=config["nodeDegreePercentage"].as<float>();
    float nodeLabelPercentage=config["nodeLabelPercentage"].as<float>();
    unsigned int layerNum=config["layerNum"].as<unsigned int>();
    int duration=config["duration"].as<int>(); 
    string server=config["server"].as<string>(); 
    maxWidth=config["width"].as<unsigned int>();
    maxHeight=config["height"].as<unsigned int>();
    maxDepth=config["depth"].as<unsigned int>();
    radius=config["radius"].as<unsigned int>();
    debug=config["debug"].as<bool>();

    const int maxSpeed=10;
    const int minSpeed=2;

    LOAD_LOG_PROPS(config["logproperties"].as<string>());

    cout << "Connecting to watcherd on " << server << endl;
    watcher::Client client(server); 
    client.addMessageHandler(MultipleMessageHandler::create());

    NodePos *positions=new NodePos[nodeNum];
    if (!positions) {
        cerr << "Unable to allocate memory to hold " << nodeNum << " nodes." << endl;
        return EXIT_FAILURE;
    }

    // init positions
    for (int i=0; i<nodeNum; i++) {
        positions[i].x=rand()%maxWidth; 
        positions[i].y=rand()%maxHeight; 
        positions[i].z=rand()%maxDepth; 
        positions[i].speed=(rand()%(maxSpeed-minSpeed))+minSpeed; 
        positions[i].theta=((rand()%(((int)M_PI*100)*2))-((int)M_PI*100))/100.0;     // random # between -PI..PI w/2 sig digits
        positions[i].phi=((rand()%(((int)M_PI*50)*2))-((int)M_PI*50))/100.0;         // random # between -PI/2..PI/2 w/2 sig digits

        cout << "Placed node " << i+1 << " at intial position " << positions[i].x << ", " << positions[i].y << ", " << positions[i].z;
        cout << " at speed " << positions[i].speed << " and direction (" << positions[i].theta << ", " << positions[i].phi << ")" << endl;
    }

    unsigned int *edges=new unsigned int[nodeNum*nodeNum];
    memset(edges, 0, nodeNum*nodeNum); 

    while (duration) { 

        doMobility(positions, nodeNum);
        computeEdges(edges, positions, nodeNum); 

        GPSMessagePtr gpsMess(GPSMessagePtr(new GPSMessage)); 
        ConnectivityMessagePtr connMess(ConnectivityMessagePtr(new ConnectivityMessage));

        for (int i=0; i<nodeNum; i++) {
            NodeIdentifier nid=boost::asio::ip::address_v4::address_v4(i+1);
            gpsMess->x=positions[i].x;
            gpsMess->y=positions[i].y;
            gpsMess->z=positions[i].z; 
            gpsMess->fromNodeID=nid;
            if (!client.sendMessage(gpsMess)) 
                cerr << "Error sending gps message #" << i << endl;

            if (rand()%100 <= (unsigned int)nodeLabelPercentage) { 
                LabelMessagePtr lm(new LabelMessage("Label", nid)); 
                if (!client.sendMessage(lm)) 
                    cerr << "Error sending node label message"; 
            }

            // All layers have the same edges. How to fix this? radius per layer maybe?
            for (unsigned int l=0; l<layerNum; l++) {
                connMess->fromNodeID=nid;
                connMess->layer="ConnectivityMessages_" + boost::lexical_cast<string>(l);
                for (int j=0; j<nodeNum; j++)  
                    if (*(edges+(i*nodeNum)+j)) 
                        connMess->neighbors.push_back(boost::asio::ip::address_v4::address_v4(j+1)); 
                if (debug) {
                    cout << "Nbrs of node " << i+1 << ": ";
                    for (ConnectivityMessage::NeighborList::const_iterator nbr=connMess->neighbors.begin(); nbr!=connMess->neighbors.end(); nbr++)
                        cout << nbr->to_string() << " ";
                    cout << endl;
                }
                if (!client.sendMessage(connMess)) 
                    cerr << "Error sending connectivity message." << endl;
                connMess->neighbors.clear();
            }
        }

        if (duration>0)
            duration--;

        sleep(1);
    }

    client.wait();
    client.close();

    return EXIT_SUCCESS;
}

int dumpEdges(unsigned int *edges, const unsigned int nodeNum) 
{
    cout << "Edges:" << endl;
    for (int i=0; i<nodeNum; i++) {
        for (int j=0; j<nodeNum; j++) 
            cout << *(edges+(i*nodeNum)+j) << " ";
        cout << endl;
    }
}

double computeDistance(const NodePos &n1, const NodePos &n2) 
{
    return sqrt( ((n2.x-n1.x)*(n2.x-n1.x)) + ((n2.y-n1.y)*(n2.y-n1.y)) + ((n2.z-n1.z)*(n2.z-n1.z)) );
}

void computeEdges(unsigned int *edges, const NodePos *nodes, const unsigned int nodeNum)
{
    for (int i=0; i<nodeNum; i++) 
        for (int j=0; j<nodeNum; j++) {
            if (i==j)
                continue;
            double dist=computeDistance(nodes[i], nodes[j]); 
            // if (debug)
            //     cout << "Distance between " << i+1 << " <--> " << j+1 << " is " << dist << endl;
            *(edges+(i*nodeNum)+j)=(radius>=dist)?1:0;
        }
    if (debug)
        dumpEdges(edges, nodeNum); 
}

void doMobility(NodePos *nodes, const unsigned int nodeNum)
{
    if (debug) 
        cout << "node 1 old position: " << nodes[0].x << ", " << nodes[0].y << ", " << nodes[0].z << endl;

    for (int i=0; i<nodeNum; i++) {
        nodes[i].x+=cos(nodes[i].theta)*nodes[i].speed;
        nodes[i].y+=sin(nodes[i].theta)*nodes[i].speed;
        nodes[i].z+=sin(nodes[i].phi)*nodes[i].speed;

        nodes[i].x=nodes[i].x>maxWidth ?nodes[i].x-maxWidth :nodes[i].x<0? maxWidth-nodes[i].x:nodes[i].x;
        nodes[i].y=nodes[i].y>maxHeight?nodes[i].y-maxHeight:nodes[i].y<0?maxHeight-nodes[i].y:nodes[i].y;
        nodes[i].z=nodes[i].z>maxDepth ?nodes[i].z-maxDepth :nodes[i].z<0? maxDepth-nodes[i].z:nodes[i].z;
    }
    if (debug) 
        cout << "node 1 new position: " << nodes[0].x << ", " << nodes[0].y << ", " << nodes[0].z << endl;
}
