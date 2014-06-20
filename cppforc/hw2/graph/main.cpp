#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <string>

#include "graph.h"

using namespace std;

static void TestGraphIntInt();
static void TestGraphStringFloat();

static void TestGraphIntFloatRandom();

int main(int argc, char *argv[]) {
    TestGraphIntFloatRandom();
    return 0;
}

class IntNodeGenerator
{
public:
    IntNodeGenerator() : value(0) {
    }

    int operator() () {
        return value++;
    }
private:
    int value;
};

class FloatCostGenerator
{
public:
    FloatCostGenerator(float min, float max) : min(min), max(max) {
        srand(time(0));
    }

    float operator() () {
        float r = ((float) rand()) / (float) RAND_MAX;
        return min + r * (max - min);
    }

private:
    float min;
    float max;
};

void TestGraphIntFloatRandom()
{
    Graph<int, float> graph(50, 0.4, IntNodeGenerator(), FloatCostGenerator(1., 10.));
    graph.dump();

    float pathAvg = 0.;
    int numPaths = 0;

    for (int i = 1; i < 50; i++) {
        Graph<int, float>::path_type path;

        if (graph.path(0, i, path)) {
            numPaths++;
            pathAvg += path.getCost();
        }
    }

    cout << "Average length: " << pathAvg / numPaths << " paths: " << numPaths << endl;
}

void TestGraphStringFloat()
{
    Graph<string, float> graph;
    graph.addNode("0");
    assert(graph.hasNode("0"));
    graph.addNode("1");
    assert(graph.hasNode("1"));
    graph.addNode("2");
    assert(graph.hasNode("2"));
    graph.addNode("3");
    assert(graph.hasNode("3"));
    graph.addNode("4");
    assert(graph.hasNode("4"));
    graph.addNode("5");
    assert(graph.hasNode("5"));
    graph.addNode("6");
    assert(graph.hasNode("6"));

    graph.addEdge("0", "1", 1);
    assert(graph.hasEdge("0", "1"));
    graph.addEdge("0", "2", 4.);
    assert(graph.hasEdge("0", "2"));
    graph.addEdge("1", "2", 2.);
    assert(graph.hasEdge("1", "2"));
    graph.addEdge("1", "3", 5.);
    assert(graph.hasEdge("1", "3"));
    graph.addEdge("2", "3", 1.);
    assert(graph.hasEdge("2", "3"));
    graph.addEdge("2", "5", 6.);
    assert(graph.hasEdge("2", "5"));
    graph.addEdge("3", "4", 8.);
    assert(graph.hasEdge("3", "4"));
    graph.addEdge("3", "5", 3.);
    assert(graph.hasEdge("3", "5"));
    graph.addEdge("5", "6", 4.);
    assert(graph.hasEdge("5", "6"));
    graph.dump();

    Graph<string, float>::path_type path;
    if (graph.path("0", "6", path)) {
        path.dump();
    }
}

void TestGraphIntInt()
{
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

    Graph<int, int>::path_type path;
    if (graph.path(0, 6, path)) {
        path.dump();
    }
}

