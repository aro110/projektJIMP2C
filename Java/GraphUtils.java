import java.util.*;
import javax.swing.*;

public class GraphUtils {

    public static void fixAndCleanGraph(Vertex[] vertices, boolean force) {
        int parts = Arrays.stream(vertices).mapToInt(v -> v.group).max().orElse(0) + 1;

        for (Vertex v : vertices) {
            List<Integer> newConn = new ArrayList<>();
            for (int neighbor : v.conn) {
                if (vertices[neighbor].group == v.group) {
                    newConn.add(neighbor);
                }
            }
            v.conn = newConn.stream().mapToInt(i -> i).toArray();
            v.edgeNum = v.conn.length;
        }

        // sprawdzenei czy wierzcholek ma polaczenie
        for (Vertex v : vertices) {
            if (v.edgeNum == 0) {
                boolean moved = false;
                for (int g = 0; g < parts; g++) {
                    if (g == v.group) continue;
                    if (hasConnectionInGroup(vertices, v, g)) {
                        v.group = g;
                        moved = true;
                        break;
                    }
                }

                if (!moved) {
                    if (force) {
                        v.group = (v.group + 1) % parts;
                    } else {
                        SwingUtilities.invokeLater(() -> {
                            JOptionPane.showMessageDialog(
                                    null,
                                    "Podział niemożliwy.\nSpróbuj ponownie z włączoną opcją wymuszenia podziału.",
                                    "Błąd krytyczny",
                                    JOptionPane.ERROR_MESSAGE
                            );
                        });
                        throw new RuntimeException("Fatal error: podział niemożliwy, zatrzymano program.");
                    }
                }

                // filtrowanie polaczen po przeniesieniu
                List<Integer> updatedConn = new ArrayList<>();
                for (int neighbor : v.conn) {
                    if (vertices[neighbor].group == v.group) {
                        updatedConn.add(neighbor);
                    }
                }
                v.conn = updatedConn.stream().mapToInt(i -> i).toArray();
                v.edgeNum = v.conn.length;
            }
        }
    }

    private static boolean hasConnectionInGroup(Vertex[] vertices, Vertex v, int group) {
        for (int neighbor : v.conn) {
            if (vertices[neighbor].group == group) {
                return true;
            }
        }
        return false;
    }
}
