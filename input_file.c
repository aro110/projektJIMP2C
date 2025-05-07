#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include "graph_partition.h"
#include "input_file.h"

void read_file_error(FILE *file) {
    if (file == NULL) {
        printf("Blad: bledne dane wejsciowe.\n");
        exit(14);
    }
}

void validate_graph_data(int max_matrix, int *x_coords, int x_count, int *y_offsets, int y_offsets_count, int *connections, int count_conn, int *offsets, int count_offsets, int parts, int error_margin) {
    // sprawdzzenie zgdnosci 1 linii
    if (max_matrix > 1024 || max_matrix < 0) {
        printf("Blad: Niepoprawny format pliku wejsciowego. Pierwsza linia pliku musi byc w przedziale 0-1024.");
        exit(13);
    }

    // liczba wierzcholkow musi byc wieksza niz 0
    if (x_count <= 0) {
        printf("Blad: Niepoprawny format pliku wejsciowego. Liczba wierzcholkow z 2 linii musi byc wiesza niz 0.");
        exit(13);
    }

    // sprawdzenie czy  wspolrzedna x miesci sie od 0 do 1 linii
    for (int i = 0; i < x_count; i++) {
        if (x_coords[i] < 0) {
            printf("Blad: Niepoprawny format pliku wejsciowego. Pozycja wierzcholka w 2 linii nie moze byc wartoscia mniejsza niz 0.");
            exit(13);
        }
    }

    for (int i = 0; i < y_offsets_count; i++) {
        if (y_offsets[i] < 0) {
            printf("Blad: Niepoprawny format pliku wejsciowego. Indeks pozycji wierzcholka w 3 linii nie moze byc wartoscia mniejsza niz 0.");
            exit(13);
        }
    }

    // czy ilosc wierzcholkow z Y zgadza sie z iloscia z X
    if (y_offsets[y_offsets_count - 1] != x_count) {
        printf("Blad: Niepoprawny format pliku wejsciowego. Obliczona ilosc wierzcholkow z 2 linii nie zgadza sie z iloscia z 3 linii.");
        exit(13);
    }

    // sprawdzenie czy liczby sa rosnaco w 3 linii
    for (int i = 0; i < y_offsets_count - 1; i++) {
        if (y_offsets[i] > y_offsets[i + 1]) {
            printf("Blad: Niepoprawny format pliku wejsciowego. Indeksy pozycji w 3 linii musza byc uporzadkowane rosnaco");
            exit(13);
        }
    }

    // sprawdzenie polaczen z liczba wierzcholkow
    for (int i = 0; i < count_conn; i++) {
        if (connections[i] < 0 || connections[i] >= x_count) {
            printf("Blad: Niepoprawny format pliku wejsciowego. Numer wierzcholka w linii 4 nie moze byc wiekszy niz ogolna liczba wierzcholkow.");
            exit(13);
        }
    }

    // sprawdzenie czy ostatni offset nie przekracza liczby krawedzi
    if (offsets[count_offsets - 1] > count_conn) {
        printf("Blad: Niepoprawny format pliku wejsciowego. Ostatni indeks polaczenia wierzcholka w 5 linii nie moze byc wiekszy niz liczba polaczen.");
        exit(13);
    }

    // sprawdzenie czy offesty sa rosnace i poprawne
    for (int i = 0; i < count_offsets - 1; i++) {
        if (offsets[i] > offsets[i + 1] || offsets[i] > count_conn) {
            printf("Blad: Niepoprawny format pliku wejsciowego. Indeksy polaczen w 5 linii musza byc uporzadkowane rosnaco, oraz nie moga przekraczac liczby polaczen.");
            exit(13);
        }
    }

    int vertex_count = x_count;
    if(parts > (floor(vertex_count/2))) {
        printf("Blad: Zbyt duza liczba podgrafow.");
        exit(20);
    }

    if (vertex_count < 4) {
        printf("Blad: graf jest zbyt maly, aby mozna bylo go podzielic.");
        exit(18);
    }

    // Obliczamy idealną liczbę wierzchołków na grupę
    int ideal_group_size = vertex_count / parts;
    int remainder = vertex_count % parts;

    // Sprawdzenie flagi 'force'
    if (force_flag) {
        // Jeśli flaga 'force' jest ustawiona, zaokrąglamy liczby wierzchołków na grupy
        if (remainder != 0) {
            // printf("Podzial z flagą force, zaokrąglamy wierzchołki: %d wierzchołków na pierwszą grupę, %d na drugą grupę.\n", ideal_group_size + remainder, ideal_group_size);
            remainder = 0;
        }
    }

    // Walidacja marginesu błędu
    if (error_margin == 0) {
        // Jeśli margines błędu wynosi 0, sprawdzamy, czy podział jest możliwy
        if (remainder != 0) {
            printf("Blad: Podzial nie moze byc wykonany bez marginesu bledu. Zbyt mala liczba wierzcholkow na podgraf.\n");
            exit(21);
        }
    } else {
        // Obliczamy minimalny i maksymalny rozmiar grupy na podstawie marginesu błędu
        int min_group_size = ideal_group_size - (int)((ideal_group_size * (error_margin / 100.0)) / 2);
        int max_group_size = ideal_group_size + (int)((ideal_group_size * (error_margin / 100.0)) / 2);

        // Jeśli margines błędu jest zbyt mały i reszta wierzchołków nie wynosi 0, podział jest niemożliwy
        if (parts > 2 && (max_group_size - min_group_size) < 1 && remainder > 0) {
            printf("Blad: Za maly margines bledu, by dokonac prawidlowego podzialu.\n");
            exit(22);
        }
    }
}


