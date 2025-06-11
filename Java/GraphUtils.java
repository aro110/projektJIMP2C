import java.util.*;
import javax.swing.*;

public class GraphUtils {

    public static void fixAndCleanGraph(Vertex[] vertices, boolean force) {

        if (!force) {
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

        for (Vertex v : vertices) {
            if (v.processed == 1) continue;
            else {
                boolean found = false;
                for (int neighbour : v.conn) {
                    if (vertices[neighbour].group == v.group) {
                        vertices[neighbour].processed = 1;
                        v.processed = 1;
                        found = true;
                    }
                }
                if (found) continue;
                v.group = vertices[v.conn[0]].group;
            }
        }

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
    }
}
