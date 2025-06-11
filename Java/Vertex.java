public class Vertex {
    public int id;
    public int edgeNum;
    public int[] conn;
    public int group;
    public int fixed;
    public int processed;
    public int x;
    public int y;

    public Vertex(int id, int edgeNum, int group, int fixed, int processed, int x, int y) {
        this.id = id;
        this.edgeNum = edgeNum;
        this.group = group;
        this.fixed = fixed;
        this.processed = processed;
        this.x = x;
        this.y = y;
    }

    public Vertex(int edgeNum, int group, int fixed, int processed, int x, int y) {
        this.edgeNum = edgeNum;
        this.group = group;
        this.fixed = fixed;
        this.processed = processed;
        this.x = x;
        this.y = y;
    }
}
