#ifndef GRAPH_UTILS_H
#define GRAPH_UTILS_H

void remove_cross_group_connections(int vertex_count, double error_margin);
int is_vertex_connected_to_own_group(int v);
int has_connection_in_group(int v, int group);
int find_swap_candidate(int from_group, int to_group, int vertex_count);
void fix_group_connectivity(int vertex_count, int parts, int min_size, int max_size);

#endif //GRAPH_UTILS_H
