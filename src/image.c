#include "image.h"

int create_copy_image_file(struct options_image *opts) {
    FILE *carrier_file = fopen(opts->carrier_name, "rb");
    if (carrier_file == NULL) {
        perror("Error opening source file");
        return 2;
    }

    FILE *result_file = fopen(opts->result_name, "wb");
    if (result_file == NULL) {
        perror("Error opening destination file");
        fclose(carrier_file);
        return 3;
    }

    fseek(carrier_file, 0, SEEK_END);
    long file_size = ftell(carrier_file);
    fseek(carrier_file, 0, SEEK_SET);

    unsigned char *buffer = (unsigned char *)malloc(file_size);
    if (buffer == NULL) {
        fprintf(stderr, "Error allocating memory\n");
        fclose(carrier_file);
        fclose(result_file);
        return 4;
    }

    size_t read_size = fread(buffer, 1, file_size, carrier_file);
    if (read_size != file_size) {
        fprintf(stderr, "Error reading source file\n");
        free(buffer);
        fclose(carrier_file);
        fclose(result_file);
        return 5;
    }

    size_t write_size = fwrite(buffer, 1, file_size, result_file);
    if (write_size != file_size) {
        fprintf(stderr, "Error writing to destination file\n");
        free(buffer);
        fclose(carrier_file);
        fclose(result_file);
        return 6;
    }

    free(buffer);
    fclose(carrier_file);
    fclose(result_file);
    printf("BMP file copied successfully.\n");

    return 0;
}