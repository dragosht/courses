#ifndef GRAPH_H__
#define GRAPH_H__

#include <cassert>
#include <cstdlib>

#include <vector>
#include <map>
#include <set>
#include <queue>
#include <iostream>


/*
 * A simple templated structure to keep the edges inside
 * a graph. Since the adjacency structure already indexes
 * the edge lists on a node only the "destination node"
 * is kept in this structure to avoid redundant data.
 */

template<typename NodeType, typename CostType>
struct NodeCost {
    NodeType node;
    CostType cost;

    NodeCost(const NodeType& node, CostType cost) :
        node(node), cost(cost) {}
};


template<typename NodeType, typename CostType>
struct NodeComparator {
    bool operator () (const NodeCost<NodeType, CostType>& n1,
                  const NodeCost<NodeType, CostType>& n2) const {
        return n1.node < n2.node;
    }
};

template<typename NodeType, typename CostType>
struct CostComparator {
    bool operator () (const NodeCost<NodeType, CostType>& n1,
                  const NodeCost<NodeType, CostType>& n2) const {
        return n1.cost < n2.cost;
    }
};



template <typename NodeType, typename EdgeCostType>
class Path : public std::vector<NodeType>
{
public:
    Path() {}
    ~Path() {};

    void setCost(EdgeCostType cost) {
        this->cost = cost;
    }

    EdgeCostType getCost() const {
        return cost;
    }

private:
    EdgeCostType cost;
};


/*
 * The main graph class.
 */
template <typename NodeType, typename EdgeCostType>
class Graph
{
public:
    typedef NodeCost<NodeType, EdgeCostType> EdgeType;
    typedef Path<NodeType, EdgeCostType> PathType;

    typedef std::set<EdgeType, NodeComparator<NodeType, EdgeCostType> > EdgeSetType;
    typedef std::map<NodeType, EdgeSetType> AdjacencyMapType;


    typedef typename std::map<NodeType, EdgeSetType>::size_type size_type;


    /* Creates an empty graph */
    Graph();
    ~Graph();

    void addNode(const NodeType& node);
    void addEdge(const NodeType& source, const NodeType& dest, EdgeCostType cost);

    bool hasNode(const NodeType& node) const;
    bool hasEdge(const NodeType& n1, const NodeType& n2) const;

    EdgeCostType getCost(const NodeType& n1, const NodeType& n2) const;

    std::vector<NodeType> neighbors(const NodeType& node) const;

    void dump() const;

    bool path(const NodeType& from, const NodeType& to, PathType& path) const;

private:
    /* Some sort of an adjancency list only a little bit more generic */
    AdjacencyMapType adjs;
};

template <typename NodeType, typename EdgeCostType>
Graph<NodeType, EdgeCostType>::Graph() {
}

template <typename NodeType, typename EdgeCostType>
Graph<NodeType, EdgeCostType>::~Graph() {
}


template<typename NodeType, typename EdgeCostType>
void Graph<NodeType, EdgeCostType>::addNode(const NodeType& node)
{
    if (adjs.find(node) != adjs.end()) {
        return;
    }

    EdgeSetType noEdges;
    adjs.insert(make_pair(node, noEdges));
}


template<typename NodeType, typename EdgeCostType>
void Graph<NodeType, EdgeCostType>::addEdge(const NodeType& source,
                                            const NodeType& dest,
                                            EdgeCostType cost)
{
    NodeType first = source < dest ? source : dest;
    NodeType second = source > dest ? source : dest;

    if (!hasNode(first) || !hasNode(dest)) {
        return;
    }

    adjs[first].insert(EdgeType(second, cost));
}

template<typename NodeType, typename EdgeCostType>
bool Graph<NodeType, EdgeCostType>::hasNode(const NodeType& node) const
{
    return (adjs.find(node) != adjs.end());
}

template<typename NodeType, typename EdgeCostType>
bool Graph<NodeType, EdgeCostType>::hasEdge(const NodeType& n1, const NodeType& n2) const
{
    if (!hasNode(n1) || !hasNode(n2)) {
        return false;
    }

    if (n1 < n2) {
        EdgeType dummy(n2, static_cast<EdgeCostType>(0));
        return (adjs[n1].find(dummy) != adjs[n1].end());
    } else {
        EdgeType dummy(n1, static_cast<EdgeCostType>(0));
        return (adjs[n2].find(dummy) != adjs[n2].end());
    }
}

