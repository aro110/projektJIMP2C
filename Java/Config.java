public class Config {
    private boolean forcePartition;
    private String inputFilePath;
    private String outputFileType;
    private int partsCount;
    private int errorMargin;
    private int graphIndex;
    private String outputFilePath;

    public Config() {
        this.forcePartition = false;
        this.inputFilePath = "";
        this.outputFileType = "ASCII";
        this.partsCount = 2;
        this.errorMargin = 10;
        this.graphIndex = 0;
        this.outputFilePath = "";
    }

    public boolean isForcePartition() {
        return forcePartition;
    }

    public void setForcePartition(boolean forcePartition) {
        this.forcePartition = forcePartition;
    }

    public String getInputFilePath() {
        return inputFilePath;
    }

    public void setInputFilePath(String inputFilePath) {
        this.inputFilePath = inputFilePath;
    }

    public String getOutputFileType() {
        return outputFileType;
    }

    public void setOutputFileType(String outputFileType) {
        this.outputFileType = outputFileType;
    }

    public int getPartsCount() {
        return partsCount;
    }

    public void setPartsCount(int partsCount) {
        this.partsCount = partsCount;
    }

    public int getErrorMargin() {
        return errorMargin;
    }

    public void setErrorMargin(int errorMargin) {
        this.errorMargin = errorMargin;
    }

    public int getGraphIndex() {
        return graphIndex;
    }

    public void setGraphIndex(int graphIndex) {
        this.graphIndex = graphIndex;
    }

    public String getOutputFilePath() {
        return outputFilePath;
    }

    public void setOutputFilePath(String outputFilePath) {
        this.outputFilePath = outputFilePath;
    }
}
