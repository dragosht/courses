#include <iostream>
#include <vector>

using namespace std;

static const int N = 40;

/*
 * Computes the sum of the elements in the "d" vector.
 */
static void sum(int& p, int n, const vector<int>& d) {
    p = 0;
    for (int i = 0; i < n; ++i) {
        p += d[i];
    }
}

int main()
{
    vector<int> data;
    int accum = 0;

    for (int i = 0; i < N; ++i) {
        data.push_back(i);
    }

    sum(accum,N, data);
    cout << "sum is " << accum << endl;

    return 0;
}

