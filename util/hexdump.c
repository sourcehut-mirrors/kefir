/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2023  Jevgenijs Protopopovs

    This file is part of Kefir project.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define BYTES_PER_LINE 12

static void print_byte(int value, FILE *output) {
    static size_t byte_counter = 0;
    if (byte_counter > 0) {
        fprintf(output, ",");
        if (byte_counter % BYTES_PER_LINE == 0) {
            fprintf(output, "\n");
        } else {
            fprintf(output, " ");
        }
    }
    fprintf(output, "0x%.2xu", value);
    byte_counter++;
}

static int hexdump(FILE *input, FILE *output) {

    int value;
    while ((value = fgetc(input)) != EOF) {
        print_byte(value, output);
    }

    if (ferror(input)) {
        perror("Failed to read from file");
        return -1;
    }
    return 0;
}

int main(int argc, const char **argv) {
    if (argc < 2) {
        hexdump(stdin, stdout);
        return EXIT_SUCCESS;
    }

    for (int i = 1; i < argc; i++) {
        const char *filepath = argv[i];
        if (strcmp(filepath, "--zero") == 0) {
            print_byte(0, stdout);
            continue;
        }

        FILE *file = fopen(filepath, "r");
        if (file == NULL) {
            perror("Failed to open file");
            return EXIT_FAILURE;
        }

        int rc = hexdump(file, stdout);
        if (rc != 0) {
            fclose(file);
            return EXIT_FAILURE;
        }

        if (fclose(file) != 0) {
            perror("Failed to close file");
            return EXIT_FAILURE;
        }
    }
    printf("\n");
    return EXIT_SUCCESS;
}
