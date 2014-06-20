/*
import java.io.FileInputStream;
import java.io.FileNotFoundException;
*/

public class Subset {
    public static void main(String []args) {
        int N = Integer.parseInt(args[0]);
        
        /*
        try {
            System.setIn(new FileInputStream(args[1]));
        } catch (FileNotFoundException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
        */
        
        RandomizedQueue<String> rq = new RandomizedQueue<String>();
        
        String[] values = StdIn.readAllStrings();
        
        for (String value : values) {
            rq.enqueue(value);
        }
        
        for (int i = 0; i < N; i++) {
            String value = rq.dequeue();
            StdOut.println(value);
        }
    }
}
