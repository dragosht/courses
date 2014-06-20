import java.util.Iterator;
import java.util.NoSuchElementException;

/**
 * 
 * Basic Randomized queue generic implementation. 
 * @author Dragos Tarcatu <tarcatu_dragosh@yahoo.com>
 *
 */
public class RandomizedQueue<Item> implements Iterable<Item> {
    
    /**
     * Randomized queue iterator.
     * @author Dragos Tarcatu <tarcatu_dragosh@yahoo.com>
     *
     */
    private class RandomizedQueueIterator implements Iterator<Item> { 
        
        private int[] permutation;
        private int current;
        
        RandomizedQueueIterator(Item[] items, int size) {
            permutation = new int[size];
            current = 0;
            
            for (int i = 0; i < size; i++) {
                permutation[i] = i;
            }
            
            for (int i = 0; i < size; i++) {
                int idx = StdRandom.uniform(size);
                int xchg = permutation[i];
                permutation[i] = permutation[idx];
                permutation[idx] = xchg;
            }           
        }
        
        @Override
        public boolean hasNext() {
            return current < size;
        }

        @Override
        public Item next() {
            if (current == size) {
                throw new NoSuchElementException();
            }
            Item item = null;
            
            if (current < permutation.length) {
                item = items[permutation[current++]];
            } else {
                item = items[current++];
            }
            
            return item;
        }

        @Override
        public void remove() {
            throw new UnsupportedOperationException();
        }
        
    }
    
    private Item[] items;
    private int size;
    
    private void resize(int capacity) {
        @SuppressWarnings("unchecked")
        Item[] copy = (Item[]) new Object[capacity];
        for (int i = 0; i < size; i++) {
            copy[i] = items[i];
        }
        items = copy;
    }
    
    /**
     * Construct an empty randomized queue.
     */
    @SuppressWarnings("unchecked")
    public RandomizedQueue() {
        items = (Item[]) new Object[2];
        size = 0;
    }

    public boolean isEmpty() {
        return size == 0;
    }

    public int size() {
        return size;
    }

    /**
     * Add the item.
     * @param item to be enqueued.
     */
    public void enqueue(Item item) {

        
        if (item == null) {
            throw new NullPointerException();
        }
        
        if (size == items.length) {
            resize(2 * items.length);
        }
        
        items[size++] = item;
    }

    /**
     * Delete and return a random item
     * @return the just deleted item.
     */
    public Item dequeue() {
       
        if (size == 0) {
            throw new NoSuchElementException();
        }
        
        int idx = StdRandom.uniform(size);
        
        Item item = items[idx];
        items[idx] = items[size - 1];
        items[size - 1] = null;
        size--;
        
        if (size > 0 && size == items.length/4) {
            resize(items.length / 2);
        }
        
        return item;
    }

    /**
     * Return (but do not delete) a random item
     * @return the sampled item.
     */
    public Item sample() {
        
        if (size == 0) {
            throw new NoSuchElementException();
        }
        
        int idx = StdRandom.uniform(size);
        return items[idx];
    }

    public Iterator<Item> iterator() {
        return new RandomizedQueueIterator(items, size);
    }

    public static void main(String[] args) {
        // unit testing
        StdOut.println("Testing");
        
        RandomizedQueue<Integer> rq = new RandomizedQueue<Integer>();
        rq.enqueue(7);
        rq.enqueue(9);
        rq.enqueue(8);
        rq.enqueue(4);
        rq.enqueue(3);
        
        rq.dequeue();
        rq.dequeue();
        rq.dequeue();
        
        Iterator<Integer> it = rq.iterator();
        
        while (it.hasNext()) {
            StdOut.println(it.next());
        }
        
        StdOut.println("Size: " + rq.size());
    }
}