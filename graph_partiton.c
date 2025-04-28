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
    short x;
    short y;
} Vertex;

Vertex *vertices = NULL;

int calc_G(int first_vertex, int second_vertex1, int second_vertex2) {
    int g = 0;
    int connection = 0;
    int second_vertex = second_vertex1 + second_vertex2;

    if (vertices[first_vertex].fixed == 1 || vertices[second_vertex].fixed == 1) {
        return 0;
    }

    for (int i = 0; i<vertices[first_vertex].edge_num; i++) {
        int c = 0;
        if (vertices[first_vertex].conn[i] == second_vertex) {
            c = 1;
            if (connection < c) {
                connection = c;
            }
        }
    }

    g = vertices[first_vertex].D + vertices[second_vertex].D - (2*connection);
    // printf("D wierzcholka %d: %d\n", first_vertex, vertices[first_vertex].D);
    // printf("D wierzcholka %d: %d\n", second_vertex, vertices[second_vertex].D);
    // printf("G wierzcholkow %d %d: %d\n", first_vertex, second_vertex, g);
    return g;
}

void calc_D (int counter) {
    if (vertices[counter].fixed == 1) {
        return;
    }

    int external_edges = 0;
    int internal_edges = 0;

    for (int i=0; i<vertices[counter].edge_num; i++) {
        int neighbour = vertices[counter].conn[i];
        if (vertices[counter].group != vertices[neighbour].group) {
            external_edges++;
        } else {
            internal_edges++;
        }
    }

    int D = external_edges - internal_edges;
    // printf("wartosc D wierzholka %d: %d\n", counter, D);
    vertices[counter].D = D;
}

int edge_cut_counter(int one_group_vertices_count) {
    int edge_cut_count = 0;
    for (int i = 0; i < one_group_vertices_count; i++) {
        for (int j = 0; j < vertices[i].edge_num; j++) {
            int neighbour = vertices[i].conn[j];
            if (vertices[i].group != vertices[neighbour].group) {
                edge_cut_count++;
            }
        }
    }
    return edge_cut_count;
}

int kernighan_lin_algorithm (int one_group_vertices_count, int vertex_count) {

    //liczba przecietych krawedzi
    int edge_cut = edge_cut_counter(one_group_vertices_count);
    int new_edge_cut = edge_cut - 1;

    while (new_edge_cut < edge_cut) {
        // printf("liczba przecietych krawedzi: %d", edge_cut);

        // liczenie zysku D kazdego wierzcholka
        for (int i = 0; i < vertex_count; i++) calc_D(i);

        // liczenie wskaznika wydajnosci G kazdej pary
        int second_group_vertices_count = vertex_count - one_group_vertices_count;
        // int size = one_group_vertices_count * second_group_vertices_count;
        int gain[one_group_vertices_count][second_group_vertices_count];
        for (int i=0; i<one_group_vertices_count; i++) {
            for (int j=0; j<second_group_vertices_count; j++) {
                gain[i][j] = calc_G(i, j, one_group_vertices_count);
            }
        }
        int max_gain = gain[0][0];
        int best_first_vertex_to_swap = 0, best_second_vertex_to_swap = 0;

        for (int i = 0; i < one_group_vertices_count; i++) {
            for (int j = 0; j < second_group_vertices_count; j++) {
                if (gain[i][j] > max_gain) {
                    max_gain = gain[i][j];
                    best_first_vertex_to_swap = i;
                    best_second_vertex_to_swap = j + one_group_vertices_count;
                }
            }
        }

        if (max_gain<0) break;

        if (vertices[best_first_vertex_to_swap].group == 1) {
            vertices[best_first_vertex_to_swap].group = 2;
            vertices[best_first_vertex_to_swap].fixed = 1;
        }

        if (vertices[best_second_vertex_to_swap].group == 2) {
            vertices[best_second_vertex_to_swap].group = 1;
            vertices[best_second_vertex_to_swap].fixed = 1;
        }
        // printf("Zamieniono: %d i %d", best_first_vertex_to_swap, best_second_vertex_to_swap);
        new_edge_cut = edge_cut - max_gain;
        // printf("Nowy edge cut: %d", max_gain);
        edge_cut = edge_cut_counter(one_group_vertices_count);
    }

    return new_edge_cut;
}

void graph_partioning(char *method, int parts, double error_margin, int vertex_count) {
    if (strcmp(method, "kl") == 0) {
        if (parts != 2) {
            printf("Blad: bledne dane wejsciowe.");
            exit(14);
        }

        // podzial na 2 grupy
        int ideal_half = vertex_count / 2;
        int other_half = vertex_count - ideal_half;
        int actual_diff = abs(ideal_half - other_half);
        int best_edge_cut = 999999; // duża liczba na start
        int best_groups[vertex_count];

        if (error_margin != -1) {
            double max_allowed_diff = vertex_count * (error_margin / 100.0);
            if (actual_diff > max_allowed_diff) {
                printf("Blad: Podzial jest niemozliwy.\n");
                exit(19);
            }

            int min_vertices_count_in_group = ideal_half - (int)max_allowed_diff/2;
            int diff_vertices_count_in_group1 = ideal_half - min_vertices_count_in_group;

            printf("Minimalna ilosc w grupie %d\n", min_vertices_count_in_group);
            printf("roznica ilosc w grupie %d\n", diff_vertices_count_in_group1);

            for(int i = 0; i <= diff_vertices_count_in_group1; i++) {
                // printf("Iteracja %d\n", i);
                int actual_vertices_count_in_group = min_vertices_count_in_group + i;
                for (int j = 0; j < vertex_count; j++) {
                    vertices[j].fixed = 0;
                    if (j < actual_vertices_count_in_group) {
                        vertices[j].group = 1;
                    } else {
                        vertices[j].group = 2;
                    }
                }

                int edge_cut = kernighan_lin_algorithm(actual_vertices_count_in_group, vertex_count);
                if (edge_cut < best_edge_cut) {
                    best_edge_cut = edge_cut;
                    for (int j = 0; j < vertex_count; j++) {
                        best_groups[j] = vertices[j].group;
                    }
                }
            }

            printf("\nNajlepszy podzial (edge cut = %d):\n", best_edge_cut);
            printf("Grupa 1: ");
            for (int i = 0; i < vertex_count; i++) {
                if (best_groups[i] == 1) printf("%d ", i);
            }
            printf("\nGrupa 2: ");
            for (int i = 0; i < vertex_count; i++) {
                if (best_groups[i] == 2) printf("%d ", i);
            }
            printf("\n");
        }
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

    size_t buffer_size = 8192;
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
    max_matrix--;
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
                vertices[i].y = y - 1;
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
                    *input_file = "C:/Users/Arkadiusz/CLionProjects/projektJIMP2C/dane2.txt";
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
