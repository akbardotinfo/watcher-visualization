#ifndef WATCHER_GRAPH_H_WHAT_DO_VEGAN_ZOMBIES_EAT_____GRAINS__GRAINS
#define WATCHER_GRAPH_H_WHAT_DO_VEGAN_ZOMBIES_EAT_____GRAINS__GRAINS

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>

#include "watcherGraphNode.h"
#include "watcherGraphEdge.h"

// GTL - How do I hide the message types from clients of WatcherGraph. I tried ME's 'class impl'
// trick, but it did not work. It's very possible that I did not do it correctly. 
#include "connectivityMessage.h"
#include "gpsMessage.h"
#include "nodeStatusMessage.h"
#include "edgeMessage.h"

namespace watcher
{
    using namespace event;      // for libwatcher and messages. 
    class MessageStreamFilter;  // 

    // Forward decl for Ptr typedef
    class WatcherGraph;
    typedef boost::shared_ptr<WatcherGraph> WatcherGraphPtr; 

    /** 
     * @class WatcherGraph
     * @author Geoff Lawler <geoff.lawler@sparta.com>
     * @date 2009-05-15
     *
     * WatcherGraph keeps track of the evolving topologies described by a series of watcher messages. 
     * i.e. you feed it watcher messages that have anything to do with edges and nodes, and it will
     * keep track of the state of the topologial graph represented by that stream of messages. i.e. you
     * throw a bunch of NodeStatus connect/disconnect, edgeMessage, and connectivityMessage messages
     * at it and it keeps track of eveything for you. It exports the current topology as a boost::graph.
     *
     * Currently this class just tracks nodes, locations, and connectivities. It may add labels, etc
     * if it turns out to be a good place to put that stuff.
     *
     */
    class WatcherGraph 
    {
        public:

            /** 
             * WatcherGraph
             */
            WatcherGraph();

            /**
             * On a large enough time line, the survival rate for everyone drops to zero.
             */
            virtual ~WatcherGraph();

            /**
             * The graph is a boost directed adjacency_list. This interface is public
             * so GUI developers can get direct access if needed. 
             */
            typedef boost::adjacency_list<
                boost::vecS, 
                boost::vecS, 
                boost::directedS,
                WatcherGraphNode, 
                WatcherGraphEdge> Graph;

            typedef boost::graph_traits<Graph>::vertex_descriptor  vertex;
            typedef boost::graph_traits<Graph>::vertices_size_type vertexInt;
            typedef boost::graph_traits<Graph>::vertex_iterator    vertexIterator;
            typedef boost::graph_traits<Graph>::edge_descriptor    edge;
            typedef boost::graph_traits<Graph>::edges_size_type    edgeInt;
            typedef boost::graph_traits<Graph>::edge_iterator      edgeIterator;

            /**
             * The actual boost::graph.
             */
            Graph theGraph;


            /**
             * updateGraph()
             * updateGraph takes a message, and if it the message applicable to the state of the 
             * watcherGraph, will update the internal graph state and execute the graphUpdated
             * callback (if it exists).
             * 
             * @param MessagePtr - the newly arrived data
             * @return bool - if the graph was updated then true, else false.
             */
            bool updateGraph(const MessagePtr &message);

