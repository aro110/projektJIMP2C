#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <stdint.h>
#include "flags.h"
#include "graph_partition.h"


void flags_error(char **format, char *raw_parts, int *parts, char **method, char *raw_error_margin, double *error_margin, char *raw_choose_graph, int *choose_graph) {
    if (*format == NULL || *method == NULL) {
        printf("Blad: parametry wywolania sa niewystarczajace, aby uruchomic program.\n");
        exit(11);
    }

    if (strcmp(*format, "ascii") != 0 && strcmp(*format, "binary") != 0) {
        printf("Blad: Bledne dane wejsciowe. Niepoprawna wartosc flagi --format.\n");
        exit(14);
    }

    if (strcmp(*method, "kl") != 0 && strcmp(*method, "m") != 0) {
        printf("Blad: Bledne dane wejsciowe. Niepoprawna wartosc flagi --method.\n");
        exit(14);
    }

    if (raw_parts != NULL) {
        char *endptr;
        *parts = strtol(raw_parts, &endptr, 10);
        if (*endptr != '\0' || *parts < 2) {
            printf("Blad: Bledne dane wejsciowe. Liczba podgrafow musi wynosic co najmniej 2.\n");
            exit(14);
        }
    }

    if (raw_error_margin != NULL) {
        double val = atof(raw_error_margin);
        if (val < 0 || val > 100) {
            printf("Blad: Bledne dane wejsciowe. Margines bledu musi byc z zakresu [0, 100].\n");
            exit(14);
        }
        *error_margin = val;
    }

    if (raw_choose_graph != NULL) {
        char *endptr;
        *choose_graph = strtol(raw_choose_graph, &endptr, 10);
        if (*endptr != '\0' || *choose_graph < 0) {
            printf("Blad: Bledne dane wejsciowe. Niepoprawna wartosc flagi --graph_index.\n");
            exit(14);
        }
    }
}

