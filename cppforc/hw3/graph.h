#ifndef GRAPH_H__
#define GRAPH_H__

//#define DEBUG_MST

#include <cstdlib>
#include <ctime>

#include <vector>
#include <list>
#include <map>
#include <queue>
#include <iostream>
#include <utility>
#include <sstream>
#include <fstream>
#include <numeric>
#include <functional>
#include <algorithm>
#include <iterator>


/* Simple templated edge representation */
template <typename NodeLabelType, typename CostType>
struct Edge
{
    Edge(NodeLabelType from, NodeLabelType to, CostType cost) :
        from(from), to(to), cost(cost) {
    }

    NodeLabelType from;
    NodeLabelType to;
    CostType cost;
};

/* And an intrinsic < operator to go with a std::priority_queue */
template <typename NodeLabelType, typename CostType>
bool operator < (const Edge<NodeLabelType, CostType>& e1,
                 const Edge<NodeLabelType, CostType>& e2)
{
    /* Trick it to reverse order in container */
    return e1.cost > e2.cost;
}


/* A list of edges for the spanning tree calculation */
template <typename NodeLabelType, typename CostType>
class EdgeSet: public std::vector< Edge<NodeLabelType, CostType> >
{
public:
    typedef typename std::vector< Edge<NodeLabelType, CostType> >::const_iterator
        const_iterator;

    CostType getCost() const {
        CostType cost = static_cast<CostType>(0);
        for (const_iterator edgeIt = this->begin();
                edgeIt != this->end(); ++edgeIt) {
            cost += edgeIt->cost;
        }
        return cost;
    }

    void dump() const {
        for (const_iterator edgeIt = this->begin();
                edgeIt != this->end(); ++edgeIt) {
            std::cout << edgeIt->from << "->" << edgeIt->to <<
                "(" << edgeIt->cost << ")" << " ";
        }
        std::cout << std::endl << "Total Cost: " <<
            getCost() << std::endl;
    }

};



/*
 * Templated class for an undirected graph. Represented as
 * a full blown adjacency matrix augmented with a labeling
 * correspondence structure (2 maps for direct/indirect
 * label to nodes mapping).
 */
template <typename NodeLabelType, typename CostType>
class Graph
{
private:
    typedef unsigned long node_type;
    typedef std::map<NodeLabelType, node_type> label2node_type;
    typedef std::map<node_type, NodeLabelType> node2label_type;
    typedef std::vector< std::vector<CostType> > adjacency_type;
    typedef typename std::vector<CostType>::size_type size_type;

    /*
     * Simple functor used for emulating unions in
     * Kruskal's algorithm implementation.
     */
    class Replacer {
    public:
        Replacer(node_type from, node_type to) :
            from(from), to(to) {
        }

        node_type operator () (node_type value) {
            if (value == from) {
                return to;
            }

            return value;
        }

    private:
        node_type from;
        node_type to;
    };


public:
    /* Default constructor. Builds an empty graph. */
    Graph();

    /*
     * Random graph constructor.
     * Uses 2 sepparate generators for node labels & cost values.
     */
    template<typename NodeGenerator, typename CostGenerator>
        Graph(int v, float density, NodeGenerator nodeGen, CostGenerator costGen)
        {
            for (int i = 0; i < v; i++) {
                addNode(nodeGen());
            }

            for (node_type n1 = 0; n1 < nodes.size(); n1++) {
                for (node_type n2 = n1 + 1; n2 < nodes.size(); n2++) {
                    float decision = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
                    if (decision < density) {
                        addEdge(n1, n2, costGen());
                    }
                }
            }
        }

    /* Read it all from a file ... */
    explicit Graph(std::ifstream& f);

    ~Graph();

    /* Adds a node with the given label to the graph */
    void addNode(const NodeLabelType& node);

    /* Adds an edge with the given cost between the two referred nodes */
    void addEdge(const NodeLabelType& from, const NodeLabelType& to, CostType cost);

    /* Returns true wether a node with the given label exists in the graph. */
    bool hasNode(const NodeLabelType& node) const;

    /* Returns true wether an edge between the two nodes exists in the graph. */
    bool hasEdge(const NodeLabelType& from, const NodeLabelType& to);

    /* Debugging purposes. This could as well have been an overloaded << operator */
    void dump();

    /* Returns true wether a minimum spanning tree exists. Computes such a tree in tree. */
    bool span(EdgeSet<NodeLabelType, CostType>& tree);

private:
    node2label_type labels; // nodes to labels correspondence
    label2node_type nodes; // labels to nodes indexes correspondence
    adjacency_type adjs; // the actual adjacenty matrix
};


template<typename NodeLabelType, typename CostType>
Graph<NodeLabelType, CostType>::Graph()
{
}

