import java.util.Arrays;


public class Brute {

    public static void main(String[] args) {
        In in = new In(args[0]);
        int N = in.readInt();
        Point[] points = new Point[N];
        
        StdDraw.setXscale(0, 32768);
        StdDraw.setYscale(0, 32768);
        
        for (int i = 0; i < N; i++) {
            int x = in.readInt();
            int y = in.readInt();
            
            Point p = new Point(x, y);
            points[i] = p;
            
            p.draw();
        }
        
        Arrays.sort(points);
        
        for (int i = 0; i < N; i++) {
            for (int j = i + 1; j < N; j++) {
                for (int k = j + 1; k < N; k++) {
                    for (int l = k + 1; l < N; l++) {
                        Point[] pts = new Point[4];
                        pts[0] = points[i];
                        pts[1] = points[j];
                        pts[2] = points[k];
                        pts[3] = points[l];

                        double spq = pts[0].slopeTo(pts[1]);
                        double spr = pts[0].slopeTo(pts[2]);
                        double sps = pts[0].slopeTo(pts[3]);
                        
                        if (spq == spr && spr == sps) {
                            Arrays.sort(pts);
                            StdOut.println(pts[0] + " -> "
                                           + pts[1] + " -> "
                                           + pts[2] + " -> "
                                           + pts[3]);
                            pts[0].drawTo(pts[3]);
                        }
                    }
                }
            }
        }
    }
}