void flags(int argc, char *argv[], char **input_file, char **output_file, char **format, int *parts, char **method, double *error_margin, int *choose_graph) {
    int opt;
    char *raw_parts = NULL;
    char *raw_error_margin = NULL;
    char *raw_choose_graph = NULL;

    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"force", no_argument, 0, 'f'},
        {"input-file", required_argument, 0, 'i'},
        {"output-file", required_argument, 0, 'o'},
        {"format", required_argument, 0, 'r'},
        {"parts", required_argument, 0, 'p'},
        {"method", required_argument, 0, 'm'},
        {"error_margin", required_argument, 0, 'b'},
        {"graph_index", required_argument, 0, 'g'},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "fhm:i:o:r:b:p:g:", long_options, NULL)) != -1) {
        switch (opt) {
            case 'f': force_flag = 1; break;
            case 'h':
                const char *help_text =
"Program do podzialu grafu na grupy przy uzyciu metod Kernighan-Lin lub spektralnej.\n\n"
"==============================  Uzycie  ==============================\n"
"Uzycie:\n"
"  graph_partition [parametry]\n\n"
"==============================  Parametry wymagane  ==================\n"
"  -i, --input-file <plik>    Okresla plik wejsciowy zawierajacy dane grafu.\n"
"  -o, --output-file <plik>   Okresla plik wyjsciowy do zapisu wynikow.\n"
"  -r, --format <format>      Okresla format wyjsciowy (\"ascii\" lub \"binary\").\n"
"  -m, --method <metoda>      Okresla metode podzialu (\"kl\" dla 2 grup lub \"m\" dla wiekszej liczby grup).\n\n"
"==============================  Parametry opcjonalne  =================\n"
"  -h, --help                 Wyswietla ta pomoc.\n"
"  -f, --force                Wymusza podzial niezaleznie od marginesu bledu.\n"
"  -p, --parts <liczba>       Liczba czesci (grup) do podzialu grafu (domyslnie 2).\n"
"  -b, --error_margin <wartosc>   Margines bledu w procentach (domyslnie 10, 0 dla dokladnego podzialu).\n"
"  -g, --graph_index <indeks>    Indeks grafu w pliku wejsciowym (jesli plik zawiera wiecej niz jeden graf).\n\n"
"==============================  Przyklady  ===========================\n"
"  graph_partition --input-file graf.txt --output-file wynik.txt --format ascii --parts 2 --method kl --error_margin 10\n"
"    Podzieli graf z pliku \"graf.txt\" na 2 grupy, uzywajac metody Kernighan-Lin, zapisujac wynik w formacie ASCII.\n\n"
"  graph_partition --input-file graf.bin --output-file wynik.bin --format binary --parts 3 --method m --error_margin 5\n"
"    Podzieli graf z pliku \"graf.bin\" na 3 grupy przy uzyciu metody spektralnej, zapisujac wynik w formacie binarnym.\n\n"
"==============================  Opis metod  ===========================\n"
"  kl       - Metoda Kernighan-Lin, stosowana do podzialu na 2 grupy. Optymalizuje ciecie krawedzi przez iteracyjne zamienianie wierzcholkow miedzy grupami.\n"
"  m        - Metoda spektralna, wykorzystuje wektor wlasny macierzy Laplacjana grafu do przypisania wierzcholkow do grup.\n\n"
"==============================  Uwagi  ===============================\n"
"  - Metoda KL wspiera jedynie podzial na 2 grupy. Jesli chcesz podzielic graf na wiecej niz 2 grupy, musisz wybrac metode m (spektralna).\n"
"  - Flaga --force pozwala na wymuszenie podzialu grafu, nawet jesli margines bledu jest zbyt maly do dokladnego podzialu.\n"
"  - Flaga --error_margin okresla dozwolona roznice w liczbie wierzcholkow w grupach, aby podzial byl uznany za poprawny.\n"
"  - W przypadku pliku z wieloma grafami, nalezy podac odpowiedni indeks grafu za pomoca flagi --graph_index.\n"
"  - Program wspiera tylko podzial na 2 grupy dla metody Kernighan-Lin, wiec przy innych liczbach czesci uzywana jest metoda spektralna.\n"
"  - Domyslne wartosci:\n"
"    - `-p` (liczba czesci) to 2.\n"
"    - `-b` (margines bledu) to 10.\n\n"
"==============================  Bledy  ===============================\n"
"  Jesli plik wejsciowy jest uszkodzony, ma niepoprawny format lub nie spelnia wymaganych warunkow (np. za mala liczba wierzcholkow), program zakonczy dzialanie i wypisze stosowny komunikat o bledzie.\n\n"
"==============================  Biblioteki zewnetrzne  =================\n"
"  - GSL (GNU Scientific Library):\n"
"    - Biblioteka uzywana do operacji zwiazanych z naukowymi obliczeniami, w tym obliczaniem wektora wlasnego macierzy Laplacjana.\n"
"    - Wiecej informacji: https://www.gnu.org/software/gsl/\n\n"
"  - SHA256 (Brad Conte):\n"
"    - Implementacja algorytmu SHA256 do haszowania, pobrana z repozytorium autora na GitHubie.\n"
"    - Repozytorium: https://github.com/B-Con/crypto-algorithms/tree/master\n"
"    - Biblioteka jest dostepna do uzytku publicznego na licencji open source, co pozwala na swobodne wykorzystanie w innych projektach.\n";
                printf("%s", help_text);
                exit(0);
            case 'm': *method = optarg; break;
            case 'i': *input_file = optarg; break;
            case 'o': *output_file = optarg; break;
            case 'r': *format = optarg; break;
            case 'p': raw_parts = optarg; break;
            case 'b': raw_error_margin = optarg; break;
            case 'g': raw_choose_graph = optarg; break;
            default: printf("Blad: Nieznany parametr.\n"); exit(12);
        }
    }

    flags_error(format, raw_parts, parts, method, raw_error_margin, error_margin, raw_choose_graph, choose_graph);
}
