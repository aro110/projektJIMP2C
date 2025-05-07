#ifndef GRAPH_PARTITION_H
#define GRAPH_PARTITION_H

typedef struct vertex {
    int edge_num;
    int *conn;
    int group;
    int fixed;
    int processed;
    int D;
    int x;
    int y;
} Vertex;

extern int force_flag;
extern Vertex *vertices;

#endif //GRAPH_PARTITION_H
