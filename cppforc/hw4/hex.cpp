#include <iostream>
#include <random>
#include <chrono>
#include <algorithm>
#include <thread>
#include <queue>

#include "hex.h"

#define HEXDEBUG

using namespace std;

#ifdef HEXDEBUG
    #define HEX_DEBUG(line) line
    #define HEX_NDEBUG(line)
#else
    #define HEX_DEBUG(line)
    #define HEX_NDEBUG(line) line
#endif


/*
 Hardwire the graph structure for a board like this:

 . - . - . - .
  \ / \ / \ / \
   . - . - . - .
    \ / \ / \ / \
     . - . - . - .
      \ / \ / \ / \
       . - . - . - .

 Note: For a different orientation of the board the
 representation would need a few changes.
 */

void HexBoard::initGraph()
{
    long max = static_cast<long>(size);

    for (HexBoard::node_type i = 0; i < max*max; i++) {
        HexBoard::node_type nd = i;
        std::list<node_type> neighbors;
        HexBoard::pos_type p = pos(nd);

        if (p.first > 0 && p.second + 1 < max) {
            neighbors.push_back(node(p.first - 1, p.second + 1));
        }
        if (p.second + 1 < max) {
            neighbors.push_back(node(p.first, p.second + 1));
        }
        if (p.first + 1 < max) {
            neighbors.push_back(node(p.first + 1, p.second));
        }
        if (p.first + 1 < max && p.second > 0) {
            neighbors.push_back(node(p.first + 1, p.second - 1));
        }
        if (p.second > 0) {
            neighbors.push_back(node(p.first, p.second - 1));
        }
        if (p.first > 0) {
            neighbors.push_back(node(p.first - 1, p.second));
        }

        adjs.push_back(neighbors);
    }
}

/* Create an empty hex board */
void HexBoard::initBoard()
{
    for (HexBoard::size_type i = 0; i < size; i++) {
        vector<HexColor> row(size, HexColor::NONE);
        for (HexBoard::size_type j = 0; j < size; j++) {
            board.push_back(row);
        }
    }
}

HexBoard::HexBoard(HexBoard::size_type size) : size(size)
{
    initGraph();
    initBoard();
}

/* Get the node for the corresponding cell */
HexBoard::node_type HexBoard::node(long x, long y)
{
    long max = static_cast<long>(size);
    return x * max + y;
}

HexBoard::node_type HexBoard::node(const HexBoard::pos_type& pos)
{
    return node(pos.first, pos.second);
}

/* Get the position of the specific node */
HexBoard::pos_type HexBoard::pos(HexBoard::node_type nd)
{
    long max = static_cast<long>(size);
    return make_pair(nd / max, nd % size);
}

void HexBoard::dumpGraph()
{
    long max = static_cast<long>(size);
    for (long i = 0; i < max; i++) {
        for (long j = 0; j < max; j++) {
            HexBoard::node_type nd = node(i, j);

            cout << "(" << i << "," << j << ") " << " node :" <<
                nd << " neighbors: ";

            for (auto nb : adjs[nd]) {
                HexBoard::pos_type p = pos(nb);
                cout << nb << " (" << p.first << "," << p.second << ") ";
            }

            cout << endl;
        }
    }
}

void HexBoard::dumpBoard()
{
    string indent = "";
    for (HexBoard::size_type i = 0; i < size; i++) {
        cout << indent;
        for (HexBoard::size_type j = 0; j < size; j++) {
            dumpCell(i, j);
            if (j + 1 < size) {
                cout << " - ";
            }
        }
        cout << endl;

        indent += " ";

        if (i + 1 < size) {
            cout << indent;
            for (HexBoard::size_type j = 0; j + 1 < 2*size; j++) {
                if (j % 2 == 0) {
                    cout << "\\ ";
                } else {
                    cout << "/ ";
                }
            }
            cout << endl;

            indent += " ";
        }
    }
}

void HexBoard::dumpCell(long x, long y)
{
    string ret = "";
    switch(board[x][y]) {
    case HexColor::BLACK:
        ret = "x";
        break;
    case HexColor::WHITE:
        ret = "o";
        break;
    default:
        ret = ".";
        break;
    }

    cout << ret;
}

