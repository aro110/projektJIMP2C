#include <stdio.h>
#include <stdlib.h>
#include "graph_partition.h"
#include "graph_utils.h"

void remove_cross_group_connections(int vertex_count, double error_margin) {
    for (int i = 0; i < vertex_count; i++) {
        int new_edge_num = 0;
        int *new_conn = malloc(vertices[i].edge_num * sizeof(int));
        if (!new_conn) {
            printf("Blad pamieci.\n");
            exit(15);
        }

        for (int j = 0; j < vertices[i].edge_num; j++) {
            int neighbor = vertices[i].conn[j];
            if (vertices[neighbor].group == vertices[i].group) {
                new_conn[new_edge_num++] = neighbor;
            }
        }

        if (new_edge_num == 0) {
            printf("Blad: Wierzcholek %d zostal bez polaczen (nie mozna usunac wszystkich polaczen).\n", i);
            exit(16);
        }

        free(vertices[i].conn);
        vertices[i].conn = new_conn;
        vertices[i].edge_num = new_edge_num;
    }
}

int is_vertex_connected_to_own_group(int v) {
    for (int i = 0; i < vertices[v].edge_num; i++) {
        int neighbor = vertices[v].conn[i];
        if (vertices[neighbor].group == vertices[v].group) return 1;
    }
    return 0;
}

int has_connection_in_group(int v, int group) {
    for (int i = 0; i < vertices[v].edge_num; i++) {
        int neighbor = vertices[v].conn[i];
        if (vertices[neighbor].group == group) return 1;
    }
    return 0;
}
int find_swap_candidate(int from_group, int to_group, int vertex_count) {
    for (int i = 0; i < vertex_count; i++) {
        if (vertices[i].group == to_group && !vertices[i].processed && is_vertex_connected_to_own_group(i)) {
            return i;
        }
    }
    return -1;
}

void fix_group_connectivity(int vertex_count, int parts, int min_size, int max_size) {
    for (int i = 0; i < vertex_count; i++) {
        vertices[i].processed = 0;
    }

    int *group_sizes = calloc(parts, sizeof(int));
    if (!group_sizes) {
        printf("Blad pamieci.\n");
        exit(15);
    }

    for (int i = 0; i < vertex_count; i++) group_sizes[vertices[i].group]++;

    for (int i = 0; i < vertex_count; i++) {
        if (!is_vertex_connected_to_own_group(i)) {
            int current = vertices[i].group;
            int best_target = -1;

            for (int g = 0; g < parts; g++) {
                if (g != current && has_connection_in_group(i, g)) {
                    best_target = g;
                    break;
                }
            }

            if (best_target != -1) {
                // Jeśli miejsce w grupie jest, przenosimy wierzchołek
                if (group_sizes[best_target] < max_size || force_flag) {
                    vertices[i].group = best_target;
                    vertices[i].processed = 1;
                    group_sizes[current]--;
                    group_sizes[best_target]++;
                }
            } else if (!force_flag) {
                // Jeżeli brak miejsca, zamieniamy wierzchołki
                for (int g = 0; g < parts; g++) {
                    if (g != current && has_connection_in_group(i, g)) {
                        int swap = find_swap_candidate(current, g, vertex_count);
                        if (swap != -1) {
                            // Wykonujemy zamianę
                            int temp_group = vertices[swap].group;
                            vertices[swap].group = current;
                            vertices[i].group = g;
                            vertices[swap].processed = 1;
                            vertices[i].processed = 1;
                            group_sizes[current]--;
                            group_sizes[temp_group]++;
                            group_sizes[g]++;
                            break;
                        }
                    }
                }
            }
        }
    }
    free(group_sizes);
}
