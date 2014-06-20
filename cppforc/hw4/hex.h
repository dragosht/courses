#ifndef HEX_H__
#define HEX_H__

#include <vector>
#include <list>
#include <map>
#include <utility>

/*
 * Hex game player.
 */
enum class HexPlayer {
    PLAYER1,
    PLAYER2
};

/*
 * Cell ownership.
 */
enum class HexColor {
    NONE,
    BLACK,
    WHITE
};


/*
 * The main class representing the hex board game.
 */
class HexBoard
{
public:
    typedef unsigned long size_type;

public:
    /* Build a hex board of the given size */
    HexBoard(size_type size);

    /* Dumps the graph structure */
    void dumpGraph();

    /* Duumps the board structure */
    void dumpBoard();

    /*
     * Dumps a specific cell:
     * o - white stone
     * x - black stone
     * . - empty cell
     */
    void dumpCell(long x, long y);

    HexColor getCell(size_type x, size_type y) const;
    void setCell(size_type x, size_type y, HexColor cell);

    /* Gets the in-game color of a specific player */
    HexColor getColor(HexPlayer player) const;

    /*
     * Executes a move by placing a piece on the board at the
     * specified position. Returns true if a chain between
     * the two opposite faces of the current player was formed
     * (game over) and false otherwise.
     */
    bool move(HexPlayer player, size_type x, size_type y);

    /*
     * Runs a game simulation based on randomly generated moves.
     */
    void simulate();

private:
    /* Some internally used data dypes */
    typedef long node_type;
    typedef std::pair<long, long> pos_type;
    typedef std::vector< std::list<node_type> > adjacency_type;
    typedef std::vector< std::vector<HexColor> > board_type;

private:
    /*
     * Utility methods for node to position and position
     * to node conversion.
     */
    node_type node(long x, long y);
    node_type node(const pos_type& pos);
    pos_type pos(node_type nd);

    /* Initializes the graph structure */
    void initGraph();
    /* Initializes the board configuration */
    void initBoard();

    /* Checks this cell's connectivity to the board edges */
    bool check(size_type x, size_type y);

private:
    size_type size;                         // Size of the board
    adjacency_type adjs;                    // Adjacency list for the graph
    board_type board;                       // Board matrix (pieces map)
    HexPlayer next;                         // Next player to move
    std::pair<HexColor, HexColor> colors;   // Colors chosen by each player
};



#endif //HEX_H__