void read_num_dynamic(FILE *file, int **array, int *count, int file_size) {
    *count = 0;
    size_t size = 128;
    *array = malloc(size * sizeof(int));
    if (!*array) {
        printf("Blad pamieci.\n");
        exit(15);
    }

    char *line = malloc(file_size);
    if (!line) {
        printf("Blad pamieci.\n");
        exit(15);
    }

    if (fgets(line, file_size, file) == NULL) {
        printf("Blad: Niepoprawny format pliku. Nie wczytano linii (sprawdz czy nie jest pusta).\n");
        free(line);
        exit(13);
    }

    char *token = strtok(line, ";");
    while (token != NULL) {
        while (*token == ' ') token++;

        char *endptr;
        long val = strtol(token, &endptr, 10);

        if (*endptr != '\0' && *endptr != '\n') {
            printf("Blad: Niepoprawny format pliku. Niedozwolony znak: '%s'. Znaki dozwolone to liczby i ';'.\n", token);
            free(line);
            exit(13);
        }

        if (*count >= size) {
            size *= 2;
            *array = realloc(*array, size * sizeof(int));
            if (!*array) {
                printf("Blad pamieci.");
                free(line);
                exit(15);
            }
        }

        (*array)[(*count)++] = (int)val;
        token = strtok(NULL, ";");
    }
    free(line);
}

int count_lines(FILE *file) {
    int lines = 1;
    int ch;
    long pos = ftell(file);
    rewind(file);
    while ((ch = fgetc(file)) != EOF) {
        if (ch == '\n') lines++;
    }
    fseek(file, pos, SEEK_SET);
    return lines;
}

void skip_lines(FILE *file, int n, int file_size) {
    char *line = malloc(file_size);
    if (!line) {
        printf("Blad pamieci.\n");
        exit(15);
    }

    for (int i=0; i < n; i++) {
        if (fgets(line, file_size, file) == NULL) {
            printf("Blad: Nie udalo sie wybrac danego grafu.");
            exit(23);
        }
    }
    free(line);
}

void read_file(char **input_file, int *vertex_count, int choose_graph, int parts, double error_margin) {
    FILE *file = fopen(*input_file, "r");
    read_file_error(file);

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);


    int line_count = count_lines(file);
    if (line_count < 5) {
        printf("Blad: Niepoprawny format pliku. Plik musi zawierac przynajmniej 5 linii danych.\n");
        exit(13);
    }
    if (line_count > 5 && choose_graph == 0) {
        printf("Blad: Wykryto wieksza ilosc grafow w pliku. Zdefiniuj z ktorego korzystasz");
        exit(24);
    }
    if (line_count == 5 && choose_graph != 0) {
        printf("Blad: Nie mozna wybrac grafu.\n");
        exit(25);
    }
    if (choose_graph > (line_count - 4)) {
        printf("Blad: Program nie wykryl takiego grafu.\n");
        exit(26);
    }

    int max_matrix = 0;
    int ch;
    int digit_found = 0, non_digit_found = 0;
    long cursor_pos = ftell(file);

    while ((ch = fgetc(file)) != '\n' && ch != EOF) {
        if (isdigit(ch)) {
            digit_found = 1;
        } else if (!isspace(ch)) {
            non_digit_found = 1;
        }
    }
    if (!digit_found || non_digit_found) {
        printf("Blad: Niepoprawny format pliku wejsciowego. W 1 linii wykryto niedozwolony znak, program przyjmuje tylko liczbe calkowita.");
        exit(13);
    }

    fseek(file, cursor_pos, SEEK_SET);
    fscanf(file, "%d", &max_matrix);

    while ((ch = fgetc(file)) != '\n' && ch != EOF);

    int *x_coords = NULL;
    int x_count = 0;
    read_num_dynamic(file, &x_coords, &x_count, file_size);

    int *y_offsets = NULL;
    int y_offsets_count = 0;
    read_num_dynamic(file, &y_offsets, &y_offsets_count, file_size);

    int *connections = NULL;
    int count_conn = 0;
    read_num_dynamic(file, &connections, &count_conn, file_size);

    int *offsets = NULL;
    int count_offsets = 0;

    if (choose_graph > 0) {
        skip_lines(file, choose_graph - 1, file_size);
    }
    read_num_dynamic(file, &offsets, &count_offsets, file_size);

    validate_graph_data(max_matrix, x_coords, x_count, y_offsets, y_offsets_count, connections, count_conn, offsets, count_offsets, parts, error_margin);

    *vertex_count = x_count;
    vertices = malloc(*vertex_count * sizeof(Vertex));
    if (!vertices) {
        printf("Blad pamieci.\n");
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
        printf("Blad pamieci.\n");
        exit(15);
    }

    for (int i = 0; i < *vertex_count; i++) {
        adjacency[i] = calloc(*vertex_count, sizeof(int));
        if (!adjacency[i]) {
            printf("Blad pamieci.\n");
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
                printf("Blad pamieci.");
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
}