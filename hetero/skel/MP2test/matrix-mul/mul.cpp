#include <cassert>

#include <vector>
#include <string>
#include <iostream>
#include <random>
#include <chrono>

using namespace std;

typedef vector< vector<int> > matrix_t;

static void Multiply(const matrix_t& m1,
              const matrix_t& m2,
              matrix_t& res);

static void Print(const matrix_t& m);

int main(int argc, char *argv[])
{
    matrix_t m1, m2, res;
    const int m = 4, n = 4, k = 4;
    const unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    default_random_engine rand(seed);

    m1.resize(m);
    for (unsigned i = 0; i < m1.size(); i++) {
        m1[i].resize(n);
        for (unsigned j = 0; j < m1[i].size(); j++) {
            m1[i][j] = rand() % 5;
        }
    }

    m2.resize(n);
    for (unsigned i = 0; i < m2.size(); i++) {
        m2[i].resize(k);
        for (unsigned j = 0; j < m2[i].size(); j++) {
            m2[i][j] = rand() % 5;
        }
    }

    Multiply(m1, m2, res);

    cout << "M: " << endl;
    Print(m1);
    cout << endl;
    Print(m2);
    cout << endl;
    Print(res);

    return 0;
}

void Multiply(const matrix_t& m1,
              const matrix_t& m2,
              matrix_t& res)
{

    assert(m1.size() > 0 && m2.size() > 0 && m1[0].size() == m2.size());

    int m = m1.size();
    int n = m2.size();
    int p = m2[0].size();

    res.resize(m);

    for (int i = 0; i < m; i++) {
        res[i].resize(p);

        for (int j = 0; j < n; j++) {
            res[i][j] = 0;
            for (int k = 0; k < p; k++) {
                res[i][j] += m1[i][k] * m2[k][j];
            }
        }
    }
}

void Print(const matrix_t& m)
{
    for (unsigned i = 0; i < m.size(); i++) {
        for (unsigned j = 0; j < m[i].size(); j++) {
            cout << m[i][j] << " ";
        }
        cout << endl;
    }
}


