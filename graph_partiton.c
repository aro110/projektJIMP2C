//
// Created by Arkadiusz on 07.03.2025.
//

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

void flags(int argc, char *argv[], char **input_file, char **output_file, char **format, int *parts, char **method, double *error_margin) {
    int opt;
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
                *input_file = optarg;
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

    // Wypisanie wartości parametrow
    // printf("wartosc -m --method: %s\n", method);
    // printf("wartosc -i: %s\n", input_file);
    // printf("wartosc -o: %s\n", output_file);
    // printf("wartosc -r: %s\n", format);
    // printf("wartosc -p: %i\n", parts);
    // printf("wartosc -b: %f\n", error_margin);

    return 0;
}
