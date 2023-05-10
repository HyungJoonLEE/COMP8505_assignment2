#include "image.h"


void create_result_image_file(struct options_image *opts) {
    uint8_t bit = 0;
    uint8_t byte;
    int temp;
    int hide_pointer;
    int encrypt_length = 0;
    int encrypt_file_name = 0;
    int encrypt_byte = 0;
    uint32_t counter = 0;
    uint32_t hiding_size = 0;
    int carrier_byte = 0;


    char result_name[] = "result.bmp";
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

    fseek(carrier, opts->carrier_offset, SEEK_SET);
    fseek(result, opts->carrier_offset, SEEK_SET);

    printf("\n\nStart to hide [ %s ] in [ %s ] ... \n", opts->hiding_name, opts->carrier_name);


    /* Hiding length of file name */
    encrypt_length = encrypt_decrypt((int) strlen(opts->hiding_name));
    for (int i = 0; i < 8; i++) {
        if (counter == opts->carrier_pixel_data_size) {
            fseek(carrier, opts->carrier_padding, SEEK_CUR);
            fseek(result, opts->carrier_padding, SEEK_CUR);
            counter = 0;
        }

        bit = bit_process((uint8_t) encrypt_length, i);

        fread(&byte, sizeof(uint8_t), 1, carrier);
        if (bit == 1 && (get_LSB(byte) == 0)) byte |= 1;
        if (bit == 0 && (get_LSB(byte) == 1)) byte &= ~1;

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

            fread(&byte, sizeof(uint8_t), 1, carrier);
            if (bit == 1 && (get_LSB(byte) == 0)) byte |= 1;
            if (bit == 0 && (get_LSB(byte) == 1)) byte &= ~1;

            fwrite(&byte, sizeof(uint8_t), 1, result);
            counter++;
        }
    }

    /* Hiding actual size */
    temp = (int) opts->hiding_actual_size;
    for (int i = 0; i < 4; i++) {
        hide_pointer = (uint8_t) encrypt_decrypt((int) opts->hiding_actual_size);
        for (int j = 0; j < 8; j++) {
            if (counter == opts->carrier_pixel_data_size) {
                fseek(carrier, opts->carrier_padding, SEEK_CUR);
                fseek(result, opts->carrier_padding, SEEK_CUR);
                counter = 0;
            }

            bit = bit_process((uint8_t) hide_pointer, j);

            fread(&byte, sizeof(uint8_t), 1, carrier);
            if (bit == 1 && (get_LSB(byte) == 0)) byte |= 1;
            if (bit == 0 && (get_LSB(byte) == 1)) byte &= ~1;

            fwrite(&byte, sizeof(uint8_t), 1, result);
            counter++;
        }
        opts->hiding_actual_size >>= 8;
    }
    opts->hiding_actual_size = (unsigned int) temp;


    /* Hiding img file */
    while (hiding_size != opts->hiding_actual_size) {
        fread(&byte, sizeof(uint8_t), 1, hiding);
        encrypt_byte = encrypt_decrypt(byte);
        for (int i = 0; i < 8; i++) {
            if (counter == opts->carrier_pixel_data_size) {
                fseek(carrier, opts->carrier_padding, SEEK_CUR);
                fseek(result, opts->carrier_padding, SEEK_CUR);
                counter = 0;
            }

            bit = bit_process((uint8_t) encrypt_byte, i);

            fread(&byte, sizeof(uint8_t), 1, carrier);
            if (bit == 1 && (get_LSB(byte) == 0)) byte |= 1;
            if (bit == 0 && (get_LSB(byte) == 1)) byte &= ~1;

            fwrite(&byte, sizeof(uint8_t), 1, result);
            counter++;
        }
        hiding_size++;
    }

    printf("Successfully to hide [ %s ] in [ %s ]\n\n\n", opts->hiding_name, opts->carrier_name);

    rewind(carrier);
    rewind(hiding);
    rewind(result);

    fclose(carrier);
    fclose(hiding);
    fclose(result);
}


