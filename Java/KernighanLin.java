import java.util.*;

public class KernighanLin {

    private Vertex[] vertices;
    private int[] partition;
    private int numPartitions = 2;

    public KernighanLin(Vertex[] vertices) {
        this.vertices = vertices;
        this.partition = new int[vertices.length];
    }

    public void kernighanLinPartition() {
        randomInitialPartition();

        boolean improved;
        do {
            improved = false;
            List<Pair> candidates = getBestCandidates();
            for (Pair pair : candidates) {
                if (shouldSwap(pair)) {
                    swapVertices(pair);
                    improved = true;
                }
            }
        } while (improved);

        // Po zakończeniu algorytmu przypisujemy wynikowy podział do grup wierzchołków
        for (int i = 0; i < vertices.length; i++) {
            vertices[i].group = partition[i];
        }
    }

    private void randomInitialPartition() {
        Random rand = new Random();
        for (int i = 0; i < vertices.length; i++) {
            partition[i] = rand.nextInt(2); // Losowy podział
        }
    }

    private List<Pair> getBestCandidates() {
        List<Pair> candidates = new ArrayList<>();
        for (int i = 0; i < vertices.length; i++) {
            for (int j = i + 1; j < vertices.length; j++) {
                if (partition[i] != partition[j]) {
                    int gain = calculateGain(i, j);
                    if (gain > 0) {
                        candidates.add(new Pair(i, j, gain));
                    }
                }
            }
        }
        candidates.sort((p1, p2) -> Integer.compare(p2.gain, p1.gain));
        return candidates;
    }

    private int calculateGain(int v1, int v2) {
        int gain = 0;
        for (int neighbor1 : vertices[v1].conn) {
            if (partition[neighbor1] == partition[v1]) {
                gain++;
            } else {
                gain--;
            }
        }

        for (int neighbor2 : vertices[v2].conn) {
            if (partition[neighbor2] == partition[v2]) {
                gain++;
            } else {
                gain--;
            }
        }

        return gain;
    }

    private boolean shouldSwap(Pair pair) {
        return calculateGain(pair.v1, pair.v2) > 0;
    }

    private void swapVertices(Pair pair) {
        partition[pair.v1] = 1 - partition[pair.v1];
        partition[pair.v2] = 1 - partition[pair.v2];
    }

    private static class Pair {
        int v1, v2, gain;

        Pair(int v1, int v2, int gain) {
            this.v1 = v1;
            this.v2 = v2;
            this.gain = gain;
        }
    }
}
