#ifndef SPECTRAL_METHOD_H
#define SPECTRAL_METHOD_H

typedef struct matrix {
    int n;
    double **data;
} Matrix;

typedef struct entry {
    int index;
    double value;
} Entry;

int cmp_entry(const void *a, const void *b);
Matrix* alloc_matrix(int n);
void free_matrix(Matrix *m);
Matrix* build_laplacian_matrix(int n);
void normalize_vector(double *v, int n);
void matvec_mul(Matrix *m, double *v, double *result);
double vector_dot(const double *a, const double *b, int n);
void power_iteration(Matrix *L, double *eigenvector, const int max_iter);
int edge_cut_all(int vertex_count);
void spectral_partitioning(int parts, int vertex_count, double error_margin);


#endif //SPECTRAL_METHOD_H
