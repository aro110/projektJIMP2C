import javax.swing.*;
import javax.swing.filechooser.FileNameExtensionFilter;
import java.awt.*;
import java.awt.event.*;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.IOException;
import java.util.*;
import java.util.List;
import javax.imageio.ImageIO;

public class GraphVisualizer extends JPanel {

    private Vertex[] vertices;
    private int padding = 50;
    private double zoom = 1.0;
    private double minZoom = 0.1;
    private int offsetX = 0, offsetY = 0;
    private int lastMouseX, lastMouseY;
    private int nodeSize = 30;

    private int canvasWidth = 1000, canvasHeight = 1000;
    private int maxX = 0, maxY = 0;

    private Map<Integer, Boolean> groupVisibility = new HashMap<>();
    private Map<Integer, Color> groupColors = new HashMap<>();
    private Set<Color> usedColors = new HashSet<>();

    private JPanel statsPanel;

    public GraphVisualizer(Vertex[] vertices) {
        this.vertices = vertices;
        analyzeGraphSize();
        generateGroupColors();
        calculateMinZoom();

        setLayout(new BorderLayout());
        JPanel drawPanel = new DrawPanel();
        add(new JScrollPane(drawPanel), BorderLayout.CENTER);
        add(createSidePanel(drawPanel), BorderLayout.EAST);
        addMouseInteraction(drawPanel);

        addComponentListener(new ComponentAdapter() {
            @Override
            public void componentResized(ComponentEvent e) {
                calculateMinZoom();  // Ponownie obliczamy minimalny zoom przy zmianie rozmiaru okna
            }
        });

    }

    private void analyzeGraphSize() {
        for (Vertex v : vertices) {
            maxX = Math.max(maxX, v.x);
            maxY = Math.max(maxY, v.y);
        }
    }

    private void calculateMinZoom() {
        double sx = (canvasWidth - 2.0 * padding) / (double) (maxX + 1);  // Dostosowanie do szerokości
        double sy = (canvasHeight - 2.0 * padding) / (double) (maxY + 1); // Dostosowanie do wysokości
        minZoom = Math.min(sx, sy);  // Wybieramy mniejszy współczynnik, aby cały graf się zmieścił
        minZoom = Math.min(minZoom, 1.0);  // Ograniczamy zoom, aby nie przekroczył 1
        resetCamera();
    }

    private void resetCamera() {
        zoom = minZoom;  // Ustawienie zoomu na minimalną wartość, która sprawia, że graf się mieści
        offsetX = 0;
        offsetY = 0;
        nodeSize = 30;
        repaint();
    }



    private void generateGroupColors() {
        Set<Integer> groups = new HashSet<>();
        for (Vertex v : vertices) {
            groups.add(v.group);
        }
        for (int group : groups) {
            Color color = generateUniquePastelColor();
            groupColors.put(group, color);
            groupVisibility.put(group, true);
        }
    }

    private Color generateUniquePastelColor() {
        Random rand = new Random();
        while (true) {
            int r = rand.nextInt(127) + 128;
            int g = rand.nextInt(127) + 128;
            int b = rand.nextInt(127) + 128;
            Color color = new Color(r, g, b);
            if (!usedColors.contains(color)) {
                usedColors.add(color);
                return color;
            }
        }
    }

