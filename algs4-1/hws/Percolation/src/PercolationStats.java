public class PercolationStats {

    int N;
    int T;
    
    private double[] values;
    
    private boolean meanActive;
    private boolean stddevActive;
    private boolean confLoActive;
    private boolean confHiActive;

    private double meanValue;
    private double stddevValue;
    private double confLoValue;
    private double confHiValue;
    
    public PercolationStats(int N, int T) {
        // perform T independent computational experiments on an N-by-N grid
        this.N = N;
        this.T = T;
        
        values = new double[T];

        meanActive = false;
        stddevActive = false;
        confLoActive = false;
        confHiActive = false;
        
        for (int i = 0; i < T; i++) {
            Percolation p = new Percolation(N);
            int value = 0;
            while (!p.percolates()) {
                int idx = StdRandom.uniform(0, N * N);
                int x = (idx / N) + 1;
                int y = (idx % N) + 1;
                if (!p.isOpen(x, y)) {
                    p.open(x, y);
                    value++;
                }
            }
            values[i] = (double) value / (N * N);
        }
    }

    /**
     * sample mean of percolation threshold
     * @return
     */
    public double mean() {
        if (!meanActive) {
            meanValue = StdStats.mean(values);
            meanActive = true;
        }
        return meanValue;
    }

    /**
     * sample standard deviation of percolation threshold
     * @return
     */
    public double stddev() {
        if (!stddevActive) {
            stddevValue = StdStats.stddev(values);
            stddevActive = true;
        }
        return stddevValue;
        
    }

    /**
     * returns lower bound of the 95% confidence interval
     * @return
     */
    public double confidenceLo() {
        if (!confLoActive) {
            confLoValue = mean() - 1.96 * stddev() / Math.sqrt(T);
            confLoActive = true;
        }
        
        return confLoValue;
    }

    /**
     * returns upper bound of the 95% confidence interval
     * @return
     */
    public double confidenceHi() {
        if (!confHiActive) {
            confHiValue = mean() + 1.96 * stddev() / Math.sqrt(T);
            confHiActive = true;
        }
        return confHiValue;
    }

    public static void main(String[] args) {
        // test client, described below
        int N = Integer.parseInt(args[0]);
        int T = Integer.parseInt(args[1]);

        if (N <= 0 || T <= 0) {
            throw new IllegalArgumentException();
        }

        PercolationStats stats = new PercolationStats(N, T);
        StdOut.println("mean                    = " + stats.mean());
        StdOut.println("stddev                  = " + stats.stddev());
        StdOut.println("95% confidence interval = " + stats.confidenceLo() + ", " + stats.confidenceHi());

    }
}