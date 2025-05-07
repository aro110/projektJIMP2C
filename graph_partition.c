#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <stdint.h>
#include "kl_method.h"
#include "graph_partition.h"
#include "spectral_method.h"
#include "input_file.h"
#include "flags.h"
#include "output_file.h"
#include "graph_utils.h"

Vertex *vertices = NULL;
int force_flag = 0;

void graph_partioning(char *method, int parts, double error_margin, int vertex_count) {
    if (strcmp(method, "kl") == 0) {
        if (parts != 2) {
            printf("Blad: Metoda KL wspiera tylko podzial na 2 grupy.");
            exit(17);
        }

        int ideal_half = vertex_count / 2;
        int best_edge_cut = 999999;
        int *best_groups = malloc(vertex_count * sizeof(int));
        if (!best_groups) {
            printf("Blad pamieci.");
            exit(15);
        }

        double max_allowed_diff = (error_margin == -1) ? 0 : vertex_count * (error_margin / 100.0);
        int min_group = ideal_half - (int)(max_allowed_diff / 2);
        int max_group = ideal_half + (int)(max_allowed_diff / 2);

        for (int size = min_group; size <= max_group; size++) {
            reset_fixed_flags(vertex_count);
            initial_bipartition(vertex_count, size);

            int edge_cut = kernighan_lin_algorithm(size, vertex_count);
            if (edge_cut < best_edge_cut) {
                best_edge_cut = edge_cut;
                for (int i = 0; i < vertex_count; i++) {
                    best_groups[i] = vertices[i].group;
                }
            }
        }

        fix_group_connectivity(vertex_count, parts, min_group, max_group);

        free(best_groups);
    } else if (strcmp(method, "m") == 0) {
        spectral_partitioning(parts, vertex_count, error_margin);
    }
}

int main(int argc, char *argv[]) {
    char *input_file = NULL;
    char *output_file = NULL;
    char *format = NULL;
    int parts = 2;
    char *method = NULL;
    double error_margin = 10.0;
    int vertex_count = 0;
    int choose_graph = 0;

    flags(argc, argv, &input_file, &output_file, &format, &parts, &method, &error_margin, &choose_graph);
    read_file(&input_file, &vertex_count, choose_graph, parts, error_margin);
    graph_partioning(method, parts, error_margin, vertex_count);

    remove_cross_group_connections(vertex_count, error_margin);

    if(strcmp(format, "binary") == 0) {
        write_binary_output(output_file, vertex_count);
    } else if (strcmp(format, "ascii") == 0) {
        write_ascii_output(output_file, vertex_count);
    }

    printf("Podzial udany.");
    for(int i=0; i < vertex_count; i++) {
        if(vertices[i].conn != NULL) {
            free(vertices[i].conn);
        }
    }
    free(vertices);
    return 0;
}