void interpret_result_image(struct options_image *opts) {
    uint8_t byte;
    int bits[8] = {0};
    uint8_t krypto_length = 0;
    char* krypto_file_name;
    uint8_t size[4];
    uint8_t decrypt_byte;
    uint32_t counter = 0;
    uint32_t krypto_size = 0;
    uint32_t krypto_size_checker = 0;
    FILE *result = NULL;
    FILE *kryptos = NULL;


    result = fopen(opts->result_name, "rb");
    if (result == NULL) {
        fprintf(stderr, "[create_result_image_file] Could not open %s.\n", opts->result_name);
        exit(EXIT_FAILURE);
    }


    fseek(result, opts->result_offset, SEEK_SET);

    printf("\nStart to resolve %s ... \n", opts->result_name);

    /* decrypt bmp file name length */
    for (int i = 7; i >= 0; i--) {
        if (counter == opts->result_pixel_data_size) {
            fseek(result, opts->result_padding, SEEK_CUR);
            counter = 0;
        }
        fread(&bits[i], sizeof(uint8_t), 1, result);
        bits[i] = (uint8_t) get_LSB((uint8_t) bits[i]);
        byte = (uint8_t) ((byte << 1) | bits[i]);
        counter++;
    }
    krypto_length = (uint8_t) encrypt_decrypt(byte);
    memset(bits, 0, 8);


    /* decrypt bmp file name */
    krypto_file_name = malloc(sizeof(char) * krypto_length + 1);
    memset(krypto_file_name, 0, sizeof(char) * krypto_length + 1);
    for (int i = 0; i < krypto_length; i++) {
        for (int j = 7; j >= 0; j--) {
            if (counter == opts->result_pixel_data_size) {
                fseek(result, opts->result_padding, SEEK_CUR);
                counter = 0;
            }
            fread(&bits[j], sizeof(uint8_t), 1, result);
            bits[j] = get_LSB((uint8_t) bits[j]);
            byte = (uint8_t) ((byte << 1) | bits[j]);
            counter++;
        }
        krypto_file_name[i] = (char) encrypt_decrypt(byte);
        memset(bits, 0, 8);
    }


    for (int i = 0; i < 4; i++) {
        for (int j = 7; j >= 0; j--) {
            if (counter == opts->result_pixel_data_size) {
                fseek(result, opts->result_padding, SEEK_CUR);
                counter = 0;
            }
            fread(&bits[j], sizeof(uint8_t), 1, result);
            bits[j] = (uint8_t) get_LSB((uint8_t) bits[j]);
            byte = (uint8_t) ((byte << 1) | bits[j]);
            counter++;
        }
        decrypt_byte = (uint8_t) encrypt_decrypt(byte);
        if (i == 0) size[3] = decrypt_byte;
        if (i == 1) size[2] = decrypt_byte;
        if (i == 2) size[1] = decrypt_byte;
        if (i == 3) size[0] = decrypt_byte;
    }
    krypto_size = ((uint32_t) size[0] << 24) |
                  ((uint32_t) size[1] << 16) |
                  ((uint32_t) size[2] << 8) |
                  ((uint32_t) size[3]);


    kryptos = fopen(krypto_file_name, "wb");
    if (kryptos == NULL) {
        fclose(result);
        fprintf(stderr, "[create_result_image_file] Could not create %s.\n", krypto_file_name);
        exit(EXIT_FAILURE);
    }


    /* decrypt bmp file header */
    if (strstr(krypto_file_name, "bmp") != NULL) {
        for (int i = 0; i < 54; i++) {
            for (int j = 7; j >= 0; j--) {
                if (counter == opts->result_pixel_data_size) {
                    fseek(result, opts->result_padding, SEEK_CUR);
                    counter = 0;
                }
                fread(&bits[j], sizeof(uint8_t), 1, result);
                bits[j] = get_LSB((uint8_t) bits[j]);
                byte = (uint8_t) ((byte << 1) | bits[j]);
                counter++;
            }
            decrypt_byte = (uint8_t) encrypt_decrypt(byte);
            fwrite(&decrypt_byte, sizeof(uint8_t), 1, kryptos);
        }


        fseek(kryptos, 54, SEEK_SET);
        while (krypto_size_checker < krypto_size - 54) {
            for (int j = 7; j >= 0; j--) {
                if (counter == opts->result_pixel_data_size) {
                    fseek(result, opts->result_padding, SEEK_CUR);
                    counter = 0;
                }
                fread(&bits[j], sizeof(uint8_t), 1, result);
                bits[j] = get_LSB((uint8_t) bits[j]);
                byte = (uint8_t) ((byte << 1) | bits[j]);
                counter++;
            }
            krypto_size_checker++;
            byte = (uint8_t) encrypt_decrypt(byte);
            fwrite(&byte, sizeof(uint8_t), 1, kryptos);
            memset(bits, 0, 8);
        }
    }

    else {
        while (krypto_size_checker < krypto_size) {
            for (int j = 7; j >= 0; j--) {
                if (counter == opts->result_pixel_data_size) {
                    fseek(result, opts->result_padding, SEEK_CUR);
                    counter = 0;
                }
                fread(&bits[j], sizeof(uint8_t), 1, result);
                bits[j] = get_LSB((uint8_t) bits[j]);
                byte = (uint8_t) ((byte << 1) | bits[j]);
                counter++;
            }
            krypto_size_checker++;
            byte = (uint8_t) encrypt_decrypt(byte);
            fwrite(&byte, sizeof(uint8_t), 1, kryptos);
            memset(bits, 0, 8);
        }
    }


    fseek(kryptos, 0, SEEK_END);

    printf("Successfully decrypt [ %s ]\n", opts->result_name);
    printf("Hidden image [ %s ] found\n\n", krypto_file_name);

    fclose(result);
    fclose(kryptos);
    free(krypto_file_name);
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

