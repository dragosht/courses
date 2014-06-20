#include "hex.h"

int main(int argc, char *argv[])
{
    /* Simply create a board and simulate a game */
    HexBoard board(7);
    board.simulate();
    return 0;
}

