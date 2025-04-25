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
} Vertex;

#define MAX_VERTICES 1024
Vertex vertices[MAX_VERTICES];

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

    // printf("Do gr 1 naleza: ");
    // for(int i=0; i<vertex_count; i++) {
    //     if (vertices[i].group == 1) {
    //         printf("%d, ", i);
    //     }
    // }
    // printf("\nDo gr 2 naleza: ");
    // for(int i=0; i<vertex_count; i++) {
    //     if (vertices[i].group == 2) {
    //         printf("%d, ", i);
    //     }
    // }

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
    if(file == NULL) {
        printf("Blad: bledne dane wejsciowe.");
        exit(14);
    }
}

void read_num(FILE *file, int *array, int *count) {
    *count = 0;

    size_t size = 128;
    char *line = malloc(size);
    if (!line) {
        printf("Blad pamieci.\n");
        exit(15);
    }

    size_t len = 0;
    int ch2;

    while ((ch2 = fgetc(file)) != EOF) {
        if (len + 1 >= size) {
            size *= 2;
            line = realloc(line, size);
            if (!line) {
                printf("Blad pamieci.\n");
                exit(15);
            }
        }
        if (ch2 == '\n' || ch2 == EOF) {
            break;
        }
        line[len++] = ch2;
    }
    line[len] = '\0';


    line[len] = '\0';

    char *token = strtok(line, ";");
    while (token != NULL) {
        while (*token == ' ') token++;  // usuń spacje na początku tokena
        if (*token != '\0') {
            array[*count] = atoi(token);
            (*count)++;
        }
        token = strtok(NULL, ";");
    }

    free(line);
}


void read_file(char **input_file, int *vertex_count) {
    FILE *file = fopen(*input_file, "r");
    read_file_error(file);

    // 1. Wczytaj liczbę wierzchołków
    char buffer[32];
    if (fgets(buffer, sizeof(buffer), file) != NULL) {
        *vertex_count = atoi(buffer);
        if (*vertex_count <= 0 || *vertex_count > MAX_VERTICES) {
            printf("Blad: Niepoprawna liczba wierzcholkow (1 linia).\n");
            exit(18);
        }
    }

    // 2. Pomiń 2 kolejne linie (nagłówki)
    int ch, line_count = 0;
    while (line_count < 2 && (ch = fgetc(file)) != EOF) {
        if (ch == '\n') line_count++;
    }

    // 3. Wczytaj linie 4 i 5 (połączenia i offsety)
    int connections[10000];  // może być zwiększone
    int offsets[1024];
    int count_conn = 0, count_offsets = 0;

    read_num(file, connections, &count_conn);
    read_num(file, offsets, &count_offsets);

    // WALIDACJA: ostatni offset nie może być > liczby połączeń
    if (offsets[count_offsets - 1] > count_conn) {
        printf("Blad: Niepoprawna konfiguracja offsetow (linia 4 i 5).\n");
        exit(18);
    }

    // Alokacja macierzy sąsiedztwa (symetrycznej)
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

    // Inicjalizacja liczby krawędzi
    for (int i = 0; i < *vertex_count; i++) {
        vertices[i].edge_num = 0;
    }

    // Parsowanie połączeń na podstawie offsetów
    for (int i = 0; i < count_offsets - 1; i++) {
        int start = offsets[i];
        int end = offsets[i + 1];
        if (start >= end) continue; // brak połączeń

        int from = connections[start];  // pierwszy element = wierzchołek źródłowy
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

    // Alokacja tablic połączeń
    for (int i = 0; i < *vertex_count; i++) {
        vertices[i].conn = malloc(vertices[i].edge_num * sizeof(int));
        if (!vertices[i].conn) {
            printf("Blad pamieci (alokacja conn[%d]).\n", i);
            exit(15);
        }
        vertices[i].edge_num = 0;  // zresetuj, będziemy zapełniać ponownie
    }

    // Wypełnienie tablicy połączeń
    for (int i = 0; i < *vertex_count; i++) {
        for (int j = 0; j < *vertex_count; j++) {
            if (adjacency[i][j]) {
                vertices[i].conn[vertices[i].edge_num++] = j;
            }
        }
    }

    // printf("Lista polaczen wierzcholkow:\n");
    // for (int i = 0; i < *vertex_count; i++) {
    //     printf("Wierzcholek %d (krawedzie: %d): ", i, vertices[i].edge_num);
    //     for (int j = 0; j < vertices[i].edge_num; j++) {
    //         printf("%d ", vertices[i].conn[j]);
    //     }
    //     printf("\n");
    // }

    // Zwolnienie pamięci pomocniczej
    for (int i = 0; i < *vertex_count; i++) {
        free(adjacency[i]);
    }
    free(adjacency);

    fclose(file);
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
                    *input_file = "C:/Users/Arkadiusz/CLionProjects/projektJIMP2C/dane1.txt";
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

    return 0;
}
