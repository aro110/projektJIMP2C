import org.apache.commons.math3.linear.*;
import java.util.*;

public class SpectralPartitioner {
    private Vertex[] vertices;
    private int parts;
    private double tolerance;

    public SpectralPartitioner(Vertex[] vertices, int parts, double tolerance) {
        this.vertices = vertices;
        this.parts = parts;
        this.tolerance = tolerance; // np. 0.15 dla 15%
    }

    public List<List<Vertex>> partitionGraph() {
        List<Integer> allIndices = new ArrayList<>();
        for (int i = 0; i < vertices.length; i++) {
            allIndices.add(i);
        }

        List<List<Integer>> groups = recursiveBisection(allIndices, parts);

        List<List<Vertex>> partitions = new ArrayList<>();
        for (List<Integer> group : groups) {
            List<Vertex> partition = new ArrayList<>();
            for (int idx : group) {
                partition.add(vertices[idx]);
                vertices[idx].group = partitions.size();
            }
            partitions.add(partition);
        }
        return partitions;
    }

    private List<List<Integer>> recursiveBisection(List<Integer> indices, int targetParts) {
        if (targetParts == 1 || indices.size() <= 1) {
            List<List<Integer>> result = new ArrayList<>();
            result.add(indices);
            return result;
        }

        List<Integer> groupA = new ArrayList<>();
        List<Integer> groupB = new ArrayList<>();

        double[][] laplacian = buildLaplacianSubgraph(indices);
        RealMatrix matrix = new Array2DRowRealMatrix(laplacian);
        EigenDecomposition eig = new EigenDecomposition(matrix);
        double[] fiedler = eig.getEigenvector(laplacian.length - 2).toArray();

        List<Integer> sorted = new ArrayList<>(indices);
        sorted.sort(Comparator.comparingDouble(i -> fiedler[indices.indexOf(i)]));

        // UWZGLĘDNIAMY TOLERANCJĘ
        int size = sorted.size();
        int idealSplit = size / 2;

        int minSplit = (int) Math.floor(size * (0.5 - tolerance));
        int maxSplit = (int) Math.ceil(size * (0.5 + tolerance));

        minSplit = Math.max(1, minSplit); // zabezpieczenie
        maxSplit = Math.min(size - 1, maxSplit);

        int bestSplit = (minSplit + maxSplit) / 2;

        groupA.addAll(sorted.subList(0, bestSplit));
        groupB.addAll(sorted.subList(bestSplit, size));

        int leftParts = targetParts / 2;
        int rightParts = targetParts - leftParts;

        List<List<Integer>> result = new ArrayList<>();
        result.addAll(recursiveBisection(groupA, leftParts));
        result.addAll(recursiveBisection(groupB, rightParts));
        return result;
    }

    private double[][] buildLaplacianSubgraph(List<Integer> indices) {
        int n = indices.size();
        Map<Integer, Integer> indexMap = new HashMap<>();
        for (int i = 0; i < n; i++) {
            indexMap.put(indices.get(i), i);
        }

        double[][] laplacian = new double[n][n];

        for (int i = 0; i < n; i++) {
            int globalI = indices.get(i);
            int degree = 0;

            for (int neighbor : vertices[globalI].conn) {
                if (indexMap.containsKey(neighbor)) {
                    int j = indexMap.get(neighbor);
                    laplacian[i][j] = -1.0;
                    degree++;
                }
            }
            laplacian[i][i] = degree;
        }
        return laplacian;
    }
}
