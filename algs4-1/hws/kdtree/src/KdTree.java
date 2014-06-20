
/**
 * 
 * @author Dragos Tarcatu <tarcatu_dragosh@yahoo.com>
 *
 */
public class KdTree {
    
    private static enum Direction {
        VERTICAL,
        HORIZONTAL
    };

    private static class Node2D {
        private Point2D point;
        private Direction dir;
        private Node2D lb, rt;
        RectHV rect;
        
        public Node2D(Point2D point, Direction dir, Node2D lb, Node2D rt, RectHV rect) {
            this.point = point;
            this.dir = dir;
            this.lb = lb;
            this.rt = rt;
            this.rect = rect;
        }
    };

    private Node2D root;
    private int size;
    
    /**
     * construct an empty KdTree
     */
    public KdTree() {
        root = null;
        size = 0;
    }

    /**
     * is this 2d tree empty?
     * @return
     */
    public boolean isEmpty() {
        return (size == 0);
    }

    /**
     * number of points in the set
     * @return
     */
    public int size() {
        return size;
    }

    
    private Node2D insert(Node2D node, Point2D point, Direction direction, RectHV rect) {
        if (node == null) {
            return new Node2D(point, direction, null, null, rect);
        }
        
        if (node.dir == Direction.VERTICAL) {
            if (point.x() < node.point.x()) {
                RectHV r = new RectHV(rect.xmin(), rect.ymin(), node.point.x(), rect.ymax());
                node.lb = insert(node.lb,  point, Direction.HORIZONTAL, r);
            } else {
                RectHV r = new RectHV(node.point.x(), rect.ymin(), rect.xmax(), rect.ymax());
                node.rt = insert(node.rt, point, Direction.HORIZONTAL, r);
            }
        } else {
            if (point.y() < node.point.y()) {
                RectHV r = new RectHV(rect.xmin(), rect.ymin(), rect.xmax(), node.point.y());
                node.lb = insert(node.lb, point, Direction.VERTICAL, r);
            } else {
                RectHV r = new RectHV(rect.xmin(), node.point.y(), rect.xmax(), rect.ymax());
                node.rt = insert(node.rt, point, Direction.VERTICAL, r);
            }
        }
        
        return node;
    }
    
    /**
     * add the point p to the set (if it is not already in the set)
     * @param p
     */
    public void insert(Point2D p) {
        assert p.x() >= 0 && p.x() <= 1 && p.y() >= 0 && p.y() <= 1;
    
        if (!contains(p)) {
            root = insert(root, p, Direction.VERTICAL, new RectHV(0, 0, 1, 1));
            size++;
        }
    }

    private Node2D get(Node2D node, Point2D point) {
        if (node == null) {
            return null;
        }

        if (node.point.equals(point))
            return node;
        
        if (node.dir == Direction.VERTICAL) {
            if (point.x() < node.point.x()) {
                return get(node.lb, point);
            } else {
                return get(node.rt, point);
            }
        } else {
            if (point.y() < node.point.y()) {
                return get(node.lb, point);
            } else {
                return get(node.rt, point);
            }
        }
    }
    
    
    /**
     * does the set contain the point p?
     * @param p
     * @return
     */
    public boolean contains(Point2D p) {
        return (get(root, p) != null);
    }
     
    private void draw(Node2D node) {
        if (node == null) {
            return;
        }
        
        //StdOut.println("Draw: " + node.point.x() + " " + node.point.y());
        
        if (node.dir == Direction.VERTICAL) {
            //StdOut.println("Vertical - drawline");
            StdDraw.setPenColor(StdDraw.RED);
            StdDraw.setPenRadius();
            Point2D from = new Point2D(node.point.x(), node.rect.ymin());
            Point2D to = new Point2D(node.point.x(), node.rect.ymax());
            from.drawTo(to);
        } else {
            //StdOut.println("Horizontal - drawline");
            StdDraw.setPenColor(StdDraw.BLUE);
            StdDraw.setPenRadius();
            Point2D from = new Point2D(node.rect.xmin(), node.point.y());
            Point2D to = new Point2D(node.rect.xmax(), node.point.y());
            from.drawTo(to);
        }
        
        StdDraw.setPenColor(StdDraw.BLACK);
        StdDraw.setPenRadius(.01);
        node.point.draw();
        
        draw(node.lb);
        draw(node.rt);
    }
    
    /**
     * draw all of the points to standard draw
     */
    public void draw() {
        draw(root);
    }

    
    private void range(Node2D node, LinkedQueue<Point2D> pts, RectHV rect) {
        if (node == null) {
            return;
        }
        
        if (rect.contains(node.point)) {
            pts.enqueue(node.point);
        }
        
        if (node.dir == Direction.VERTICAL) {
            if (rect.xmin() < node.point.x()) {
                range(node.lb, pts, rect);
            }
            if (rect.xmax() > node.point.x()) {
                range(node.rt, pts, rect);
            }
        } else {
            if (rect.ymin() < node.point.y()) {
                range(node.lb, pts, rect);
            }
            if (rect.ymax() > node.point.y()) {
                range(node.rt, pts, rect);
            }
        }
    }
    
    /**
     * all points in the set that are inside the rectangle
     * @param rect
     * @return
     */
    public Iterable<Point2D> range(RectHV rect) {
        LinkedQueue<Point2D> pts = new LinkedQueue<Point2D>();
        range(root, pts, rect);
        return pts;
    }
    
    
    private double clamp(double val, double min, double max) {
        return Math.max(min, Math.min(max, val));
    }
    
    private boolean intersects(RectHV rect, Point2D point, double radius) {
        double closestX = clamp(point.x(), rect.xmin(), rect.xmax());
        double closestY = clamp(point.y(), rect.ymin(), rect.ymax());
        double distanceX = point.x() - closestX;
        double distanceY = point.y() - closestY;
        double distanceSquared = (distanceX * distanceX) + (distanceY * distanceY);
        return distanceSquared < (radius * radius);
    }
    
    private Point2D nearest(Node2D root, Point2D query) {
        if (root == null) {
            return null;
        }
        
        Point2D champ = root.point;
        double best = query.distanceTo(champ);
        
        LinkedQueue<Node2D> nodes = new LinkedQueue<Node2D>();
        nodes.enqueue(root);
        
        while (!nodes.isEmpty()) {
            Node2D node = nodes.dequeue();
            
            if (query.distanceSquaredTo(champ) > query.distanceSquaredTo(node.point)) {
                champ = node.point;
                best = query.distanceTo(champ);
            }
            
            if (node.dir == Direction.VERTICAL) {
                if (node.lb != null && intersects(node.lb.rect, query, best)) {
                    nodes.enqueue(node.lb);
                }
                if (node.rt != null && intersects(node.rt.rect, query, best)) {
                    nodes.enqueue(node.rt);
                }
            } else {
                if (node.lb != null && intersects(node.lb.rect, query, best)) {
                    nodes.enqueue(node.lb);
                }
                if (node.rt != null && intersects(node.rt.rect, query, best)) {
                    nodes.enqueue(node.rt);
                }
            }
        }
        
        return champ;
    }
    
    /**
     * a nearest neighbor in the set to p; null if set is empty
     * @param p
     * @return
     */
    public Point2D nearest(Point2D p) {
        return nearest(root, p);
    }
}