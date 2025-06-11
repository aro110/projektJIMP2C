import java.io.*;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.*;

public class FileOutput {

    // ASCII zapis
    public static void writeAsciiOutput(String filename, Vertex[] vertices) throws IOException {
        try (BufferedWriter writer = new BufferedWriter(new FileWriter(filename))) {
            writer.write(vertices.length + "\n");

            int maxGroup = 0;
            for (Vertex v : vertices) {
                if (v.group > maxGroup) maxGroup = v.group;
            }
            writer.write((maxGroup + 1) + "\n");

            for (Vertex v : vertices) {
                writer.write(v.x + ";" + v.y + ";" + v.group + ";" + v.edgeNum + ";");
                for (int i = 0; i < v.edgeNum; i++) {
                    writer.write(v.conn[i] + ";");
                }
                writer.write("\n");
            }
        }
    }

    // BIN zapis
    public static void writeBinaryOutput(String filename, Vertex[] vertices) throws IOException, NoSuchAlgorithmException {
        Random random = new Random();
        int fileId = random.nextInt();

        try (RandomAccessFile raf = new RandomAccessFile(filename, "rw")) {
            raf.setLength(0);

            // Write header: endian byte
            raf.write(0x01);

            // File ID
            writeUint32LE(raf, fileId);

            // Placeholder for checksum
            writeUint32LE(raf, 0);

            long dataOffset = raf.getFilePointer();

            for (Vertex v : vertices) {
                writeUint16LE(raf, v.x);
                writeUint16LE(raf, v.y);
                writeUint16LE(raf, v.group);
                writeUint16LE(raf, v.edgeNum);
                for (int i = 0; i < v.edgeNum; i++) {
                    writeUint16LE(raf, v.conn[i]);
                }
            }

            raf.getChannel().force(true);

            int checksum = calculateChecksum(filename, dataOffset);
            raf.seek(5);
            writeUint32LE(raf, checksum);
        }
    }

    // Pomocnicze funkcje:
    private static void writeUint16LE(RandomAccessFile raf, int val) throws IOException {
        raf.write(val & 0xFF);
        raf.write((val >> 8) & 0xFF);
    }

    private static void writeUint32LE(RandomAccessFile raf, int val) throws IOException {
        raf.write(val & 0xFF);
        raf.write((val >> 8) & 0xFF);
        raf.write((val >> 16) & 0xFF);
        raf.write((val >> 24) & 0xFF);
    }

    private static int calculateChecksum(String filename, long dataOffset) throws IOException, NoSuchAlgorithmException {
        MessageDigest sha256 = MessageDigest.getInstance("SHA-256");

        try (RandomAccessFile raf = new RandomAccessFile(filename, "r")) {
            raf.seek(dataOffset);
            byte[] buffer = new byte[4096];
            int read;
            while ((read = raf.read(buffer)) > 0) {
                sha256.update(buffer, 0, read);
            }
        }
        byte[] hash = sha256.digest();
        ByteBuffer bb = ByteBuffer.wrap(hash);
        bb.order(ByteOrder.LITTLE_ENDIAN);
        return bb.getInt();
    }

    // ASCII odczyt
    public static Vertex[] readAsciiOutput(String filename) throws IOException {
        try (BufferedReader reader = new BufferedReader(new FileReader(filename))) {
            int vertexCount = Integer.parseInt(reader.readLine().trim());
            reader.readLine(); // skip number of groups (unused)
            Vertex[] vertices = new Vertex[vertexCount];

            for (int i = 0; i < vertexCount; i++) {
                String line = reader.readLine().trim();
                String[] tokens = line.split(";");
                int x = Integer.parseInt(tokens[0]);
                int y = Integer.parseInt(tokens[1]);
                int group = Integer.parseInt(tokens[2]);
                int edgeNum = Integer.parseInt(tokens[3]);
                int[] conn = new int[edgeNum];
                for (int j = 0; j < edgeNum; j++) {
                    conn[j] = Integer.parseInt(tokens[4 + j]);
                }
                vertices[i] = new Vertex(edgeNum, group, 0, 0, x, y);
                vertices[i].conn = conn;
            }

            buildAdjacency(vertices); // budujemy symetrię po odczycie
            return vertices;
        }
    }

    // BIN odczyt
    public static Vertex[] readBinaryOutput(String filename) throws IOException {
        try (RandomAccessFile raf = new RandomAccessFile(filename, "r")) {
            int endian = raf.read();
            if (endian != 1) throw new IOException("Nieobsługiwane endianness!");

            raf.skipBytes(4); // fileID
            raf.skipBytes(4); // checksum

            long remainingBytes = raf.length() - raf.getFilePointer();
            List<Vertex> vertices = new ArrayList<>();
            while (remainingBytes > 0) {
                int x = readUint16LE(raf);
                int y = readUint16LE(raf);
                int group = readUint16LE(raf);
                int edgeNum = readUint16LE(raf);
                int[] conn = new int[edgeNum];
                for (int i = 0; i < edgeNum; i++) {
                    conn[i] = readUint16LE(raf);
                }
                Vertex v = new Vertex(edgeNum, group, 0, 0, x, y);
                v.conn = conn;
                vertices.add(v);
                remainingBytes = raf.length() - raf.getFilePointer();
            }

            Vertex[] vertexArray = vertices.toArray(new Vertex[0]);
            buildAdjacency(vertexArray); // budujemy symetrię po odczycie
            return vertexArray;
        }
    }

    private static void buildAdjacency(Vertex[] vertices) {
        Map<Integer, Set<Integer>> adj = new HashMap<>();
        for (int i = 0; i < vertices.length; i++) {
            adj.putIfAbsent(i, new HashSet<>());
            for (int neighbor : vertices[i].conn) {
                adj.get(i).add(neighbor);
                adj.putIfAbsent(neighbor, new HashSet<>());
                adj.get(neighbor).add(i);
            }
        }
        for (int i = 0; i < vertices.length; i++) {
            Set<Integer> neighbors = adj.get(i);
            vertices[i].conn = neighbors.stream().mapToInt(Integer::intValue).toArray();
            vertices[i].edgeNum = vertices[i].conn.length;
        }
    }


    private static int readUint16LE(RandomAccessFile raf) throws IOException {
        int b1 = raf.readUnsignedByte();
        int b2 = raf.readUnsignedByte();
        return (b2 << 8) | b1;
    }

}