    private JPanel createSidePanel(JPanel drawPanel) {
        JPanel panel = new JPanel(new BorderLayout());
        panel.setBorder(BorderFactory.createTitledBorder("Opcje wyświetlania"));
        panel.setPreferredSize(new Dimension(260, getHeight()));

        // Grupy
        JPanel groupPanel = new JPanel();
        groupPanel.setLayout(new BoxLayout(groupPanel, BoxLayout.Y_AXIS));
        groupPanel.setBorder(BorderFactory.createTitledBorder("Widoczne grupy"));

        for (int group : groupColors.keySet()) {
            JCheckBox checkBox = new JCheckBox("Grupa " + group, true);
            checkBox.setBackground(groupColors.get(group));
            int finalG = group;
            checkBox.addItemListener(e -> {
                groupVisibility.put(finalG, e.getStateChange() == ItemEvent.SELECTED);
                updateStats();
                repaint();
            });
            groupPanel.add(checkBox);
        }

        JScrollPane scrollPane = new JScrollPane(groupPanel);
        panel.add(scrollPane, BorderLayout.CENTER);


        // Wewnątrz metody createSidePanel
        JPanel vertexSizePanel = new JPanel();
        vertexSizePanel.setLayout(new BoxLayout(vertexSizePanel, BoxLayout.Y_AXIS));
        vertexSizePanel.setBorder(BorderFactory.createTitledBorder("Zmiana rozmiaru wierzchołków"));

// Dodajemy suwak
        JSlider vertexSizeSlider = new JSlider(JSlider.HORIZONTAL, 10, 100, nodeSize);  // Zakres od 10 do 100
        vertexSizeSlider.setMajorTickSpacing(10);
        vertexSizeSlider.setMinorTickSpacing(1);
        vertexSizeSlider.setPaintTicks(true);
        vertexSizeSlider.setPaintLabels(true);

// Dodajemy listener
        vertexSizeSlider.addChangeListener(e -> {
            int newSize = vertexSizeSlider.getValue();
            updateVertexSize(newSize);
        });
        vertexSizePanel.add(vertexSizeSlider);

// Dodajemy do panelu
        panel.add(vertexSizePanel, BorderLayout.SOUTH);

        // Statystyki
        statsPanel = new JPanel();
        statsPanel.setLayout(new BoxLayout(statsPanel, BoxLayout.Y_AXIS));
        statsPanel.setBorder(BorderFactory.createTitledBorder("Statystyki"));
        updateStats();
        panel.add(statsPanel, BorderLayout.NORTH);

        // Sterowanie + eksport
        JButton resetButton = new JButton("Reset widoku");
        resetButton.addActionListener(e -> resetCamera());

        JButton exportButton = new JButton("Eksportuj PNG");
        exportButton.addActionListener(e -> exportToImage(drawPanel));

        JPanel controlPanel = new JPanel();
        controlPanel.setBorder(BorderFactory.createTitledBorder("Inne"));
        controlPanel.add(resetButton);
        controlPanel.add(exportButton);
        panel.add(controlPanel, BorderLayout.SOUTH);

        return panel;
    }

    private void updateVertexSize(int newSize) {
        nodeSize = newSize;  // Aktualizujemy globalny rozmiar wierzchołków
        repaint();  // Przerysowujemy cały panel
    }


    private void updateStats() {
        statsPanel.removeAll();
        Map<Integer, Integer> groupCounts = new HashMap<>();
        int visibleVertices = 0;
        int visibleEdges = 0;

        // Liczymy wierzchołki na grupy i sumujemy aktualnie widoczne
        for (Vertex v : vertices) {
            groupCounts.put(v.group, groupCounts.getOrDefault(v.group, 0) + 1);
            if (groupVisibility.getOrDefault(v.group, true)) {
                visibleVertices++;
            }
        }

        // Dodajemy etykiety grupowe
        for (int group : groupColors.keySet()) {
            JLabel label = new JLabel("Grupa " + group + ": " + groupCounts.getOrDefault(group, 0) + " wierzchołków");
            statsPanel.add(label);
        }

        // Liczymy krawędzie widoczne (bez podwójnego liczenia)
        Set<String> countedEdges = new HashSet<>();
        for (Vertex v : vertices) {
            if (!groupVisibility.getOrDefault(v.group, true)) continue;
            for (int neighborIndex : v.conn) {
                Vertex neighbor = vertices[neighborIndex];
                if (!groupVisibility.getOrDefault(neighbor.group, true)) continue;
                String key = v.hashCode() < neighbor.hashCode()
                        ? v.hashCode() + "-" + neighbor.hashCode()
                        : neighbor.hashCode() + "-" + v.hashCode();
                if (!countedEdges.contains(key)) {
                    countedEdges.add(key);
                    visibleEdges++;
                }
            }
        }

        // Dodajemy podsumowanie globalne
        JLabel totalLabel = new JLabel("Widoczne: " + visibleVertices + " wierzchołków, " + visibleEdges + " krawędzi");
        statsPanel.add(Box.createVerticalStrut(10));
        statsPanel.add(totalLabel);

        statsPanel.revalidate();
        statsPanel.repaint();
    }

    private void exportToImage(JPanel drawPanel) {
        JFileChooser fileChooser = new JFileChooser();
        fileChooser.setDialogTitle("Zapisz jako");
        fileChooser.setFileFilter(new FileNameExtensionFilter("PNG Images", "png"));
        int userSelection = fileChooser.showSaveDialog(this);
        if (userSelection == JFileChooser.APPROVE_OPTION) {
            File file = fileChooser.getSelectedFile();
            if (!file.getAbsolutePath().endsWith(".png")) {
                file = new File(file.getAbsolutePath() + ".png");
            }
            BufferedImage image = new BufferedImage(drawPanel.getWidth(), drawPanel.getHeight(), BufferedImage.TYPE_INT_ARGB);
            Graphics2D g2 = image.createGraphics();
            drawPanel.paint(g2);
            g2.dispose();
            try {
                ImageIO.write(image, "png", file);
                JOptionPane.showMessageDialog(this, "Plik zapisany: " + file.getAbsolutePath());
            } catch (IOException ex) {
                JOptionPane.showMessageDialog(this, "Błąd podczas zapisu: " + ex.getMessage());
            }
        }
    }

