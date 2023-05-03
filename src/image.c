#include "image.h"


void create_result_image_file(struct options_image *opts) {
    uint8_t bit = 0;
    int encrypt_length = 0;
    int encrypt_file_name = 0;
    int encrypt_hiding = 0;
    uint32_t counter = 0;
    uint32_t hiding_size = 0;
    int carrier_byte = 0;
    int hiding_byte = 0;

    char *result_name = "result.bmp";
    uint8_t byte;


    strcpy(opts->result_name, result_name);

    FILE *carrier = fopen(opts->carrier_name, "rb");
    if (carrier == NULL) {
        fprintf(stderr, "[create_result_image_file] Could not open %s.\n", opts->carrier_name);
        exit(EXIT_FAILURE);
    }

    FILE *hiding = fopen(opts->hiding_name, "rb");
    if (hiding == NULL) {
        fclose(carrier);
        fprintf(stderr, "[create_result_image_file] Could not open %s.\n", opts->hiding_name);
        exit(EXIT_FAILURE);
    }

    FILE *result = fopen(opts->result_name, "wb");
    if (result == NULL) {
        fclose(carrier);
        fclose(hiding);
        fprintf(stderr, "[create_result_image_file] Could not create %s.\n", opts->result_name);
        exit(EXIT_FAILURE);
    }



    while ((carrier_byte = fgetc(carrier)) != EOF)
        fputc(carrier_byte, result);
    fseek(result, 0, SEEK_END);

    validate_file_size(opts);


    fseek(carrier, opts->carrier_offset, SEEK_SET);
    fseek(result, opts->carrier_offset, SEEK_SET);

    int hide;
    int encode_hide;
    int original_length;

    encrypt_length = encrypt_decrypt((int) strlen(opts->hiding_name));



    /* Hiding length of file name */
    for (int i = 0; i < 8; i++) {
        if (counter == opts->carrier_pixel_data_size) {
            fseek(carrier, opts->carrier_padding, SEEK_CUR);
            fseek(result, opts->carrier_padding, SEEK_CUR);
            counter = 0;
        }
        bit = bit_process((uint8_t) encrypt_length, i);
        printf("[carrier] Read : %ld\n", ftell(carrier));
        fread(&byte, sizeof(uint8_t), 1, carrier);
        if (bit == 1 && (get_LSB(byte) == 0)) byte |= 1;
        if (bit == 0 && (get_LSB(byte) == 1)) byte &= ~1;

        printf("[result] Write: %ld\n", ftell(result));
        fwrite(&byte, sizeof(uint8_t), 1, result);
        counter++;
    }

    /* Hiding file name */
    for (int i = 0; i < strlen(opts->hiding_name); i++) {
        encrypt_file_name = encrypt_decrypt((int) opts->hiding_name[i]);
        for (int j = 0; j < 8; j++) {
            if (counter == opts->carrier_pixel_data_size) {
                fseek(carrier, opts->carrier_padding, SEEK_CUR);
                fseek(result, opts->carrier_padding, SEEK_CUR);
                counter = 0;
            }

            bit = bit_process((uint8_t) encrypt_file_name, j);
            printf("[carrier] Read : %ld\n", ftell(carrier));
            fread(&byte, sizeof(uint8_t), 1, carrier);
            if (bit == 1 && (get_LSB(byte) == 0)) byte |= 1;
            if (bit == 0 && (get_LSB(byte) == 1)) byte &= ~1;
            printf("[result] Write: %ld\n", ftell(result));
            fwrite(&byte, sizeof(uint8_t), 1, result);
            counter++;
        }
    }

    while (hiding_size != opts->hiding_actual) {
        if (counter == opts->carrier_pixel_data_size) {
            fseek(carrier, opts->carrier_padding, SEEK_CUR);
            fseek(result, opts->carrier_padding, SEEK_CUR);
            counter = 0;
        }
        hiding_byte = fgetc(hiding);
        encrypt_hiding = encrypt_decrypt(hiding_byte);
        for (int i = 0; i < 8; i++) {
            bit_process((uint8_t) encrypt_hiding, i);
            printf("[carrier] Read : %ld\n", ftell(carrier));
            fread(&byte, sizeof(uint8_t), 1, carrier);
            if (bit == 1 && (get_LSB(byte) == 0)) byte |= 1;
            if (bit == 0 && (get_LSB(byte) == 1)) byte &= ~1;
            printf("[result] Write: %ld\n", ftell(result));
            fwrite(&byte, sizeof(uint8_t), 1, result);
            counter++;
        }
        hiding_size++;
    }


    rewind(carrier);
    rewind(hiding);
    rewind(result);
    fclose(carrier);
    fclose(hiding);
    fclose(result);
}


void foo(uint8_t bit, FILE *carrier, FILE *result, int *carrier_pos, int *result_pos) {
    uint8_t byte;
    printf("[carrier] Read : %ld\n", ftell(carrier));
    fread(&byte, sizeof(uint8_t), 1, carrier);
    (*carrier_pos) += 1;
    if (bit == 1 && (get_LSB(byte) == 0)) byte |= 1;
    if (bit == 0 && (get_LSB(byte) == 1)) byte &= ~1;

    printf("[result] Write: %ld\n", ftell(result));
    fwrite(&byte, sizeof(uint8_t), 1, result);
    (*result_pos) += 1;
}


uint8_t bit_process(uint8_t value, int index) {
    uint8_t byte = 0;
    byte |= (value >> (7 - index)) & 1; // Set the LSB to the corresponding bit in value
    return byte;
}


int get_LSB(uint8_t value) {
    return value & 1;
}


int encrypt_decrypt(int input) {
    uint8_t key = 'Z';
    return input ^ key;
}


void validate_file_size(struct options_image *opts) {
    char name[20] = {0};

    /* carrier */
    strcpy(name, opts->carrier_name);
    printf("\n[ Name = %s ]\n", opts->carrier_name);
    printf("carrier width = %d\n", opts->carrier_width);
    printf("carrier height = %d\n", opts->carrier_height);
    printf("carrier bpp = %d bits (%d bytes)\n", opts->carrier_bpp, opts->carrier_bpp / 8);
    printf("carrier row_size = %d bytes\n", opts->carrier_row_size);
    printf("carrier pixel_data_size = %d\n", opts->carrier_pixel_data_size);
    printf("carrier padding = %d\n", opts->carrier_padding);
    printf("carrier offset = %u\n", opts->carrier_offset);
    memset(name, 0, 20);

    /* hiding */
    strcpy(name, opts->hiding_name);
    printf("\n[ Name = %s ]\n", opts->hiding_name);
    printf("hiding width = %d\n", opts->hiding_width);
    printf("hiding height = %d\n", opts->hiding_height);
    printf("hiding bpp = %d bits (%d bytes)\n", opts->hiding_bpp, opts->hiding_bpp / 8);
    printf("hiding row_size = %d bytes\n", opts->hiding_row_size);
    printf("hiding pixel_data_size = %d\n", opts->hiding_pixel_data_size);
    printf("hiding padding = %d\n", opts->hiding_padding);
}



