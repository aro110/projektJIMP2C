import java.util.*;

public class GraphConverter {

    public static Vertex[] convertToVertices(int[] xCoords, int[] yOffsets, int[] connections, int[] offsets, int vertexCount) {
        // Tworzenie tablicy wierzchołków
        Vertex[] vertices = new Vertex[vertexCount];
        for (int i = 0; i < vertexCount; i++) {
            vertices[i] = new Vertex(0, 0, 0, 0, xCoords[i], -1);  // xCoords[i] to pozycja wierzchołka, y - na początek -1
        }

        // Ustawianie grupy (y) na podstawie yOffsets
        for (int i = 0; i < yOffsets.length - 1; i++) {
            for (int j = yOffsets[i]; j < yOffsets[i + 1]; j++) {
                vertices[j].y = i;  // Przypisujemy grupę na podstawie zakresu yOffsets
            }
        }

        // Tworzymy listę sąsiedztwa (macierz sąsiedztwa)
        int[][] adjacency = new int[vertexCount][vertexCount];
        for (int i = 0; i < adjacency.length; i++) {
            Arrays.fill(adjacency[i], 0);  // Zainicjalizowanie zerami
        }

        // Dodawanie krawędzi do sąsiedztwa
        for (int i = 0; i < offsets.length - 1; i++) {
            int start = offsets[i];
            int end = offsets[i + 1];

            if (start >= end) continue;

            int from = connections[start];
            for (int j = start + 1; j < end; j++) {
                int to = connections[j];
                if (adjacency[from][to] == 0) {
                    adjacency[from][to] = 1;
                    adjacency[to][from] = 1;
                    vertices[from].edgeNum++;
                    vertices[to].edgeNum++;
                }
            }
        }

        // Przypisanie połączeń do wierzchołków
        for (int i = 0; i < vertexCount; i++) {
            if (vertices[i].edgeNum > 0) {
                vertices[i].conn = new int[vertices[i].edgeNum];  // Alokacja pamięci dla połączeń
                int index = 0;
                for (int j = 0; j < vertexCount; j++) {
                    if (adjacency[i][j] == 1) {
                        vertices[i].conn[index++] = j;
                    }
                }
            }
        }

        return vertices;
    }
}
