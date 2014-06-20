
import java.util.Iterator;
import java.util.NoSuchElementException;

/**
 * 
 * Class deque. Implements a basic doubly ended queue.
 * @author Dragos Tarcatu <tarcatu_dragosh@yahoo.com>
 *
 */
public class Deque<Item> implements Iterable<Item> {
    
    /**
     * Class Node. Use an underlying doubly-linked list representation
     * to keep the deque. 
     * @author Dragos Tarcatu
     */
    private class Node {
        public Item item;
        public Node next;
        public Node prev;
        
        public Node(Item item, Node next, Node prev) {
            this.item = item;
            this.next = next;
            this.prev = prev;
        }
    };
    
    /**
     * 
     * Class DequeIterator - implement iterating facitilies on the Deque container.
     * @author Dragos Tarcatu
     *
     */
    private class DequeIterator implements Iterator<Item> {
        private Node current = first;
        
        @Override
        public boolean hasNext() {
            return current != null;
        }
        
        @Override
        public void remove() {
            throw new UnsupportedOperationException();
        }
        
        @Override
        public Item next() {
            if (current == null) {
                throw new NoSuchElementException();
            }
            Item item = current.item;
            current = current.next;
            return item;
        }
    };
    
    private Node first;
    private Node last;
    private int size;
    
    
    /**
     * Construct an empty deque.
     */
    public Deque() {
        first = null;
        last = null;
        size = 0;
    }
    
    /**
     * Is the deque empty?
     * @return true if the deque is empty, false otherwise
     */
    public boolean isEmpty() {
        return (size == 0);
    }
    
    /**
     * @return the number of items in the deque.
     */
    public int size() {
        return size;
    }
    
    /**
     * Insert the item at the front.
     * @param item - the new item to be inserted
     */
    public void addFirst(Item item) {
        if (item == null) {
            throw new NullPointerException(); 
        }
        
        Node newfirst = new Node(item, first, null);
        if (first != null) {
            first.prev = newfirst;
        }
        first = newfirst;
        
        if (last == null) {
            last = first;
        }
        
        size++;
    }
    
    /**
     * Insert the item at the end.
     * @param item - the new item to be inserted
     */
    public void addLast(Item item) {
        if (item == null) {
            throw new NullPointerException();
        }
        
        Node newlast = new Node(item, null, last);
        if (last != null) {
            last.next = newlast;
        }
        last = newlast;
        
        if (first == null) {
            first = last;
        }
        
        size++;
    }
    
    /**
     * Delete and return the item at the front.
     * @return the first item.
     */
    public Item removeFirst() {
        if (size == 0) {
            throw new NoSuchElementException();
        }
        
        assert first != null;
        
        Item item = first.item;
        if (size == 1) {
            first = null;
            last = null;
        } else {
            first = first.next;
            first.prev = null;
        }
        size--;
        
        return item;
    }
    
    /**
     * Delete and return the item at the end.
     * @return the last item.
     */
    public Item removeLast() {
        if (size == 0) {
            throw new NoSuchElementException();
        }
        
        assert last != null;
        
        Item item = last.item;
        if (size == 1) {
            first = null;
            last = null;
        } else {
            last = last.prev;
            last.next = null;
        }
        size--;
        
        return item;
    }
    
    /**
     * Return an iterator over items in order from front to end.
     */
    public Iterator<Item> iterator() {
        return new DequeIterator();
    }
    
    
    public static void main(String[] args) {
        // unit testing
        
        StdOut.println("Testing");
        
        Deque<Integer> deque = new Deque<Integer>();
        deque.addFirst(5);
        Iterator<Integer> it = deque.iterator();
        deque.addFirst(4);
        deque.addFirst(9);
        
        while (it.hasNext()) {
            StdOut.println(it.next());
        }
        
    }
}

