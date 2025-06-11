import javax.swing.*;
import java.io.*;
import java.util.*;

public class GraphReader {

    private static int[] xCoords;
    private static int[] yOffsets;
    private static int[] connections;
    private static int[] offsets;

    public static void readFile(String inputFilePath, int chooseGraph, int parts, double errorMargin) throws IOException {
        BufferedReader reader = new BufferedReader(new FileReader(inputFilePath));
        List<String> lines = new ArrayList<>();
        String line;

        while ((line = reader.readLine()) != null) {
            lines.add(line);
        }

        int lineCount = lines.size();
        if (lineCount < 5) {
            System.out.println("Błąd: Niepoprawny format pliku. Plik musi zawierać przynajmniej 5 linii danych.");
            System.exit(13);
        }

        if (lineCount > 5 && chooseGraph == 0) {
            System.out.println("Błąd: Wykryto większą ilość grafów w pliku. Zdefiniuj z którego korzystasz.");
            System.exit(24);
        }

        if (lineCount == 5 && chooseGraph != 0) {
            System.out.println("Błąd: Nie można wybrać grafu.");
            System.exit(25);
        }

        if (chooseGraph > (lineCount - 4)) {
            System.out.println("Błąd: Program nie wykrył takiego grafu.");
            System.exit(26);
        }

        int maxMatrix = Integer.parseInt(lines.get(0).trim());

        if (maxMatrix > 1024 || maxMatrix < 0) {
            System.out.println("Błąd: Niepoprawny format pliku wejściowego. Pierwsza linia pliku musi być w przedziale 0-1024.");
            System.exit(13);
        }

        String[] xCoordsStr = lines.get(1).split(";");
        xCoords = new int[xCoordsStr.length];
        for (int i = 0; i < xCoordsStr.length; i++) {
            xCoords[i] = Integer.parseInt(xCoordsStr[i].trim());
        }

        String[] yOffsetsStr = lines.get(2).split(";");
        yOffsets = new int[yOffsetsStr.length];
        for (int i = 0; i < yOffsetsStr.length; i++) {
            yOffsets[i] = Integer.parseInt(yOffsetsStr[i].trim());
        }

        String[] connectionsStr = lines.get(3).split(";");
        connections = new int[connectionsStr.length];
        for (int i = 0; i < connectionsStr.length; i++) {
            connections[i] = Integer.parseInt(connectionsStr[i].trim());
        }

        String[] offsetsStr = lines.get(4).split(";");
        offsets = new int[offsetsStr.length];
        for (int i = 0; i < offsetsStr.length; i++) {
            offsets[i] = Integer.parseInt(offsetsStr[i].trim());
        }

        validateGraphData(maxMatrix, xCoords, xCoords.length, yOffsets, yOffsets.length, connections, connections.length, offsets, offsets.length, parts, errorMargin);
        System.out.println("Plik został pomyślnie wczytany.");

    }


    public static int[] getXCoords() {
        return xCoords;
    }

    public static int[] getYOffsets() {
        return yOffsets;
    }

    public static int[] getConnections() {
        return connections;
    }

    public static int[] getOffsets() {
        return offsets;
    }

    private static void validateGraphData(int maxMatrix, int[] xCoords, int xCount, int[] yOffsets, int yOffsetsCount, int[] connections, int countConn, int[] offsets, int countOffsets, int parts, double errorMargin) {
        for (int i = 0; i < xCount; i++) {
            if (xCoords[i] < 0) {
                JOptionPane.showMessageDialog(null, "Błąd: Pozycja wierzchołka nie może być mniejsza niż 0.", "Błąd", JOptionPane.ERROR_MESSAGE);
                System.exit(13);
            }
        }

        if (yOffsets[yOffsetsCount - 1] != xCount) {
            JOptionPane.showMessageDialog(null, "Błąd: Obliczona liczba wierzchołków z 2 linii nie zgadza się z liczbą z 3 linii.", "Błąd", JOptionPane.ERROR_MESSAGE);
            System.exit(13);
        }

        for (int i = 0; i < yOffsetsCount - 1; i++) {
            if (yOffsets[i] > yOffsets[i + 1]) {
                JOptionPane.showMessageDialog(null, "Błąd: Indeksy pozycji w 3 linii muszą być uporządkowane rosnąco.", "Błąd", JOptionPane.ERROR_MESSAGE);
                System.exit(13);
            }
        }

        for (int i = 0; i < countConn; i++) {
            if (connections[i] < 0 || connections[i] >= xCount) {
                JOptionPane.showMessageDialog(null, "Błąd: Numer wierzchołka w 4 linii nie może być większy niż ogólna liczba wierzchołków.", "Błąd", JOptionPane.ERROR_MESSAGE);
                System.exit(13);
            }
        }

        if (offsets[countOffsets - 1] > countConn) {
            JOptionPane.showMessageDialog(null, "Błąd: Ostatni indeks połączenia w 5 linii nie może być większy niż liczba połączeń.", "Błąd", JOptionPane.ERROR_MESSAGE);
            System.exit(13);
        }

        for (int i = 0; i < countOffsets - 1; i++) {
            if (offsets[i] > offsets[i + 1] || offsets[i] > countConn) {
                JOptionPane.showMessageDialog(null, "Błąd: Indeksy połączeń w 5 linii muszą być uporządkowane rosnąco, oraz nie mogą przekraczać liczby połączeń.", "Błąd", JOptionPane.ERROR_MESSAGE);
                System.exit(13);
            }
        }

        if ((xCount / 2) < parts || xCount < 4) {
            JOptionPane.showMessageDialog(null, "Błąd: Za mała liczba wierzchołków w grafie.", "Błąd", JOptionPane.ERROR_MESSAGE);
            System.exit(13);
        }

    }
}
