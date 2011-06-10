import java.io.FileInputStream;
import java.nio.channels.FileChannel;
import java.util.Arrays;
import java.util.concurrent.*;
import static java.nio.channels.FileChannel.MapMode.READ_ONLY;
import static java.nio.ByteOrder.LITTLE_ENDIAN;


/**
 * 
 * Original Quicksort: https://github.com/pmbauer/parallel/tree/master/src/main/java/pmbauer/parallel
 *
 */
public class ForkJoinQuicksortTask extends RecursiveAction {

    public static void main(String[] args) throws Exception {

        double[] array = new double[Integer.valueOf(args[0])];

        FileChannel fileChannel = new FileInputStream("gaussian.dat").getChannel();
        fileChannel.map(READ_ONLY, 0, fileChannel.size()).order(LITTLE_ENDIAN).asDoubleBuffer().get(array);

        ForkJoinPool mainPool = new ForkJoinPool();

        System.out.println("Starting parallel computation");

        mainPool.invoke(new ForkJoinQuicksortTask(array));        
    }

    private static final long serialVersionUID = -642903763239072866L;
    private static final int SERIAL_THRESHOLD = 0x1000;

    private final double a[];
    private final int left, right;

    public ForkJoinQuicksortTask(double[] a) {this(a, 0, a.length - 1);}

    private ForkJoinQuicksortTask(double[] a, int left, int right) {
        this.a = a;
        this.left = left;
        this.right = right;
    }

    @Override
    protected void compute() {
        if (right - left < SERIAL_THRESHOLD) {
            Arrays.sort(a, left, right + 1);
        } else {
            int pivotIndex = partition(a, left, right);
            ForkJoinTask<Void> t1 = null;

            if (left < pivotIndex)
                t1 = new ForkJoinQuicksortTask(a, left, pivotIndex).fork();
            if (pivotIndex + 1 < right)
                new ForkJoinQuicksortTask(a, pivotIndex + 1, right).invoke();

            if (t1 != null)
                t1.join();
        }
    }

    public static int partition(double[] a, int left, int right) {
        // chose middle value of range for our pivot
        double pivotValue = a[left + (right - left) / 2];

        --left;
        ++right;

        while (true) {
            do
                ++left;
            while (a[left] < pivotValue);

            do
                --right;
            while (a[right] > pivotValue);

            if (left < right) {
                double tmp = a[left];
                a[left] = a[right];
                a[right] = tmp;
            } else {
                return right;
            }
        }
    }    
}
