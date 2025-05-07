#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdint.h>
#include <time.h>
#include "crypto/sha256.h"
#include "graph_partition.h"
#include "output_file.h"

static void write_uint16_le(FILE *f, uint16_t val) {
    uint8_t b[2] = { val & 0xFF, (val >> 8) & 0xFF };
    fwrite(b, 1, 2, f);
}

static void write_uint32_le(FILE *f, uint32_t val) {
    uint8_t b[4] = {
        val & 0xFF,
        (val >> 8) & 0xFF,
        (val >> 16) & 0xFF,
        (val >> 24) & 0xFF
    };
    fwrite(b, 1, 4, f);
}

static uint32_t calculate_sha256_checksum(const char *filename, long data_offset) {
    FILE *f = fopen(filename, "rb");
    if (!f) return 0;
    fseek(f, data_offset, SEEK_SET);

    SHA256_CTX sha256;
    sha256_init(&sha256);

    uint8_t buffer[4096];
    size_t read_bytes;
    while ((read_bytes = fread(buffer, 1, sizeof(buffer), f)) > 0) {
        sha256_update(&sha256, buffer, read_bytes);
    }
    fclose(f);

    uint8_t hash[32];
    sha256_final(&sha256, hash);
    return *(uint32_t *)hash;
}

static uint32_t generate_file_id_from_graph() {
    srand((unsigned)time(NULL));
    return (uint32_t)rand();
}

int validate_checksum(const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (!f) return 0;

    fseek(f, 1, SEEK_SET);
    uint32_t file_id;
    fread(&file_id, sizeof(uint32_t), 1, f);

    uint32_t stored_checksum;
    fread(&stored_checksum, sizeof(uint32_t), 1, f);

    long data_offset = ftell(f);
    fclose(f);

    uint32_t computed_checksum = calculate_sha256_checksum(filename, data_offset);

    return stored_checksum == computed_checksum;
}

void write_binary_output(const char *filename, int vertex_count) {
    uint32_t file_id = generate_file_id_from_graph();

    FILE *f = fopen(filename, "wb+");
    if (!f) {
        printf("Blad: bledne dane wejsciowe.");
        exit(14);
    }

    uint8_t endian_byte = 0x01;
    fwrite(&endian_byte, 1, 1, f);
    write_uint32_le(f, file_id);
    write_uint32_le(f, 0);

    long data_offset = ftell(f);

    for (int i = 0; i < vertex_count; i++) {
        write_uint16_le(f, (uint16_t)vertices[i].x);
        write_uint16_le(f, (uint16_t)vertices[i].y);
        write_uint16_le(f, (uint16_t)vertices[i].group);
        write_uint16_le(f, (uint16_t)vertices[i].edge_num);
        for (int j = 0; j < vertices[i].edge_num; j++) {
            write_uint16_le(f, (uint16_t)vertices[i].conn[j]);
        }
    }

    fflush(f);

    uint32_t checksum = calculate_sha256_checksum(filename, data_offset);
    fseek(f, 5, SEEK_SET);
    write_uint32_le(f, checksum);

    fclose(f);
}

void write_ascii_output(const char *filename, int vertex_count) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        printf("Blad: bledne dane wejsciowe");
        exit(14);
    }

    fprintf(f, "%d\n", vertex_count);

    int max_group = 0;
    for (int i = 0; i < vertex_count; i++) {
        if (vertices[i].group > max_group) max_group = vertices[i].group;
    }
    fprintf(f, "%d\n", max_group + 1);

    for (int i = 0; i < vertex_count; i++) {
        fprintf(f, "%d;%d;%d;%d;", vertices[i].x, vertices[i].y, vertices[i].group, vertices[i].edge_num);
        for (int j = 0; j < vertices[i].edge_num; j++) {
            fprintf(f, "%d", vertices[i].conn[j]);
            fprintf(f, ";");
        }
        fprintf(f, "\n");
    }

    fclose(f);
}