import java.util.Arrays;


public class Fast {
    
    
    //@SuppressWarnings("unused")
    private static void printSlopes(Point[] p) {
        StdOut.print(p[0] + " : ");
        for (int i = 1; i < p.length; i++) {
            StdOut.print(p[0].slopeTo(p[i]) + " ");
        }
        StdOut.println();
    }
    
    
    public static void main(String[] args) {
        In in = new In(args[0]);
        int N = in.readInt();
        Point[] points = new Point[N];
        Point[] pts = new Point[N];
        Point pivot = null;

        StdDraw.setXscale(0, 32768);
        StdDraw.setYscale(0, 32768);
        
        for (int i = 0; i < N; i++) {
            int x = in.readInt();
            int y = in.readInt();
            
            Point p = new Point(x, y);
            points[i] = p;
            pts[i] = p;
            p.draw();
        }
        
        Arrays.sort(points);
        
        for (int i = 0; i < N; i++) {
            
            pivot = points[i];
            Arrays.sort(pts, 0, pts.length, pivot.SLOPE_ORDER);
            
            //printSlopes(pts);
            
            int start = 0;

            while (start < N)
            {
                int stop = start + 1;
                if (stop >= N) {
                    break;
                }
                
                double slope = pts[0].slopeTo(pts[start]);
                
                while (stop < N && pts[0].slopeTo(pts[stop]) == slope) {
                    stop++;
                }
                
                int count = stop - start + 1;
                
                if (count >= 4) {
                    
                    Point[] collinear = new Point[count];
                    for (int k = 0; k < count - 1; k++) {
                        collinear[k] = pts[start + k];
                    }
                    collinear[count - 1] = pts[0];
                    Arrays.sort(collinear);

                    if (collinear[0].compareTo(pivot) >= 0) {
                        for (int k = 0; k < count - 1; k++) {
                            StdOut.print(collinear[k] + " -> ");
                        }
                        StdOut.println(collinear[count - 1]);
                        collinear[0].drawTo(collinear[count - 1]);
                    }
                }
                
                start = stop;
            }
        }
    }
}
