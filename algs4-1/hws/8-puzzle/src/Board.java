/**
 * 
 * @author Dragos Tarcatu <tarcatu_dragosh@yahoo.com>
 *
 */
public class Board {
    private int N;
    private int [][] blocks;
    
    /**
     * construct a board from an N-by-N array of blocks
     * (where blocks[i][j] = block in row i, column j)
     * @param blocks
     */
    public Board(int[][] blocks) {
        
        this.N = blocks.length;
        this.blocks = new int[N][N];

        for (int i = 0; i < blocks.length; i++) {
            for (int j = 0; j < blocks[i].length; j++) {
                assert this.N == blocks[i].length;
                this.blocks[i][j] = blocks[i][j];
            }
        }
    }
                                           
    public int dimension() {
        return N;
    }
    
    /**
     * number of blocks out of place
     * @return
     */
    public int hamming() {
        int sum = 0;
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                if (blocks[i][j] != 0) {
                    if (blocks[i][j] != i * N + j + 1) {
                        sum++;
                    }
                }
            }
        }
        
        return sum;
    }
    
    /**
     * sum of Manhattan distances between blocks and goal
     * @return
     */
    public int manhattan() {
        int sum = 0;
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                if (blocks[i][j] != 0) {
                    int goalRow = (blocks[i][j] - 1) / N;
                    int goalCol = (blocks[i][j] - 1) % N;
                    sum += (Math.abs(goalRow - i) + Math.abs(goalCol - j));
                }
            }
        }
        
        return sum;
    }
    
    public boolean isGoal() {
        // is this board the goal board?
        boolean found = true;
        int k = 1;
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                if (blocks[i][j] != (k % (N*N))) {
                    found = false;
                    break;
                }
                k++;
            }
        }
        return found;
    }
    
    public Board twin() {
        // a board obtained by exchanging two adjacent blocks in the same row
        Board board = new Board(blocks);
        
        int row = StdRandom.uniform(N);
        int col1 = 0;
        int col2 = 1;
        
        if (board.blocks[row][0] == 0
                || board.blocks[row][1] == 0) {
            row = (row + 1) % N;
        }
        
        int tmp = board.blocks[row][col1];
        board.blocks[row][col1] = board.blocks[row][col2];
        board.blocks[row][col2] = tmp;
        
        return board;
    }
    
    /**
     * does this board equal y?
     */
    public boolean equals(Object y) {
        if (y == this) return true;
        if (y == null) return false;
        if (y.getClass() != this.getClass()) return false;
        
        Board that = (Board) y;
        
        if (this.N != that.N) {
            return false;
        }
        
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                if (this.blocks[i][j] != that.blocks[i][j]) {
                    return false;
                }
            }
        }
        
        return true;
    }
    
    /**
     * all neighboring boards
     * @return
     */
    public Iterable<Board> neighbors() {
        Queue<Board> queue = new Queue<Board>();
        
        //Find the empty box
        int iEmpty = 0;
        int jEmpty = 0;
        
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                if (blocks[i][j] == 0) {
                    iEmpty = i;
                    jEmpty = j;
                    break;
                }
            }
            
        }
        
        if (iEmpty > 0) {
            Board up = new Board(this.blocks);
            int tmp = up.blocks[iEmpty - 1][jEmpty];
            up.blocks[iEmpty - 1][jEmpty] = up.blocks[iEmpty][jEmpty];
            up.blocks[iEmpty][jEmpty] = tmp;
            queue.enqueue(up);
        }
        
        if (jEmpty > 0) {
            Board left = new Board(this.blocks);
            int tmp = left.blocks[iEmpty][jEmpty - 1];
            left.blocks[iEmpty][jEmpty - 1] = left.blocks[iEmpty][jEmpty];
            left.blocks[iEmpty][jEmpty] = tmp;
            queue.enqueue(left);
        }
        
        if (iEmpty < N - 1) {
            Board down = new Board(this.blocks);
            int tmp = down.blocks[iEmpty + 1][jEmpty];
            down.blocks[iEmpty + 1][jEmpty] = down.blocks[iEmpty][jEmpty];
            down.blocks[iEmpty][jEmpty] = tmp;
            queue.enqueue(down);
        }
        
        if (jEmpty < N - 1) {
            Board right = new Board(this.blocks);
            int tmp = right.blocks[iEmpty][jEmpty + 1];
            right.blocks[iEmpty][jEmpty + 1] = right.blocks[iEmpty][jEmpty];
            right.blocks[iEmpty][jEmpty] = tmp;
            queue.enqueue(right);
        }
        
        return queue;
    }
    
    /**
     * string representation of the board (in the output format specified below)
     */
    public String toString() {
        StringBuilder s = new StringBuilder();
        s.append(N + "\n");
        
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                s.append(String.format("%2d ", blocks[i][j]));
            }
            s.append("\n");
        }
        
        return s.toString();
    }
    
    public static void main(String[] args) {
        int [][] pos = new int[3][3];
        //int [] vals = {1, 2, 3, 4, 5, 6, 7, 8, 0};
        int [] vals = {1, 2, 3, 4, 0, 5, 6, 7, 8};
        int k = 0;
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                pos[i][j] = vals[k++];
            }
        }
        
        Board test = new Board(pos);
        StdOut.println("hamming: " + test.hamming());
        StdOut.println("manhattan: " + test.manhattan());
        StdOut.println("Goal: " + test.isGoal());
        StdOut.println("Board.toString(): \n" + test.toString());
        StdOut.println("Board.twin(): \n" + test.twin());
        
        StdOut.println("Neighbors: \n");
        for (Board b : test.neighbors()) {
            StdOut.println(b);
            StdOut.println("hamming: " + b.hamming());
            StdOut.println("manhattan: " + b.manhattan());
            StdOut.println("Goal: " + b.isGoal());
            StdOut.println("Board.toString(): \n" + b.toString());
            StdOut.println("Board.twin(): \n" + b.twin());
        }
        
    }
}
