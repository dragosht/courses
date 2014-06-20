
/**
 * Simple N-puzzle solver.
 * @author Dragos Tarcatu <tarcatu_dragosh@yahoo.com>
 *
 */

public class Solver {
    /**
     * A* state node consisting in:
     * - Current board configuration
     * - the already done number of moves
     * - previous state node
     * - twin flag (?) - shows whether this node is trying
     *   to reach a solution or prove none exists
     */
    
    private static class State implements Comparable<State> {
        private Board board;
        private int numMoves;
        private State prev;
        private boolean twin;
        
        State(Board board, int numMoves, State prev, boolean twin) {
            this.board = board;
            this.numMoves = numMoves;
            this.prev = prev;
            this.twin = twin;
        }
        
        public Board getBoard() {
            return board;
        }
        
        public int getNumMoves() {
            return numMoves;
        }
        
        public State getPrev() {
            return prev;
        }
        
        public boolean getTwin() {
            return twin;
        }

        @Override
        public int compareTo(State that) {
            int cost1 = this.getBoard().manhattan() + this.getNumMoves();
            int cost2 = that.getBoard().manhattan() + that.getNumMoves();
            if (cost1 < cost2) {
                return -1;
            } else if (cost1 > cost2) {
                return 1;
            }
            return 0;
        }
    }
    
    
    private MinPQ<State> queue;
    private boolean solvable;
    private LinkedQueue<Board> game; 
    
    private void trace(State goal) {
        LinkedStack<Board> boards = new LinkedStack<Board>();
        
        State curr = goal;
        while (curr != null) {
            boards.push(curr.getBoard());
            curr = curr.getPrev();
        }
        
        while (!boards.isEmpty()) {
            game.enqueue(boards.pop());
        }
    }
    
    /**
     * find a solution to the initial board (using the A* algorithm)
     * @param initial
     */
    public Solver(Board initial) {
        queue = new MinPQ<State>();
        game = new LinkedQueue<Board>();
        
        queue.insert(new State(initial, 0, null, false));
        queue.insert(new State(initial.twin(), 0, null, true));
        
        while (true) {
            State current = queue.delMin();
            
            //StdOut.println("current: \n" + current.getBoard().toString());
            
            if (current.board.isGoal()) {
                solvable = !current.getTwin();
                if (solvable) {
                    trace(current);
                }
                break;
            }
            
            for (Board crtBoard : current.board.neighbors()) {
                State next = new State(crtBoard, current.getNumMoves() + 1,
                                       current, current.getTwin());
                
                if (current.getPrev() != null
                        && current.getPrev().getBoard().equals(next.getBoard())) {
                    continue;
                }
                
                queue.insert(next);
            }
        }
    }
    
    /**
     * is the initial board solvable?
     * @return
     */
    public boolean isSolvable() {
        return solvable;
    }
    
    /**
     * min number of moves to solve initial board; -1 if no solution
     * @return
     */
    public int moves() {
        int size = game.size();
        
        if (!solvable) {
            return -1;
        }
        
        if (size > 0) {
            return size - 1;
        }
        
        return 0;
    }
    
    
    /**
     * sequence of boards in a shortest solution; null if no solution
     * @return
     */
    public Iterable<Board> solution() {
        if (!solvable) {
            return null;
        }
        return game;
    }
    
    public static void main(String[] args) {
        
        // create initial board from file
        In in = new In(args[0]);
        int N = in.readInt();
        int[][] blocks = new int[N][N];
        for (int i = 0; i < N; i++)
            for (int j = 0; j < N; j++)
                blocks[i][j] = in.readInt();
        Board initial = new Board(blocks);

        // solve the puzzle
        Solver solver = new Solver(initial);

        // print solution to standard output
        if (!solver.isSolvable())
            StdOut.println("No solution possible");
        else {
            StdOut.println("Minimum number of moves = " + solver.moves());
            for (Board board : solver.solution())
                StdOut.println(board);
        }
    }
}

