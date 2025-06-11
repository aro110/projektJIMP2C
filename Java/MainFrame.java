import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.*;
import java.security.NoSuchAlgorithmException;
import java.util.*;
import java.util.List;

public class MainFrame extends JFrame {
    private Config config;
    private JTextField filePathField;
    private JCheckBox forceCheckBox;
    private JComboBox<String> outputFileTypeComboBox;
    private JTextField partsCountField;
    private JTextField errorMarginField;
    private JTextField graphIndexField;
    private JTextField outputFilePathField;

    public MainFrame() {
        config = new Config();
        setTitle("Podział grafu");
        setSize(500, 400);
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        setLocationRelativeTo(null);

        setLayout(new GridBagLayout());
        GridBagConstraints gbc = new GridBagConstraints();
        gbc.insets = new Insets(5, 5, 5, 5);

        JLabel fileLabel = new JLabel("Wybierz plik:");
        gbc.gridx = 0;
        gbc.gridy = 0;
        add(fileLabel, gbc);

        filePathField = new JTextField(20);
        gbc.gridx = 1;
        add(filePathField, gbc);

        forceCheckBox = new JCheckBox("Wymuś podział");
        gbc.gridx = 0;
        gbc.gridy = 2;
        gbc.gridwidth = 2;
        add(forceCheckBox, gbc);

        JLabel outputFileLabel = new JLabel("Typ pliku wyjściowego:");
        gbc.gridx = 0;
        gbc.gridy = 3;
        gbc.gridwidth = 1;
        add(outputFileLabel, gbc);

        String[] outputFileTypes = {"ASCII", "Binary"};
        outputFileTypeComboBox = new JComboBox<>(outputFileTypes);
        gbc.gridx = 1;
        add(outputFileTypeComboBox, gbc);

        JLabel partsCountLabel = new JLabel("Ilość części:");
        gbc.gridx = 0;
        gbc.gridy = 4;
        add(partsCountLabel, gbc);

        partsCountField = new JTextField(5);
        gbc.gridx = 1;
        add(partsCountField, gbc);

        JLabel errorMarginLabel = new JLabel("Margines błędu (%):");
        gbc.gridx = 0;
        gbc.gridy = 5;
        add(errorMarginLabel, gbc);

        errorMarginField = new JTextField(5);
        gbc.gridx = 1;
        add(errorMarginField, gbc);

        JLabel graphIndexLabel = new JLabel("Indeks grafu:");
        gbc.gridx = 0;
        gbc.gridy = 6;
        add(graphIndexLabel, gbc);

        graphIndexField = new JTextField(5);
        gbc.gridx = 1;
        add(graphIndexField, gbc);

        JLabel outputFilePathLabel = new JLabel("Ścieżka do pliku wyjściowego:");
        gbc.gridx = 0;
        gbc.gridy = 7;
        add(outputFilePathLabel, gbc);

        outputFilePathField = new JTextField(20);
        gbc.gridx = 1;
        add(outputFilePathField, gbc);

        JButton runButton = new JButton("Uruchom");
        runButton.addActionListener(e -> runGraphPartition());
        gbc.gridx = 0;
        gbc.gridy = 8;
        gbc.gridwidth = 2;
        add(runButton, gbc);

        JButton loadGraphButton = new JButton("Wczytaj graf");
        loadGraphButton.addActionListener(e -> loadAndVisualizeGraph());
        gbc.gridx = 0;
        gbc.gridy = 9;
        gbc.gridwidth = 2;
        add(loadGraphButton, gbc);
    }

    private void runGraphPartition() {
        try {
            String filePath = filePathField.getText();
            boolean forcePartition = forceCheckBox.isSelected();
            String outputFileType = (String) outputFileTypeComboBox.getSelectedItem();

            int partsCount = Integer.parseInt(partsCountField.getText());
            if (partsCount < 2) {
                JOptionPane.showMessageDialog(this, "Liczba części musi wynosić co najmniej 2!", "Błąd", JOptionPane.ERROR_MESSAGE);
                return;
            }

            int errorMargin = Integer.parseInt(errorMarginField.getText());
            if (errorMargin < 0 || errorMargin > 100) {
                JOptionPane.showMessageDialog(this, "Margines błędu musi być między 0 a 100!", "Błąd", JOptionPane.ERROR_MESSAGE);
                return;
            }

            int graphIndex = Integer.parseInt(graphIndexField.getText());
            if (graphIndex < 0) {
                JOptionPane.showMessageDialog(this, "Indeks grafu musi być liczbą większą lub równą 0!", "Błąd", JOptionPane.ERROR_MESSAGE);
                return;
            }

            String outputFilePath = outputFilePathField.getText();

            GraphReader.readFile(filePath, graphIndex, partsCount, errorMargin);

            int[] xCoords = GraphReader.getXCoords();
            int[] yOffsets = GraphReader.getYOffsets();
            int[] connections = GraphReader.getConnections();
            int[] offsets = GraphReader.getOffsets();

            Vertex[] vertices = GraphConverter.convertToVertices(xCoords, yOffsets, connections, offsets, xCoords.length);

            SpectralPartitioner spectralPartitioner = new SpectralPartitioner(vertices, partsCount, errorMargin);
            List<List<Vertex>> partitions = spectralPartitioner.partitionGraph();

            GraphUtils.fixAndCleanGraph(vertices, forcePartition);

            if (outputFileType.equals("ASCII")) FileOutput.writeAsciiOutput(outputFilePath, vertices);
            else if (outputFileType.equals("Binary")) FileOutput.writeBinaryOutput(outputFilePath, vertices);

            SwingUtilities.invokeLater(() -> visualizeGraph(vertices));

        } catch (NumberFormatException | IOException e) {
            JOptionPane.showMessageDialog(this, "Wprowadź poprawne liczby lub sprawdź plik wejściowy!", "Błąd", JOptionPane.ERROR_MESSAGE);
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException(e);
        }
    }

    private void loadAndVisualizeGraph() {
        JFileChooser fileChooser = new JFileChooser();
        int result = fileChooser.showOpenDialog(this);
        if (result == JFileChooser.APPROVE_OPTION) {
            File file = fileChooser.getSelectedFile();
            try {
                Vertex[] vertices;
                if (file.getName().toLowerCase().endsWith(".txt")) {
                    vertices = FileOutput.readAsciiOutput(file.getAbsolutePath());
                } else {
                    vertices = FileOutput.readBinaryOutput(file.getAbsolutePath());
                }
                visualizeGraph(vertices);
            } catch (Exception e) {
                JOptionPane.showMessageDialog(this, "Błąd wczytywania pliku: " + e.getMessage(), "Błąd", JOptionPane.ERROR_MESSAGE);
            }
        }
    }

    private void visualizeGraph(Vertex[] vertices) {
        JFrame vizFrame = new JFrame("Wizualizacja grafu");
        vizFrame.setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
        vizFrame.add(new GraphVisualizer(vertices));
        vizFrame.pack();
        vizFrame.setLocationRelativeTo(null);
        vizFrame.setVisible(true);
    }

    public static void main(String[] args) {
        SwingUtilities.invokeLater(() -> new MainFrame().setVisible(true));
    }
}
