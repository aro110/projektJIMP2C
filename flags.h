#ifndef FLAGS_H
#define FLAGS_H

void flags_error(char **format, char *raw_parts, int *parts, char **method, char *raw_error_margin, double *error_margin, char *raw_choose_graph, int *choose_graph);
void flags(int argc, char *argv[], char **input_file, char **output_file, char **format, int *parts, char **method, double *error_margin, int *choose_graph);

#endif //FLAGS_H
