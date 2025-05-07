#ifndef INPUT_FILE_H
#define INPUT_FILE_H
#include <stdio.h>

void read_file_error(FILE *file);
void validate_graph_data(int max_matrix, int *x_coords, int x_count, int *y_offsets, int y_offsets_count, int *connections, int count_conn, int *offsets, int count_offsets, int parts, int error_margin);
void read_num_dynamic(FILE *file, int **array, int *count, int file_size);
int count_lines(FILE *file);
void skip_lines(FILE *file, int n, int file_size);
void read_file(char **input_file, int *vertex_count, int choose_graph, int parts, double error_margin);


#endif //INPUT_FILE_H