    private void addMouseInteraction(JPanel panel) {
        panel.addMouseWheelListener(e -> {
            if (e.getPreciseWheelRotation() < 0) {
                zoom *= 1.1;
            } else {
                zoom /= 1.1;
            }
            zoom = Math.max(minZoom, zoom);
            repaint();
        });

        panel.addMouseListener(new MouseAdapter() {
            @Override
            public void mousePressed(MouseEvent e) {
                lastMouseX = e.getX();
                lastMouseY = e.getY();
            }
        });

        panel.addMouseMotionListener(new MouseAdapter() {
            @Override
            public void mouseDragged(MouseEvent e) {
                offsetX += e.getX() - lastMouseX;
                offsetY += e.getY() - lastMouseY;
                lastMouseX = e.getX();
                lastMouseY = e.getY();
                clampOffsets(panel.getWidth(), panel.getHeight());
                repaint();
            }
        });
    }

    private void applyRepulsion(Vertex[] vertices, double scaleX, double scaleY) {
        final double repulsionForce = 0.01; // Siła odpychania, im większa tym większe odległości
        for (int i = 0; i < vertices.length; i++) {
            Vertex v = vertices[i];
            double x1 = v.x * scaleX;
            double y1 = v.y * scaleY;

            for (int j = i + 1; j < vertices.length; j++) {
                Vertex u = vertices[j];
                double x2 = u.x * scaleX;
                double y2 = u.y * scaleY;

                double dx = x1 - x2;
                double dy = y1 - y2;
                double distance = Math.sqrt(dx * dx + dy * dy);

                if (distance < 50) {
                    double force = repulsionForce / distance;
                    v.x += (int) (force * dx);
                    v.y += (int) (force * dy);
                    u.x -= (int) (force * dx);
                    u.y -= (int) (force * dy);
                }
            }
        }
    }


    private void clampOffsets(int width, int height) {
        double scaleX = ((width - 2.0 * padding) / (double) (maxX + 1)) * zoom;
        double scaleY = ((height - 2.0 * padding) / (double) (maxY + 1)) * zoom;

        int maxOffsetX = (int)((maxX + 1) * scaleX / 2);
        int maxOffsetY = (int)((maxY + 1) * scaleY / 2);

        offsetX = Math.max(-maxOffsetX, Math.min(offsetX, maxOffsetX));
        offsetY = Math.max(-maxOffsetY, Math.min(offsetY, maxOffsetY));
    }

    private class DrawPanel extends JPanel {

        private Vertex hoveredVertex = null;
        private Point mousePos = null;

        public DrawPanel() {
            setPreferredSize(new Dimension(canvasWidth, canvasHeight));

            addMouseMotionListener(new MouseMotionAdapter() {
                @Override
                public void mouseMoved(MouseEvent e) {
                    mousePos = e.getPoint();
                    hoveredVertex = findHoveredVertex(e.getPoint());
                    repaint();
                }
            });
        }

        private Vertex findHoveredVertex(Point mousePoint) {
            if (vertices == null) return null;

            double scaleX = ((getWidth() - 2.0 * padding) / (double)(maxX + 1)) * zoom;
            double scaleY = ((getHeight() - 2.0 * padding) / (double)(maxY + 1)) * zoom;

            int nodeSize = (int)(30 * zoom);
            for (int i = 0; i < vertices.length; i++) {
                Vertex v = vertices[i];
                if (!groupVisibility.getOrDefault(v.group, true)) continue;
                int px = (int)(padding + v.x * scaleX + offsetX);
                int py = (int)(padding + v.y * scaleY + offsetY);

                double dist = mousePoint.distance(px, py);
                if (dist <= nodeSize / 2.0) {
                    return v;
                }
            }
            return null;
        }

        @Override
        protected void paintComponent(Graphics g) {
            super.paintComponent(g);
            if (vertices == null || vertices.length == 0) return;

            Graphics2D g2d = (Graphics2D) g;
            g2d.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);

            double scaleX = ((getWidth() - 2.0 * padding) / (double) (maxX + 1)) * zoom;
            double scaleY = ((getHeight() - 2.0 * padding) / (double) (maxY + 1)) * zoom;

            // Zastosowanie siły odpychania
            applyRepulsion(vertices, scaleX, scaleY);

