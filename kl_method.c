#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdint.h>
#include "kl_method.h"
#include "graph_partition.h"

void reset_fixed_flags(int vertex_count) {
    for (int i = 0; i < vertex_count; i++) {
        vertices[i].fixed = 0;
    }
}

void initial_bipartition(int vertex_count, int group1_size) {
    for (int i = 0; i < vertex_count; i++) {
        vertices[i].group = (i < group1_size) ? 0 : 1;
    }
}

int edge_cut_counter(int one_group_vertices_count) {
    int edge_cut_count = 0;
    for (int i = 0; i < one_group_vertices_count; i++) {
        if (vertices[i].conn == NULL) continue;
        for (int j = 0; j < vertices[i].edge_num; j++) {
            int neighbour = vertices[i].conn[j];
            if (vertices[i].group != vertices[neighbour].group) {
                edge_cut_count++;
            }
        }
    }
    return edge_cut_count;
}

int calc_G(int first_vertex, int second_vertex, int vertex_count) {
    if (second_vertex >= vertex_count || first_vertex >= vertex_count) return 0;
    if (vertices[first_vertex].fixed == 1 || vertices[second_vertex].fixed == 1) {
        return 0;
    }

    int connection = 0;
    if (vertices[first_vertex].conn != NULL) {
        for (int i = 0; i < vertices[first_vertex].edge_num; i++) {
            if (vertices[first_vertex].conn[i] == second_vertex) {
                connection = 1;
                break;
            }
        }
    }

    return vertices[first_vertex].D + vertices[second_vertex].D - (2 * connection);
}

void calc_D(int counter) {
    if (vertices[counter].fixed == 1 || vertices[counter].conn == NULL) {
        return;
    }

    int external_edges = 0;
    int internal_edges = 0;

    for (int i = 0; i < vertices[counter].edge_num; i++) {
        int neighbour = vertices[counter].conn[i];
        if (vertices[counter].group != vertices[neighbour].group) {
            external_edges++;
        } else {
            internal_edges++;
        }
    }

    vertices[counter].D = external_edges - internal_edges;
}

int kernighan_lin_algorithm(int one_group_vertices_count, int vertex_count) {
    int edge_cut = edge_cut_counter(one_group_vertices_count);
    int group2_size = vertex_count - one_group_vertices_count;
    int best_cut = edge_cut;

    int *initial_groups = malloc(vertex_count * sizeof(int));
    if (!initial_groups) {
        printf("Blad pamieci.");
        exit(15);
    }
    for (int i = 0; i < vertex_count; i++) {
        initial_groups[i] = vertices[i].group;
    }

    int *gain = malloc(one_group_vertices_count * group2_size * sizeof(int));
    Swap *swaps = malloc(one_group_vertices_count * sizeof(Swap));

    while (1) {
        for (int i = 0; i < vertex_count; i++) calc_D(i);
        reset_fixed_flags(vertex_count);

        int swap_count = 0;

        for (int s = 0; s < one_group_vertices_count; s++) {
            int max_gain = -999999;
            int best_i = -1, best_j = -1;

            for (int i = 0; i < one_group_vertices_count; i++) {
                if (vertices[i].fixed) continue;
                for (int j = 0; j < group2_size; j++) {
                    int idx_j = j + one_group_vertices_count;
                    if (vertices[idx_j].fixed) continue;
                    int g_val = calc_G(i, idx_j, vertex_count);
                    if (g_val > max_gain) {
                        max_gain = g_val;
                        best_i = i;
                        best_j = idx_j;
                    }
                }
            }

            if (max_gain < 0) break;

            vertices[best_i].fixed = 1;
            vertices[best_j].fixed = 1;
            swaps[swap_count++] = (Swap){best_i, best_j, max_gain};
        }

        int prefix_sum = 0, max_prefix_sum = -999999, k_max = -1;
        for (int i = 0; i < swap_count; i++) {
            prefix_sum += swaps[i].gain;
            if (prefix_sum > max_prefix_sum) {
                max_prefix_sum = prefix_sum;
                k_max = i;
            }
        }

        if (max_prefix_sum <= 0) break;

        for (int i = 0; i <= k_max; i++) {
            int a = swaps[i].a;
            int b = swaps[i].b;
            int tmp = vertices[a].group;
            vertices[a].group = vertices[b].group;
            vertices[b].group = tmp;
        }

        edge_cut = edge_cut_counter(one_group_vertices_count);
        if (edge_cut < best_cut) {
            best_cut = edge_cut;
            for (int i = 0; i < vertex_count; i++) initial_groups[i] = vertices[i].group;
        } else break;
    }

    for (int i = 0; i < vertex_count; i++) {
        vertices[i].group = initial_groups[i];
    }

    free(gain);
    free(swaps);
    free(initial_groups);
    return best_cut;
}