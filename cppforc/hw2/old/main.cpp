#include <iterator>
#include <algorithm>

#include "graph.h"

using namespace std;

static void test();


int main(int argc, char *argv[]) {

    test();

    return 0;
}


void test() {
    Graph<int, int> graph;

    graph.addNode(0);
    assert(graph.hasNode(0));
    graph.addNode(1);
    assert(graph.hasNode(1));
    graph.addNode(2);
    assert(graph.hasNode(2));
    graph.addNode(3);
    assert(graph.hasNode(3));
    graph.addNode(4);
    assert(graph.hasNode(4));
    graph.addNode(5);
    assert(graph.hasNode(5));
    graph.addNode(6);
    assert(graph.hasNode(6));

    graph.addEdge(0, 1, 1);
    assert(graph.hasEdge(0, 1));
    graph.addEdge(0, 2, 4);
    assert(graph.hasEdge(0, 2));
    graph.addEdge(1, 2, 2);
    assert(graph.hasEdge(1, 2));
    graph.addEdge(1, 3, 5);
    assert(graph.hasEdge(1, 3));
    graph.addEdge(2, 3, 1);
    assert(graph.hasEdge(2, 3));
    graph.addEdge(2, 5, 6);
    assert(graph.hasEdge(2, 5));
    graph.addEdge(3, 4, 8);
    assert(graph.hasEdge(3, 4));
    graph.addEdge(3, 5, 3);
    assert(graph.hasEdge(3, 5));
    graph.addEdge(5, 6, 4);
    assert(graph.hasEdge(5, 6));

    graph.dump();

    vector<int> neighs = graph.neighbors(0);
    cout << "nighbors: 0 - ";
    copy(neighs.begin(), neighs.end(), ostream_iterator<int>(cout, " "));
    cout << endl;

    neighs = graph.neighbors(6);
    cout << "neighbors: 6 - ";
    copy(neighs.begin(), neighs.end(), ostream_iterator<int>(cout, " "));
    cout << endl;

    Path<int, int> path;

    graph.path(0, 6, path);
}