template<typename NodeType, typename EdgeCostType>
EdgeCostType Graph<NodeType, EdgeCostType>::getCost(const NodeType& n1, const NodeType& n2) const
{
    if (!hasEdge(n1, n2)) {
        return static_cast<EdgeCostType>(0);
    }

    if (n1 < n2) {
        return adjs[n1][n2].cost;
    } else {
        return adjs[n2][n1].cost;
    }
}


template<typename NodeType, typename EdgeCostType>
std::vector<NodeType> Graph<NodeType, EdgeCostType>::neighbors(const NodeType& node) const
{
    std::vector<NodeType> result;
    EdgeType dummy(node, static_cast<EdgeCostType>(0));

    for (typename AdjacencyMapType::const_iterator adjsIt = adjs.begin();
            adjsIt != adjs.end(); ++adjsIt) {
        const EdgeSetType& edges = adjsIt->second;
        if (node == adjsIt->first) {
            for (typename EdgeSetType::const_iterator edgesIt = edges.begin();
                    edgesIt != edges.end(); ++edgesIt) {
                result.push_back(edgesIt->node);
            }
            break;
        } else if (edges.find(dummy) != edges.end()) {
            result.push_back(adjsIt->first);
        }
    }
    return result;
}

template<typename NodeType, typename EdgeCostType>
void Graph<NodeType, EdgeCostType>::dump() const
{
    std::cout << "Nodes: ";
    for (typename AdjacencyMapType::const_iterator adjsIt = adjs.begin();
            adjsIt != adjs.end(); ++adjsIt) {
        std::cout << adjsIt->first << std::endl;
    }

    std::cout << std::endl << "Adjacencies: " << std::endl;

    for (typename AdjacencyMapType::const_iterator adjsIt = adjs.begin();
            adjsIt != adjs.end(); ++adjsIt) {
        NodeType node = adjsIt->first;
        const EdgeSetType& edges = adjsIt->second;

        std::cout << node << " : ";
        for (typename EdgeSetType::const_iterator edgeIt = edges.begin();
                edgeIt != edges.end(); ++edgeIt) {
            EdgeType edge = *edgeIt;
            std::cout << "(" << edge.node << ", " << edge.cost << ")" << " ";
        }
        std::cout << std::endl;

    }
}

/*
 * Computes the shortest path between the two given nodes.
 * Returs true if such a path was found and false otherwise.
 * Keeps the computed path in the PathType parameter.
 */
template<typename NodeType, typename EdgeCostType>
bool Graph<NodeType, EdgeCostType>::path(const NodeType& from,
                                         const NodeType& to,
                                         PathType& path) const
{
    // One of nodes not in the graph
    if (!hasNode(from) || !hasNode(to)) {
        return false;
    }

    std::map<NodeType, NodeType> pred;
    std::priority_queue<NodeCost<NodeType, EdgeCostType>,
                        CostComparator<NodeType, EdgeCostType> > queue;

    NodeCost<NodeType, EdgeCostType> start(from,
                                    static_cast<EdgeCostType>(0));
    queue.push(start);

    while (!queue.empty()) {
        NodeCost<NodeType, EdgeCostType> next = queue.top();
        std::vector<NodeType> neighs = neighbors(next.node);
        for (typename std::vector<NodeType>::const_iterator nIt = neighs.begin();
                nIt != neighs.end(); ++nIt) {
            NodeType node = nIt->first;
            EdgeCostType cost = next.cost + getCost(node, next.node);
            NodeCost<NodeType, EdgeCostType> nc(node, cost);
            queue.push(nc);
            pred.insert(std::make_pair(node, next.node));
        }
    }

    if (pred.find(to) != pred.end()) {
        NodeType last = to;
        EdgeCostType cost = static_cast<EdgeCostType>(0);
        while (pred.find(last) != pred.end()) {
            NodeType next = pred[last];
            path.push_front(last);
            cost += getCost(last, next);
            last = next;
        }

        path.setCost(cost);
    }

    return false;
}

#endif //GRAPH_H__

