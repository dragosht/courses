
public class Percolation {

    private QuickFindUF prc;
    private QuickFindUF flow;
    private boolean[] grid;
    private int N;
    
    private final int BOTTOM;
    private final int TOP;
    
    /**
     * create N-by-N grid, with all sites blocked
     * @param N
     */
    public Percolation(int N) {
        this.N = N;
        this.TOP = N * N;
        this.BOTTOM = N * N + 1;
        grid = new boolean[N * N];
        prc = new QuickFindUF(N * N + 2);
        flow = new QuickFindUF(N * N + 1);
    }

    /**
     * Returns a linearized index for the given coordinates!
     * @param i
     * @param j
     * @return 
     */
    private int getIndex(int i, int j) {
        return (i - 1) * N + (j - 1);
    }
    
    /**
     * open site (row i, column j) if it is not already
     * @param i
     * @param j
     */
    public void open(int i, int j) {
        if (i < 1 || j < 1 || i > N || j > N) {
            throw new IndexOutOfBoundsException();
        }
        
        if (!isOpen(i, j)) {
            int idx = getIndex(i, j);
            
            grid[idx] = true;
            
            if (i > 1 && isOpen(i - 1, j)) {
                int up = getIndex(i - 1, j);
                prc.union(idx, up);
                flow.union(idx, up);
            }
            
            if (i < N && isOpen(i + 1, j)) {
                int down = getIndex(i + 1, j);
                prc.union(idx, down);
                flow.union(idx, down);
            }
            
            if (j > 1 && isOpen(i, j - 1)) {
                int left = getIndex(i, j - 1);
                prc.union(idx, left);
                flow.union(idx, left);
            }
            
            if (j < N && isOpen(i, j + 1)) {
                int right = getIndex(i, j + 1);
                prc.union(idx, right);
                flow.union(idx, right);
            }
            
            if (i == 1) {
                prc.union(idx, TOP);
                flow.union(idx,  TOP);
            }
            
            if (i == N) {
                prc.union(idx, BOTTOM);
            }
        }
    }

    /**
     * Is site (row i, column j) open?
     * @param i
     * @param j
     * @return
     */
    public boolean isOpen(int i, int j) {
        if (i < 1 || j < 1 || i > N || j > N) {
            throw new IndexOutOfBoundsException();
        }
        int idx = (i - 1) * N + (j - 1);
        return grid[idx];
    }

    /**
     * is site (row i, column j) full?
     * @param i
     * @param j
     * @return
     */
    public boolean isFull(int i, int j) {
        if (i < 1 || j < 1 || i > N || j > N) {
            throw new IndexOutOfBoundsException();
        }
        int idx = (i - 1) * N + (j - 1);
        return (isOpen(i, j) && flow.connected(idx, TOP));
    }

    /**
     * Does the system percolate?
     * @return
     */
    public boolean percolates() {
        return prc.connected(TOP, BOTTOM);
    }
}