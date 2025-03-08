//
// Created by Arkadiusz on 07.03.2025.
//

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

typedef struct vertex {
    int edge_num;
    int *conn;
} Vertex;

Vertex vertices[100];

void read_file_error(FILE *file) {
    if(file == NULL) {
        printf("Blad: bledne dane wejsciowe.");
        exit(14);
    }
}

void read_num(FILE *file, int *array, int *count) {
    *count = 0;  // RESET LICZNIKA

    size_t size = 128;
    char *line = malloc(size);
    if (!line) {
        printf("Blad pamieci3.\n");
        exit(15);
    }

    size_t len = 0;
    int ch2;
    int last_char_was_eof = 0;

    while ((ch2 = fgetc(file)) != EOF) {
        if (len + 1 >= size) {
            size *= 2;
            line = realloc(line, size);
            if (!line) {
                printf("Blad pamieci2.\n");
                exit(15);
            }
        }
        line[len++] = ch2;

        // Sprawdzenie, czy doszliśmy do końca linii
        if (ch2 == '\n') {
            line[len] = '\0';
            break;
        }

        // Jeśli następne wywołanie fgetc() zwróci EOF, oznacza to koniec pliku bez \n
        if (feof(file)) {
            last_char_was_eof = 1;
            break;
        }
    }

    // Jeśli plik kończy się bez '\n', ręcznie zakończ linię
    if (last_char_was_eof) {
        line[len] = '\0';
    }

    // printf("DEBUG: Wczytana linia: \"%s\"\n", line);

    char *token = strtok(line, ";");
    while (token != NULL) {
        array[(*count)++] = atoi(token);
        // printf("DEBUG: Wczytano %d, count = %d\n", array[*count - 1], *count);
        token = strtok(NULL, ";");
    }

    // printf("DEBUG: Liczba elementow PO pętli = %d\n", *count);
    free(line);
}


void read_file(char **input_file, char **format) {
    // sprawdzenie jaki jest format pliku
    if (stricmp(*format, "ascii") == 0) {

        FILE *file = fopen(*input_file, "r");
        read_file_error(file);

        // do dodania sprawdzenie 1 linii (tej opcjonalnej)

        // pomija 3 pierwsze linie pliku
        int ch, line_count = 0;
        while (line_count < 3 && (ch = fgetc(file)) != EOF) {
            if(ch == '\n') {
                line_count++;
            }
        }

        int connections[100];
        int offsets[100];
        int count_conn = 0, count_offsets = 0;

        // printf("DEBUG: Pozycja pliku przed pierwszym read_num(): %ld\n", ftell(file));
        read_num(file, connections, &count_conn);
        // printf("DEBUG: Pozycja pliku po wymuszeniu nowej linii: %ld\n", ftell(file));
        read_num(file, offsets, &count_offsets);

        fclose(file);

        // printf("DEBUG: count_conn = %d, count_offsets = %d\n", count_conn, count_offsets);

        // Tworzenie tablicy wierzcholdkow Vertex vertices
        for (int i=0; i < count_offsets - 1; i++) {
            // if (i + 1 >= count_offsets) {  // ZABEZPIECZENIE
            //     printf("Blad: Przekroczono zakres tablicy offsets!\n");
            //     exit(17);
            // }

            int num_of_edges = offsets[i+1] - offsets[i] - 1;
            // printf("DEBUG: Wierzchołek %d, num_of_edges = %d\n", i, num_of_edges);

            // if (num_of_edges < 0) {  // ZABEZPIECZENIE
            //     printf("Blad: num_of_edges jest ujemne!\n");
            //     exit(18);
            // }
            // alokacja pamieci
            vertices[i].conn = malloc(num_of_edges * sizeof(int));
            if(!vertices[i].conn) {
                printf("Blad pamieci1.");
                exit(15);
            }

            // kopiowanie polaczen
            for (int j=0; j<num_of_edges; j++) {
                vertices[i].conn[j] = connections[offsets[i] + j];
            }
            vertices[i].edge_num = num_of_edges;
        }

        // Wyświetlenie listy połączeń wierzchołków
        printf("Lista polaczen wierzcholkow:\n");
        // printf("%d", count_offsets);
        for (int i = 0; i < count_offsets - 1; i++) {
            printf("Wierzcholek %d (krawedzie: %d): ", i, vertices[i].edge_num);
            for (int j = 0; j < vertices[i].edge_num; j++) {
                printf("%d ", vertices[i].conn[j]);
            }
            printf("\n");
        }
    }
}

void flags(int argc, char *argv[], char **input_file, char **output_file, char **format, int *parts, char **method, double *error_margin) {
    int opt;

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
                if (strcmp(optarg, "kl") != 0 && strcmp(optarg, "w") != 0 && strcmp(optarg, "m") != 0) {
                    printf("Blad: bledne dane wejsciowe.\n");
                    exit(14);
                }
                *method = optarg;
                break;
            case 'i': // input_file
                // *input_file = optarg;
                    *input_file = "C:/Users/Arkadiusz/CLionProjects/projektJIMP2C/dane.txt";
                break;
            case 'o': // output_file
                *output_file = optarg;
                break;
            case 'r': // format
                if (strcmp(optarg, "ascii") != 0 && strcmp(optarg, "binary") != 0) {
                    printf("Blad: bledne dane wejsciowe.\n");
                    exit(14);
                }
                *format = optarg;
                break;
            case 'p': // number of parts
                char *endptr;
                *parts = strtol(optarg, &endptr, 10);
                if (*endptr != '\0') {
                    printf("Blad: bledne dane wejsciowe.\n");
                    exit(14);
                }
                break;
            case 'b': // error_margin
                if (atof(optarg) < -1 || atof(optarg) > 100) {
                    printf("Blad: bledne dane wejsciowe.\n");
                    exit(14);
                }
                if (atof(optarg) != -1) {
                    *error_margin = atof(optarg);
                }
                break;
            default:
                printf("Blad: nieznany parametr.\n");
                exit(12);
        }
    }
}

int main(int argc, char *argv[]) {
    char *input_file = NULL;
    char *output_file = NULL;
    char *format = NULL;
    int parts = 0;
    char *method = NULL;
    double error_margin = 0.0;

    // Wywołanie funkcji obsługującej flagi
    flags(argc, argv, &input_file, &output_file, &format, &parts, &method, &error_margin);

    read_file(&input_file, &format);
    // Wypisanie wartości parametrow
    // printf("wartosc -m --method: %s\n", method);
    // printf("wartosc -i: %s\n", input_file);
    // printf("wartosc -o: %s\n", output_file);
    printf("wartosc -r: %s\n", format);
    // printf("wartosc -p: %i\n", parts);
    // printf("wartosc -b: %f\n", error_margin);

    return 0;
}
