#ifndef OUTPUT_FILE_H
#define OUTPUT_FILE_H
#include <stdint.h>
#include <stdio.h>

static void write_uint16_le(FILE *f, uint16_t val);
static void write_uint32_le(FILE *f, uint32_t val);
static uint32_t calculate_sha256_checksum(const char *filename, long data_offset);
static uint32_t generate_file_id_from_graph();
int validate_checksum(const char *filename);
void write_binary_output(const char *filename, int vertex_count);
void write_ascii_output(const char *filename, int vertex_count);


#endif //OUTPUT_FILE_H