            /**
             * updateGraph(filter)
             * Applies the passed filter to the in-memory graph. Read details below. 
             *
             * When a filter is applied to the MessageStream feeding the graph, 
             * it should also be applied to the graph instance, otherwise old data that does
             * not match the new stream may still be in the in-memory graph.
             *
             * Note: that there are problems with filtering by message type for in-memory graphs.
             * The graphs are built from incoming message, but the message themselves may not be
             * kept. The message type->graph may be a one way operation: how do we know that an edge
             * was created with a edgeMessage or a connectivityMessage? So filtering by message type 
             * is not supported. 
             *
             * Filtering by layer works, with certain understandings. Unless it is a base layer (like 
             * the physical layer), a layer sits on top of another layer. When a layer is removed, 
             * all layers above that layer are removed as well. For instance if a layer that contains
             * an edge is removed, all labels attached to that edge regardless of layer, are removed 
             * as well. This may not be what you expect. The problem is what location do you draw a label
             * at that is attached to a now non-existant edge? 
             *
             * The best approach for GUI developers may be to just clear this graph, apply the filter
             * to the message stream, and let the filtered-message stream rebuild the graph. This may not
             * work well for region filters though if the region is being filtered/updated more often 
             * than the GPS messages are recieved. 
             *
             * It's a whole thing, you see? 
             *
             */
             bool updateGraph(const MessageStreamFilter &filter); 

             /**
              * Update Graph internals (component experations, etc). 
              * Should be called periodically.
              */
            void doMaintanence();

            /** Find a node in the graph based on a NodeIdentifier 
             * @param[in] id - the id of the node you want to find. 
             * @param[out] retIter - an iterator that points to the found node.
             * @return bool - true if successful, false otherwise
             */
            bool findNode(const NodeIdentifier &id, boost::graph_traits<Graph>::vertex_iterator &retVal);

            /**
             * Write an instance of this class as a human readable stream to the otream given
             */
            virtual std::ostream &toStream(std::ostream &out) const;

            /**
             * Write an instance of this class as a human readable stream to the otream given.
             */
            std::ostream &operator<<(std::ostream &out) const { return toStream(out); }

            /** 
             * Serialize the Graph to a stream
             */
             // bool pack(std::ostream &os);

             /**
              * Unserialize a Graph from a stream
              */
             // WatcherGraphPtr unpack(std::istream& is);

        protected:

        private:

            // friend class boost::serialization::access;
            // template <typename Archive> void serialize(Archive & ar, const unsigned int /* file_version */);

            DECLARE_LOGGER();

            /**
             * Update the graph with a list of neighbors addes or removed.
             */
            bool addNodeNeighbors(const ConnectivityMessagePtr &message);

            /**
             * Add or remove a single edge in the graph.
             */
            bool addEdge(const EdgeMessagePtr &message);

            /**
             * Update a node's location.
             */
            bool updateNodeLocation(const GPSMessagePtr &message);

            /**
             * Update a node's state (connected or disconnected).
             */
            bool updateNodeStatus(const NodeStatusMessagePtr &message);

            /**
             * Update a node's color.
             */
            bool updateNodeColor(const ColorMessagePtr &message);

            /**
             * Update an attached label - either add or remove it.
             */
            bool addRemoveAttachedLabel(const LabelMessagePtr &message);

            /** Find a node, if it doesn't exist, create it. Returns iterator to the node 
             * @param[in] id - the id of the node you want to find/create.
             * @param[out] retIter - an iterator that points to the found node.
             * @param[in] layer - if the node is created, load this layers display information into it. 
             * @return bool - true if successful, false otherwise
             */
            bool findOrCreateNode(const NodeIdentifier &id, boost::graph_traits<Graph>::vertex_iterator &retIter, const GUILayer &layer);

            /** Create a node and return an iterator to it. 
             * @param[in] id - the id of the node you want to create. 
             * @param[out] retIter - an iterator that points to the found node.
             * @return bool - true if successful, false otherwise
             */
            bool createNode(const NodeIdentifier &id, boost::graph_traits<Graph>::vertex_iterator &retIter);

    }; // like a fired school teacher.

    /** typedef a shared pointer to this class
    */
    typedef boost::shared_ptr<WatcherGraph> WatcherGraphPtr;

    /** write a human readable version of the WatcherGraph class to the ostream given
    */
    std::ostream &operator<<(std::ostream &out, const WatcherGraph &watcherGraph);

}

#endif // WATCHER_GRAPH_H_WHAT_DO_VEGAN_ZOMBIES_EAT_____GRAINS__GRAINS
