#include "graph.h"

using namespace std;

// Tests the sample file (reads from 'test.in')
static void TestFile();

// Tests on a randomly generated graph
static void TestRandom();


int main(int argc, char* argv[])
{
    cout << "Testing MST from file loaded graph ..." << endl;
    TestFile();
    cout << endl << "Testing MST on a random graph ..." << endl;
    TestRandom();

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

static void TestRandom()
{
    Graph<int, float> graph(50, 0.4, IntNodeGenerator(), FloatCostGenerator(1., 10.));
    graph.dump();
    EdgeSet<int, float> tree;
    if (graph.span(tree)) {
        tree.dump();
    } else {
        cout << "Not connected!" << endl;
    }
}

static void TestFile()
{
    ifstream fin("test.in");
    Graph<int, int> g(fin);
    //g.dump();
    EdgeSet<int, int> tree;
    if (g.span(tree)) {
        tree.dump();
    } else {
        cout << "Not connected!" << endl;
    }
}