            Map<Vertex, Point> pos = new HashMap<>();
            for (Vertex v : vertices) {
                if (!groupVisibility.getOrDefault(v.group, true)) continue; // Rysuj tylko widoczne grupy

                int px = (int)(padding + v.x * scaleX + offsetX);
                int py = (int)(padding + v.y * scaleY + offsetY);
                pos.put(v, new Point(px, py));
            }

            // Zbiór wierzchołków sąsiadów dla podświetlania
            Set<Vertex> neighbors = new HashSet<>();
            if (hoveredVertex != null) {
                for (int neighborIndex : hoveredVertex.conn) {
                    neighbors.add(vertices[neighborIndex]);
                }
            }

            // Rysowanie krawędzi
            g2d.setStroke(new BasicStroke(1));
            g2d.setColor(new Color(150, 150, 150, 50));
            for (Vertex v : vertices) {
                if (!groupVisibility.getOrDefault(v.group, true)) continue;

                Point p1 = pos.get(v);
                for (int neighborIndex : v.conn) {
                    Vertex neighbor = vertices[neighborIndex];

                    // Podświetlanie krawędzi (w zależności od najechanego wierzchołka)
                    if (hoveredVertex != null && (v.equals(hoveredVertex) || neighbor.equals(hoveredVertex))) {
                        g2d.setColor(new Color(255, 100, 0, 200));  // Krawędź podświetlona na pomarańczowo
                    } else {
                        g2d.setColor(new Color(150, 150, 150, 50));  // Domyślna krawędź
                    }

                    if (!groupVisibility.getOrDefault(neighbor.group, true)) continue;

                    Point p2 = pos.get(neighbor);
                    g2d.drawLine(p1.x, p1.y, p2.x, p2.y);
                }
            }

            // Rysowanie wierzchołków
            for (int i = 0; i < vertices.length; i++) {
                Vertex v = vertices[i];
                if (!groupVisibility.getOrDefault(v.group, true)) continue;
                Point p = pos.get(v);

                // Podświetlanie wierzchołka
                if (v.equals(hoveredVertex)) {
                    g2d.setColor(Color.ORANGE);  // Podświetlenie wierzchołka
                } else if (neighbors.contains(v)) {
                    g2d.setColor(new Color(255, 180, 80));  // Kolor dla sąsiadów
                } else {
                    g2d.setColor(groupColors.get(v.group));  // Kolor wierzchołka wg grupy
                }

                // Rysowanie wierzchołka
                g2d.fillOval(p.x - nodeSize / 2, p.y - nodeSize / 2, nodeSize, nodeSize);
                g2d.setColor(Color.BLACK);
                g2d.drawOval(p.x - nodeSize / 2, p.y - nodeSize / 2, nodeSize, nodeSize);

                // Etykieta wierzchołka
                int fontSize = Math.max(8, nodeSize / 2);
                g2d.setFont(new Font("Arial", Font.BOLD, fontSize));
                FontMetrics fm = g2d.getFontMetrics();
                String label = String.valueOf(i);
                int lw = fm.stringWidth(label);
                int lh = fm.getAscent();
                g2d.drawString(label, p.x - lw / 2, p.y + lh / 2 - 3);
            }

            // Rysowanie tooltipa dla wierzchołka
            if (hoveredVertex != null && mousePos != null) {
                String tooltipText = String.format(
                        "Wierzchołek %d\nGrupa: %d\nPołączenia: %d\nSąsiedzi: %s",
                        Arrays.asList(vertices).indexOf(hoveredVertex),
                        hoveredVertex.group,
                        hoveredVertex.conn.length,
                        Arrays.toString(hoveredVertex.conn)
                );

                String[] lines = tooltipText.split("\n");
                int tooltipWidth = 0;
                FontMetrics fm = g2d.getFontMetrics();
                for (String line : lines) {
                    tooltipWidth = Math.max(tooltipWidth, fm.stringWidth(line));
                }
                int tooltipHeight = lines.length * fm.getHeight();

                int tx = mousePos.x + 10;
                int ty = mousePos.y + 10;

                g2d.setColor(new Color(255, 255, 229, 230));
                g2d.fillRoundRect(tx, ty, tooltipWidth + 10, tooltipHeight + 10, 10, 10);
                g2d.setColor(Color.BLACK);
                g2d.drawRoundRect(tx, ty, tooltipWidth + 10, tooltipHeight + 10, 10, 10);

                for (int i = 0; i < lines.length; i++) {
                    g2d.drawString(lines[i], tx + 5, ty + 15 + i * fm.getHeight());
                }
            }
        }

    }
}
