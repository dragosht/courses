#include <cstdlib>
#include <ctime>

#include <vector>
#include <list>
#include <map>
#include <queue>
#include <iostream>
#include <utility>

template <typename NodeType, typename CostType>
struct DistanceToNode {
    NodeType node;
    CostType cost;

    DistanceToNode(NodeType node, CostType cost) :
        node(node), cost(cost) {}

};

template <typename NodeType, typename CostType>
bool operator < (const DistanceToNode<NodeType, CostType>& d1,
                 const DistanceToNode<NodeType, CostType>& d2) {
    return d1.cost > d2.cost;
}



template<typename NodeType, typename CostType>
class Path : public std::list<NodeType>
{
public:
    CostType getCost() const {
        return cost;
    }

    void setCost(CostType cost) {
        this->cost = cost;
    }

    void dump() const {
        for (typename std::list<NodeType>::const_iterator pathIt = this->begin();
                pathIt != this->end(); pathIt++) {
            std::cout << *pathIt << " ";
        }

        std::cout << "Cost: " << cost << std::endl;
    }

private:
    CostType cost;
};

template <typename NodeLabelType, typename CostType>
class Graph
{
public:
    typedef unsigned long node_type;
    typedef std::map<NodeLabelType, node_type> label2node_type;
    typedef std::map<node_type, NodeLabelType> node2label_type;
    typedef std::vector< std::vector<CostType> > adjacency_type;
    typedef typename std::vector<CostType>::size_type size_type;
    typedef Path<NodeLabelType, CostType> path_type;

public:
    Graph();

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

    ~Graph();

    void addNode(const NodeLabelType& node);
    void addEdge(const NodeLabelType& from, const NodeLabelType& to, CostType cost);

    bool hasNode(const NodeLabelType& node) const;
    bool hasEdge(const NodeLabelType& from, const NodeLabelType& to);

    bool path(const NodeLabelType& from, const NodeLabelType& to, path_type& path);

    void dump();

private:
    node2label_type labels;
    label2node_type nodes;
    adjacency_type adjs;
};


template<typename NodeLabelType, typename CostType>
Graph<NodeLabelType, CostType>::Graph(){
}

template<typename NodeLabelType, typename CostType>
Graph<NodeLabelType, CostType>::~Graph(){
}

template<typename NodeLabelType, typename CostType>
void Graph<NodeLabelType, CostType>::addNode(const NodeLabelType& node)
{
    int index = static_cast<int>(nodes.size());
    nodes.insert(std::make_pair(node, index));
    labels.insert(std::make_pair(index, node));

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
    if (nodes.find(from) == nodes.end() ||
        nodes.find(to) == nodes.end()) {
        return;
    }

    node_type fromNode = nodes[from];
    node_type toNode = nodes[to];

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
bool Graph<NodeLabelType, CostType>::path(const NodeLabelType& from,
                                          const NodeLabelType& to,
                                          path_type& path) {
    if (!hasNode(from) || !hasNode(to)) {
        return false;
    }

    node_type fromNode = nodes[from];
    node_type toNode = nodes[to];

    DistanceToNode<node_type, CostType> start(fromNode,
                    static_cast<CostType>(0));

    std::priority_queue<DistanceToNode<node_type, CostType> > queue;
    std::vector<bool> visited(nodes.size(), false);
    std::vector<CostType> distances(nodes.size(), static_cast<CostType>(0));
    std::map<node_type, node_type> pred;

    queue.push(start);

    while (!queue.empty()) {
        DistanceToNode<node_type, CostType> next = queue.top();
        queue.pop();
        visited[next.node] = true;

        for (size_type i = 0; i < nodes.size(); i++) {
            if (adjs[next.node][i] != static_cast<CostType>(0)) {
                CostType dist = next.cost + adjs[next.node][i];
                if (!visited[i] &&
                        ((distances[i] == static_cast<CostType>(0) && i != fromNode) ||
                        distances[i] > dist)) {
                     distances[i] = dist;
                     pred[i] = next.node;
                     DistanceToNode<node_type, CostType> neighbor(i, dist);
                     queue.push(neighbor);
                }
            }
        }
    }

    if (pred.find(toNode) == pred.end()) {
        return false;
    }

    node_type last = toNode;
    while (pred.find(last) != pred.end()) {
        path.push_front(labels[last]);
        last = pred[last];
    }
    path.push_front(from);
    path.setCost(distances[toNode]);

    return true;
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