template<typename NodeLabelType, typename CostType>
Graph<NodeLabelType, CostType>::Graph(std::ifstream& f)
{
    unsigned long numNodes;
    std::string line;
    std::stringstream sstream;

    if (!f.is_open()) {
        std::cerr << "File not opened!" << std::endl;
        return;
    }

    f >> numNodes;

    /* Forced by the genericity of the class to add nodes one by one */
    for (unsigned long i = 0; i < numNodes; i++) {
        addNode(static_cast<NodeLabelType>(i));
    }

    NodeLabelType nl1, nl2;
    CostType cost;

    /* Stream the file in line by line */
    while (getline(f, line)) {
        sstream.clear();
        sstream << line;
        sstream >> nl1 >> nl2 >> cost;
        addEdge(nl1, nl2, cost);
    }

    /* Also do a little check here ... */
    if (nodes.size() != numNodes) {
        std::cerr << "WARNING: Expected no. of nodes: " << numNodes <<
            " but found: " << nodes.size() << std::endl;
        std::cerr << "Graph may be incomplete." << std::endl;
        return;
    }

}

template<typename NodeLabelType, typename CostType>
Graph<NodeLabelType, CostType>::~Graph()
{
}

template<typename NodeLabelType, typename CostType>
void Graph<NodeLabelType, CostType>::addNode(const NodeLabelType& node)
{
    // First translate the label no a node index ...
    node_type index = static_cast<node_type>(nodes.size());
    nodes.insert(std::make_pair(node, index));
    labels.insert(std::make_pair(index, node));

    // Then take care of node insertion
    for (typename adjacency_type::iterator adjsIt  = adjs.begin();
            adjsIt != adjs.end(); ++adjsIt) {
        adjsIt->push_back(static_cast<CostType>(0));
    }

    std::vector<CostType> adj(nodes.size(), static_cast<CostType>(0));
    adjs.push_back(adj);
}

template<typename NodeLabelType, typename CostType>
void Graph<NodeLabelType, CostType>::addEdge(const NodeLabelType& from,
                                             const NodeLabelType& to,
                                             CostType value)
{
    //First check the two nodes are there ...
    if (nodes.find(from) == nodes.end() ||
        nodes.find(to) == nodes.end()) {
        return;
    }

    node_type fromNode = nodes[from];
    node_type toNode = nodes[to];

    //Then insert the new edge
    adjs[fromNode][toNode] = adjs[toNode][fromNode] = value;
}

template<typename NodeLabelType, typename CostType>
bool Graph<NodeLabelType, CostType>::hasNode(const NodeLabelType& node) const
{
    return (nodes.find(node) != nodes.end());
}

template<typename NodeLabelType, typename CostType>
bool Graph<NodeLabelType, CostType>::hasEdge(const NodeLabelType& from,
                                             const NodeLabelType& to)
{
    if (!hasNode(from) || !hasNode(to)) {
        return false;
    }

    node_type fromNode = nodes[from];
    node_type toNode = nodes[to];

    return (adjs[fromNode][toNode] != static_cast<CostType>(0));
}


template<typename NodeLabelType, typename CostType>
void Graph<NodeLabelType, CostType>::dump()
{
    std::cout << "Nodes: ";

    for (typename node2label_type::const_iterator lIt = labels.begin();
            lIt != labels.end(); lIt++) {
        std::cout << lIt->second << " ";
    }

    std::cout << std::endl << "Adjacencies: " << std::endl;

    for (size_type i = 0; i < nodes.size(); i++) {
        std::cout << labels[i] << " : ";
        for (size_type j = 0; j < nodes.size(); j++) {
            if (adjs[i][j] != static_cast<CostType>(0)) {
                std::cout << labels[j] << "/" << adjs[i][j] << " ";
            }
        }
        std::cout << std::endl;
    }
}

template<typename NodeLabelType, typename CostType>
bool Graph<NodeLabelType, CostType>::span(EdgeSet<NodeLabelType,
                                          CostType>& tree)
{
    // Keep the Edge in internal representation for computations but
    // return it in user-friendly format (labeled nodes)
    std::priority_queue< Edge<node_type, CostType> > q;
    std::vector<node_type> groups(nodes.size());

    for (node_type i = 0; i < nodes.size(); i++) {
        groups[i] = i; /* Make each node it's own group */
        for (node_type j = i + 1; j < nodes.size(); j++) {
            if (adjs[i][j] != static_cast<CostType>(0)) {
                q.push(Edge<node_type, CostType>(i, j, adjs[i][j]));
            }
        }
    }

    while (!q.empty() && tree.size() + 1 < nodes.size()) {
        Edge<node_type, CostType> edge = q.top();
        q.pop();

#ifdef DEBUG_MST
        std::cout << "Adding edge: " << edge.from <<
            "->" << edge.to << std::endl;
#endif
        // Expand the tree, then unite the two groups ...
        if (groups[edge.from] != groups[edge.to]) {
            tree.push_back(Edge<NodeLabelType, CostType>(labels[edge.from],
                                                         labels[edge.to],
                                                         edge.cost));

            std::transform(groups.begin(), groups.end(), groups.begin(),
                    Replacer(groups[edge.from], groups[edge.to]));

#ifdef DEBUG_MST
            std::copy(groups.begin(), groups.end(),
                    std::ostream_iterator<node_type>(std::cout, " "));
            std::cout << std::endl;
#endif
        }
    }

    if (tree.size() + 1 == nodes.size()) {
        return true;
    }

    //Looks like the graph is not connected!
    return false;
}

#endif //GRAPH_H__

