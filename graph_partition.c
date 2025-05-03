#include <ctype.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

typedef struct vertex {
    int edge_num;
    int *conn;
    int group;
    int fixed;
    int D;
    int x;
    int y;
} Vertex;

Vertex *vertices = NULL;

void reset_fixed_flags(Vertex *vertices, int vertex_count) {
    for (int i = 0; i < vertex_count; i++) {
        vertices[i].fixed = 0;
    }
}

void initial_bipartition(Vertex *vertices, int vertex_count, int group1_size) {
    for (int i = 0; i < vertex_count; i++) {
        vertices[i].group = (i < group1_size) ? 0 : 1;
    }
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

typedef struct swap {
    int a;
    int b;
    int gain;
} Swap;

int kernighan_lin_algorithm(int one_group_vertices_count, int vertex_count) {
    int edge_cut = edge_cut_counter(one_group_vertices_count);
    int group2_size = vertex_count - one_group_vertices_count;
    int best_cut = edge_cut;

    int *initial_groups = malloc(vertex_count * sizeof(int));
    if (!initial_groups) exit(15);
    for (int i = 0; i < vertex_count; i++) {
        initial_groups[i] = vertices[i].group;
    }

    int *gain = malloc(one_group_vertices_count * group2_size * sizeof(int));
    Swap *swaps = malloc(one_group_vertices_count * sizeof(Swap));

    while (1) {
        for (int i = 0; i < vertex_count; i++) calc_D(i);
        reset_fixed_flags(vertices, vertex_count);

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

void graph_partioning(char *method, int parts, double error_margin, int vertex_count) {
    if (strcmp(method, "kl") == 0) {
        if (parts != 2) {
            printf("Blad: Metoda KL wspiera tylko podzial na 2 grupy.\n");
            exit(14);
        }

        int ideal_half = vertex_count / 2;
        int best_edge_cut = 999999;
        int *best_groups = malloc(vertex_count * sizeof(int));
        if (!best_groups) {
            printf("Blad pamieci (best_groups).\n");
            exit(15);
        }

        double max_allowed_diff = (error_margin == -1) ? 0 : vertex_count * (error_margin / 100.0);
        int min_group = ideal_half - (int)(max_allowed_diff / 2);
        int max_group = ideal_half + (int)(max_allowed_diff / 2);

        for (int size = min_group; size <= max_group; size++) {
            reset_fixed_flags(vertices, vertex_count);
            initial_bipartition(vertices, vertex_count, size);

            int edge_cut = kernighan_lin_algorithm(size, vertex_count);
            if (edge_cut < best_edge_cut) {
                best_edge_cut = edge_cut;
                for (int i = 0; i < vertex_count; i++) {
                    best_groups[i] = vertices[i].group;
                }
            }
        }

        printf("\nNajlepszy podzial (edge cut = %d):\n", best_edge_cut);
        printf("Grupa 0: ");
        for (int i = 0; i < vertex_count; i++) {
            if (best_groups[i] == 0) printf("%d ", i);
        }
        printf("\nGrupa 1: ");
        for (int i = 0; i < vertex_count; i++) {
            if (best_groups[i] == 1) printf("%d ", i);
        }
        printf("\n");

        free(best_groups);
    } else {
        printf("Blad: Wybrana metoda '%s' nie jest jeszcze wspierana.\n", method);
        exit(14);
    }
}


void read_file_error(FILE *file) {
    if (file == NULL) {
        printf("Blad: bledne dane wejsciowe.\n");
        exit(14);
    }
}

void validate_graph_data(int max_matrix, int *x_coords, int x_count, int *y_offsets, int y_offsets_count, int *connections, int count_conn, int *offsets, int count_offsets) {
    if (x_count <= 0) {
        printf("Blad: Niepoprawna liczba wierzcholkow.\n");
        exit(18);
    }

    for (int i = 0; i < x_count; i++) {
        if (x_coords[i] < 0 || x_coords[i] > max_matrix) {
            printf("Blad: Wspolrzedna x poza zakresem (x[%d]=%d, max_matrix=%d).\n", i, x_coords[i], max_matrix);
            exit(18);
        }
    }

    if (y_offsets[y_offsets_count - 1] != x_count) {
        printf("Blad: Liczba wierzcholkow nie zgadza sie z offsetami Y.\n");
        exit(18);
    }

    for (int i = 0; i < y_offsets_count - 1; i++) {
        if (y_offsets[i] > y_offsets[i + 1]) {
            printf("Blad: Offsety Y musza byc rosnace (y_offsets[%d]=%d, y_offsets[%d]=%d).\n", i, y_offsets[i], i + 1, y_offsets[i + 1]);
            exit(18);
        }
    }

    for (int i = 0; i < count_conn; i++) {
        if (connections[i] < 0 || connections[i] >= x_count) {
            printf("Blad: Niepoprawny indeks w connections[%d]=%d.\n", i, connections[i]);
            exit(18);
        }
    }

    if (offsets[count_offsets - 1] > count_conn) {
        printf("Blad: Niepoprawna konfiguracja offsetow (edges).\n");
        exit(18);
    }

    for (int i = 0; i < count_offsets - 1; i++) {
        if (offsets[i] > offsets[i + 1] || offsets[i] > count_conn) {
            printf("Blad: Offsety edges musza byc rosnace i poprawne (offsets[%d]=%d, offsets[%d]=%d).\n", i, offsets[i], i + 1, offsets[i + 1]);
            exit(18);
        }
    }
}

void read_num_dynamic(FILE *file, int **array, int *count) {
    *count = 0;
    size_t size = 128;
    *array = malloc(size * sizeof(int));
    if (!*array) {
        printf("Blad pamieci.\n");
        exit(15);
    }

    int buffer_size = 8192;
    char *line = malloc(buffer_size);
    if (!line) {
        printf("Blad pamieci.\n");
        exit(15);
    }

    if (fgets(line, buffer_size, file) == NULL) {
        printf("Blad: Brak danych.\n");
        free(line);
        exit(18);
    }

    char *token = strtok(line, ";");
    while (token != NULL) {
        while (*token == ' ') token++;
        if (*count >= size) {
            size *= 2;
            *array = realloc(*array, size * sizeof(int));
            if (!*array) {
                printf("Blad pamieci realloc.\n");
                free(line);
                exit(15);
            }
        }
        (*array)[(*count)++] = atoi(token);
        token = strtok(NULL, ";");
    }

    free(line);
}

void read_file(char **input_file, int *vertex_count) {
    FILE *file = fopen(*input_file, "r");
    read_file_error(file);

    int max_matrix = 0;
    fscanf(file, "%d", &max_matrix);
    int ch;
    while ((ch = fgetc(file)) != '\n' && ch != EOF);

    int *x_coords = NULL;
    int x_count = 0;
    read_num_dynamic(file, &x_coords, &x_count);

    int *y_offsets = NULL;
    int y_offsets_count = 0;
    read_num_dynamic(file, &y_offsets, &y_offsets_count);

    *vertex_count = x_count;

    int *connections = NULL;
    int *offsets = NULL;
    int count_conn = 0, count_offsets = 0;

    read_num_dynamic(file, &connections, &count_conn);
    read_num_dynamic(file, &offsets, &count_offsets);

    validate_graph_data(max_matrix, x_coords, x_count, y_offsets, y_offsets_count, connections, count_conn, offsets, count_offsets);

    vertices = malloc(*vertex_count * sizeof(Vertex));
    if (!vertices) {
        printf("Blad pamieci (alokacja vertices).\n");
        exit(15);
    }

    for (int i = 0; i < *vertex_count; i++) {
        vertices[i].x = x_coords[i];
        vertices[i].fixed = 0;
        vertices[i].group = 0;
        vertices[i].D = 0;
        vertices[i].edge_num = 0;
        vertices[i].conn = NULL;

        for (int y = 0; y < y_offsets_count - 1; y++) {
            if (i >= y_offsets[y] && i < y_offsets[y + 1]) {
                vertices[i].y = y;
                break;
            }
        }
    }

    int **adjacency = malloc(*vertex_count * sizeof(int *));
    if (!adjacency) {
        printf("Blad pamieci (alokacja adjacency).\n");
        exit(15);
    }

    for (int i = 0; i < *vertex_count; i++) {
        adjacency[i] = calloc(*vertex_count, sizeof(int));
        if (!adjacency[i]) {
            printf("Blad pamieci (alokacja adjacency[%d]).\n", i);
            exit(15);
        }
    }

    for (int i = 0; i < count_offsets - 1; i++) {
        int start = offsets[i];
        int end = offsets[i + 1];
        if (start >= end) continue;

        int from = connections[start];
        for (int j = start + 1; j < end; j++) {
            int to = connections[j];
            if (!adjacency[from][to]) {
                adjacency[from][to] = 1;
                adjacency[to][from] = 1;
                vertices[from].edge_num++;
                vertices[to].edge_num++;
            }
        }
    }

    for (int i = 0; i < *vertex_count; i++) {
        if (vertices[i].edge_num > 0) {
            vertices[i].conn = malloc(vertices[i].edge_num * sizeof(int));
            if (!vertices[i].conn) {
                printf("Blad pamieci (alokacja conn[%d]).\n", i);
                exit(15);
            }
        } else {
            vertices[i].conn = NULL;
        }
        vertices[i].edge_num = 0;
    }

    for (int i = 0; i < *vertex_count; i++) {
        for (int j = 0; j < *vertex_count; j++) {
            if (adjacency[i][j]) {
                vertices[i].conn[vertices[i].edge_num++] = j;
            }
        }
    }

    for (int i = 0; i < *vertex_count; i++) {
        free(adjacency[i]);
    }
    free(adjacency);

    free(x_coords);
    free(y_offsets);
    free(connections);
    free(offsets);

    fclose(file);

    printf("Wspolrzedne wierzcholkow:\n");
    for (int i = 0; i < *vertex_count; i++) {
        printf("Wierzcholek %d: x = %d, y = %d\n", i, vertices[i].x, vertices[i].y);
    }
}



void flags_error(char **format, char *raw_parts, int *parts, char **method, char *raw_error_margin, double *error_margin) {
    if (*format == NULL || *method == NULL) {
        printf("Blad: brak wymaganych parametrow --format lub --method.\n");
        exit(14);
    }

    // format
    if (strcmp(*format, "ascii") != 0 && strcmp(*format, "binary") != 0) {
        printf("Blad: bledne dane wejsciowe.\n");
        exit(14);
    }

    // method
    if (strcmp(*method, "kl") != 0 && strcmp(*method, "w") != 0 && strcmp(*method, "m") != 0) {
        printf("Blad: bledne dane wejsciowe.\n");
        exit(14);
    }

    // parts
    if (raw_parts != NULL) {
        char *endptr;
        *parts = strtol(raw_parts, &endptr, 10);
        if (*endptr != '\0') {
            printf("Blad: bledne dane wejsciowe.\n");
            exit(14);
        }
    }

    // error_margin
    if (raw_error_margin != NULL) {
        double val = atof(raw_error_margin);
        if (val < -1 || val > 100) {
            printf("Blad: bledne dane wejsciowe.\n");
            exit(14);
        }
        if (val != -1) {
            *error_margin = val;
        }
    }
}

void flags(int argc, char *argv[], char **input_file, char **output_file, char **format, int *parts, char **method, double *error_margin) {
    int opt;
    char *raw_parts = NULL;
    char *raw_error_margin = NULL;


    // przypisanie dlugich flag
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"force", no_argument, 0, 'f'},
        {"input-file", required_argument, 0, 'i'},
        {"output-file", required_argument, 0, 'o'},
        {"format", required_argument, 0, 'r'},
        {"parts", required_argument, 0, 'p'},
        {"method", required_argument, 0, 'm'},
        {0, 0, 0, 0}
    };

    // obsluga flag
    while ((opt = getopt_long(argc, argv, "fhm:i:o:r:b:p:", long_options, NULL)) != -1) {
        switch (opt) {
            case 'f': // force
                *error_margin = -1;
                break;
            case 'h': // help
                printf("POMOC\n");
                exit(0);
            case 'm': // method
                *method = optarg;
                break;
            case 'i': // input_file
                // *input_file = optarg;
                    *input_file = "C:/Users/Arkadiusz/CLionProjects/nastiaprojekt/dane2.txt";
                break;
            case 'o': // output_file
                *output_file = optarg;
                break;
            case 'r': // format
                *format = optarg;
                break;
            case 'p': // number of parts
                raw_parts = optarg;
                break;
            case 'b': // error_margin
                raw_error_margin = optarg;
                break;
            default:
                printf("Blad: nieznany parametr.\n");
                exit(12);
        }
    }

    flags_error(format, raw_parts, parts, method, raw_error_margin, error_margin);
}

int main(int argc, char *argv[]) {
    char *input_file = NULL;
    char *output_file = NULL;
    char *format = NULL;
    int parts = 0;
    char *method = NULL;
    double error_margin = 0.0;
    int vertex_count = 0;

    // Wywołanie funkcji obsługującej flagi
    flags(argc, argv, &input_file, &output_file, &format, &parts, &method, &error_margin);

    read_file(&input_file, &vertex_count);

    graph_partioning(method, parts, error_margin, vertex_count);

    // Wypisanie wartości parametrow
    // printf("wartosc -m --method: %s\n", method);
    // printf("wartosc -i: %s\n", input_file);
    // printf("wartosc -o: %s\n", output_file);
    // printf("wartosc -r: %s\n", format);
    // printf("wartosc -p: %i\n", parts);
    // printf("wartosc -b: %f\n", error_margin);
    // printf("%d", vertex_count);

    free(vertices);
    return 0;
}