HexColor HexBoard::getCell(size_type x, size_type y) const
{
    if (x >= size || y >= size) {
        return HexColor::NONE;
    }
    return board[x][y];
}

void HexBoard::setCell(HexBoard::size_type x, HexBoard::size_type y, HexColor cell)
{
    if (x >= size || y >= size) {
        return;
    }

    board[x][y] = cell;
}

HexColor HexBoard::getColor(HexPlayer player) const
{
    if (player == HexPlayer::PLAYER1) {
        return colors.first;
    }
    return colors.second;
}

bool HexBoard::move(HexPlayer player, size_type x, size_type y)
{
    if (player != next) {
        return false;
    }

    if (getCell(x, y) != HexColor::NONE) {
        return false;
    }

    /* It all right - do the move ... */
    HexColor color = getColor(player);
    board[x][y] = color;

    next = (player == HexPlayer::PLAYER1) ?
                HexPlayer::PLAYER2 : HexPlayer::PLAYER1;

    /* Now check to see if we won by doing it */
    return check(x, y);
}

/*
 * Does a breadth-first search (BFS) from the just
 * placed piece to see if a winning chain was formed.
 */
bool HexBoard::check(size_type x, size_type y)
{
    HEX_DEBUG(cout << "Checking move to: " << x << ","   << y << endl);
    if (x >= size || y >= size) {
        cerr << "Invalid position: " << x << "," << y  << endl;
        return false;
    }

    HexColor color = board[x][y];
    if (color == HexColor::NONE) {
        cerr << "No valid color at: " << x << ", " << y << endl;
        return false;
    }

    vector<bool> checked(size*size, false);
    queue<HexBoard::node_type> q;
    bool top, buttom, left, right;

    q.push(node(x, y));
    top = buttom = left = right = false;

    while (!q.empty()) {
        HexBoard::node_type nd = q.front();
        HexBoard::pos_type p = pos(nd);
        HEX_DEBUG(cout << "Going through: " << nd << ": " <<
                p.first << ", " << p.second << endl);
        checked[nd] = true;
        q.pop();

        if (p.first == 0) {
            HEX_DEBUG(cout << "Reaches top " << p.first << ", " << p.second << endl);
            top = true;
        }
        if (p.first + 1 == static_cast<long>(size)) {
            HEX_DEBUG(cout << "Reaches buttom " << p.first << ", "
                    << p.second << endl);
            buttom = true;
        }
        if (p.second == 0) {
            HEX_DEBUG(cout << "Reaches left " << p.first << ", " << p.second << endl);
            left = true;
        }
        if (p.second + 1 == static_cast<long>(size)) {
            HEX_DEBUG(cout << "Reaches right " << p.first << ", " << p.second << endl);
            right = true;
        }

        if (color == HexColor::WHITE && left && right) {
            return true;
        }

        if (color == HexColor::BLACK && top && buttom) {
            return true;
        }

        for (auto neighbor: adjs[nd]) {
            HexBoard::pos_type p = pos(neighbor);
            if (!checked[neighbor] && board[p.first][p.second] == color) {
                q.push(neighbor);
                HEX_DEBUG(cout << "Addind " << neighbor << " : " <<
                        p.first << ", " << p.second << endl);
                /* No point to visit it multiple times */
                checked[neighbor] = true;
            }
        }
    }

    HEX_DEBUG(cout << "No win" << endl);
    return false;
}


void HexBoard::simulate()
{
    next = HexPlayer::PLAYER1;
    colors = make_pair(HexColor::WHITE, HexColor::BLACK);
    node_type max = static_cast<node_type>(size * size);

    vector<node_type> nodes;
    for (node_type i = 0; i < max; i++) {
        nodes.push_back(i);
    }
    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    shuffle(nodes.begin(), nodes.end(), default_random_engine(seed));

    HEX_NDEBUG(system("clear"));

    bool finished = false;
    for (auto node : nodes) {
        pos_type p = pos(node);
        HexPlayer player = next;
        finished = move(player, p.first, p.second);
        HEX_NDEBUG(system("clear")); //This won't work on Windows
        dumpBoard();
        if (finished) {
            cout << "Player " << ((player == HexPlayer::PLAYER1) ? "1" : "2") << " wins!" << endl;
            break;
        }

        std::chrono::milliseconds millis(300);
        std::this_thread::sleep_for(millis);
    }
}

