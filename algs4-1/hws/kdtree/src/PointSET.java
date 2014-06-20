
/**
 * 
 * @author Dragos Tarcatu <tarcatu_dragosh@yahoo.com>
 *
 */
public class PointSET {
   
    private SET<Point2D> points;
    
    public PointSET() {
       // construct an empty set of points
        points = new SET<Point2D>();
   }
   
    /**
     * is the set empty?
     * @return
     */
   public boolean isEmpty() {
       return points.isEmpty();
   }
   
   /**
    * number of points in the set
    * @return
    */
   public int size() {
       return points.size();
   }
   
   /**
    * add the point p to the set (if it is not already in the set)
    * @param p
    */
   public void insert(Point2D p) {
       if (!contains(p)) {
           points.add(p);
       }
   }
   
   /**
    * does the set contain the point p?
    * @param p
    * @return
    */
   public boolean contains(Point2D p) {
       return points.contains(p);
   }
   
   /**
    * draw all of the points to standard draw 
    */
   public void draw() {
       for (Point2D p : points) {
           p.draw();
       }
   }
   
   /**
    * all points in the set that are inside the rectangle
    * @param rect
    * @return
    */
   public Iterable<Point2D> range(RectHV rect) {
       LinkedQueue<Point2D> inside = new LinkedQueue<Point2D>();
       
       for (Point2D point : points) {
           if (rect.contains(point)) {
               inside.enqueue(point);
           }
       }
       
       return inside;
   }
   
   /**
    * a nearest neighbor in the set to p; null if set is empty
    * @param p
    * @return
    */
   public Point2D nearest(Point2D p) {
       if (points.size() == 0) {
           return null;
       }
       
       Point2D champ = points.min();
       
       for (Point2D point : points) {
           if (point.distanceSquaredTo(p) < champ.distanceSquaredTo(p)) {
               champ = point;
           }
       }
       
       return champ;
   }
}
