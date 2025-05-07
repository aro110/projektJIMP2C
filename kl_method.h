#ifndef KL_METHOD_H
#define KL_METHOD_H

typedef struct swap {
    int a;
    int b;
    int gain;
} Swap;

int kernighan_lin_algorithm(int one_group_vertices_count, int vertex_count);

void initial_bipartition(int vertex_count, int group1_size);
void calc_D(int counter);
int calc_G(int first_vertex, int second_vertex, int vertex_count);
void reset_fixed_flags(int vertex_count);
int edge_cut_counter(int one_group_vertices_count);

#endif // KL_METHOD_H
