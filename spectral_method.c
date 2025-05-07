#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <math.h>
#include <stdint.h>
#include <gsl/gsl_eigen.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_sort.h>
#include "graph_partition.h"
#include "spectral_method.h"
#include "graph_utils.h"

#define EPSILON 1e-6

int cmp_entry(const void *a, const void *b) {
    double diff = ((Entry *)a)->value - ((Entry *)b)->value;
    return (diff < 0) ? -1 : (diff > 0);
}

Matrix *alloc_matrix(int n) {
    Matrix *m = malloc(sizeof(Matrix));
    if (!m) {
        printf("Blad pamieci.");
        exit(15);
    }
    m->n = n;
    m->data = malloc(n * sizeof(double *));
    if (!m->data) {
        printf("Blad pamieci.");
        exit(15);
    }
    for (int i = 0; i < n; i++) {
        m->data[i] = calloc(n, sizeof(double));
        if (!m->data[i]) {
            printf("Blad pamieci.");
            exit(15);
        }
    }
    return m;
}

void free_matrix(Matrix *m) {
    for (int i = 0; i < m->n; i++) {
        free(m->data[i]);
    }
    free(m->data);
    free(m);
}

Matrix *build_laplacian_matrix(int n) {
    Matrix *L = alloc_matrix(n);
    for (int i = 0; i < n; i++) {
        int deg = vertices[i].edge_num;
        L->data[i][i] = deg;
        for (int j = 0; j < deg; j++) {
            int neighbor = vertices[i].conn[j];
            L->data[i][neighbor] = -1;
        }
    }
    return L;
}

void normalize_vector(double *v, int n) {
    double norm = 0;
    for (int i = 0; i < n; i++) norm += v[i] * v[i];
    norm = sqrt(norm);
    if (norm > EPSILON) {
        for (int i = 0; i < n; i++) v[i] /= norm;
    }
}

void matvec_mul(Matrix *m, double *v, double *result) {
    for (int i = 0; i < m->n; i++) {
        result[i] = 0;
        for (int j = 0; j < m->n; j++) {
            result[i] += m->data[i][j] * v[j];
        }
    }
}

double vector_dot(const double *a, const double *b, int n) {
    double sum = 0;
    for (int i = 0; i < n; i++) sum += a[i] * b[i];
    return sum;
}

void power_iteration(Matrix *L, double *eigenvector, const int max_iter) {
    const int n = L->n;

    // GSL Matrix i Vector
    gsl_matrix *gsl_L = gsl_matrix_alloc(n, n);
    gsl_vector *eigenvalues = gsl_vector_alloc(n);
    gsl_matrix *eigenvectors = gsl_matrix_alloc(n, n);

    // Zapisz dane z macierzy Laplacjana L do macierzy GSL
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            gsl_matrix_set(gsl_L, i, j, L->data[i][j]);
        }
    }

    // Obliczanie wartości i wektorów własnych
    gsl_eigen_symmv_workspace *workspace = gsl_eigen_symmv_alloc(n);
    int status = gsl_eigen_symmv(gsl_L, eigenvalues, eigenvectors, workspace);
    if (status != GSL_SUCCESS) {
        printf("Blad podczas obliczania wartości i wektorów własnych.\n");
        exit(19);
    }

    // Wybieramy drugi najmniejszy wektor własny (Fiedler vector)
    double min_value = gsl_vector_get(eigenvalues, 0);
    int min_index = 0;
    for (int i = 1; i < n; i++) {
        double val = gsl_vector_get(eigenvalues, i);
        if (val > min_value) {
            min_value = val;
            min_index = i;
        }
    }

    // Kopiowanie drugiego najmniejszego wektora własnego
    gsl_vector_view vec = gsl_matrix_column(eigenvectors, min_index);
    for (int i = 0; i < n; i++) {
        eigenvector[i] = gsl_vector_get(&vec.vector, i);
    }

    // Zwolnij pamięć
    gsl_eigen_symmv_free(workspace);
    gsl_matrix_free(gsl_L);
    gsl_vector_free(eigenvalues);
    gsl_matrix_free(eigenvectors);
}

int edge_cut_all(int vertex_count) {
    int cut = 0;
    for (int i = 0; i < vertex_count; i++) {
        for (int j = 0; j < vertices[i].edge_num; j++) {
            int neighbor = vertices[i].conn[j];
            if (vertices[i].group != vertices[neighbor].group) cut++;
        }
    }
    return cut / 2;
}

void spectral_partitioning(int parts, int vertex_count, double error_margin) {
    Matrix *L = build_laplacian_matrix(vertex_count);
    double *eigenvector = malloc(vertex_count * sizeof(double));
    Entry *entries = malloc(vertex_count * sizeof(Entry));
    int *best_groups = malloc(vertex_count * sizeof(int));

    if (eigenvector == NULL || entries == NULL || best_groups == NULL) {
        printf("Blad pamieci.");
        exit(15);
    }

    int max_iter = vertex_count * 10;
    power_iteration(L, eigenvector, max_iter);

    for (int i = 0; i < vertex_count; i++) {
        entries[i].index = i;
        entries[i].value = eigenvector[i];
    }
    qsort(entries, vertex_count, sizeof(Entry), cmp_entry);

    int best_edge_cut = 999999;

    for (int start = 0; start < parts; start++) {
        int *group_counts = calloc(parts, sizeof(int));
        if(group_counts == NULL) {
            printf("Blad pamieci");
            exit(15);
        }
        for (int i = 0; i < vertex_count; i++) {
            int g = i % parts;
            vertices[entries[i].index].group = g;
            group_counts[g]++;
        }

        int target = vertex_count / parts;
        int margin = (int)(target * error_margin / 100.0);
        int min_size = target - margin;
        if (min_size < 0) min_size = 0;
        int max_size = target + margin;

        fix_group_connectivity(vertex_count, parts, min_size, max_size);
        int edge_cut = edge_cut_all(vertex_count);
        if (edge_cut < best_edge_cut) {
            best_edge_cut = edge_cut;
            for (int i = 0; i < vertex_count; i++) best_groups[i] = vertices[i].group;
        }
        free(group_counts);
    }

    for (int i = 0; i < vertex_count; i++) vertices[i].group = best_groups[i];

    free(best_groups);
    free_matrix(L);
    free(eigenvector);
    free(entries);
}
