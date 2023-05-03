#include "image.h"

uint8_t reverse_byte(uint8_t byte);

void create_result_image_file(struct options_image *opts) {
    uint8_t bit = 0;
    int encrypt_length = 0;
    int encrypt_file_name = 0;
    int encrypt_hiding_byte = 0;
    uint32_t counter = 0;
    uint32_t hiding_size = 0;
    int carrier_byte = 0;
    int hiding_byte = 0;
    uint8_t byte;
    uint8_t reversed_byte = 0;


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

    validate_file_size(opts, 'e');


    fseek(carrier, opts->carrier_offset, SEEK_SET);
    fseek(result, opts->carrier_offset, SEEK_SET);




    /* Hiding length of file name */
    encrypt_length = encrypt_decrypt((int) strlen(opts->hiding_name));
    for (int i = 0; i < 8; i++) {
        if (counter == opts->carrier_pixel_data_size) {
            fseek(carrier, opts->carrier_padding, SEEK_CUR);
            fseek(result, opts->carrier_padding, SEEK_CUR);
            counter = 0;
        }
        bit = bit_process((uint8_t) encrypt_length, i);
//        printf("[carrier] Read : %ld\n", ftell(carrier));
        fread(&byte, sizeof(uint8_t), 1, carrier);
        if (bit == 1 && (get_LSB(byte) == 0)) byte |= 1;
        if (bit == 0 && (get_LSB(byte) == 1)) byte &= ~1;

//        printf("[result] Write: %ld\n", ftell(result));
        fwrite(&byte, sizeof(uint8_t), 1, result);
        counter++;
    }

    /* Hiding file name */
    for (int i = 0; i < strlen(opts->hiding_name); i++) {
        encrypt_file_name = encrypt_decrypt((int) opts->hiding_name[i]);
//        printf("%u -> %u\n", opts->hiding_name[i], encrypt_file_name);
        for (int j = 0; j < 8; j++) {
            if (counter == opts->carrier_pixel_data_size) {
                fseek(carrier, opts->carrier_padding, SEEK_CUR);
                fseek(result, opts->carrier_padding, SEEK_CUR);
                counter = 0;
            }

            bit = bit_process((uint8_t) encrypt_file_name, j);
//            printf("[carrier] Read : %ld\n", ftell(carrier));
            fread(&byte, sizeof(uint8_t), 1, carrier);
            if (bit == 1 && (get_LSB(byte) == 0)) byte |= 1;
            if (bit == 0 && (get_LSB(byte) == 1)) byte &= ~1;
//            printf("[result] Write: %ld\n", ftell(result));
            fwrite(&byte, sizeof(uint8_t), 1, result);
            counter++;
        }
    }


    /* Hiding img file */
    while (hiding_size != opts->hiding_actual) {
        fread(&byte, sizeof(uint8_t), 1, hiding);
        encrypt_hiding_byte = encrypt_decrypt(hiding_byte);
        for (int i = 0; i < 8; i++) {
            if (counter == opts->carrier_pixel_data_size) {
                fseek(carrier, opts->carrier_padding, SEEK_CUR);
                fseek(result, opts->carrier_padding, SEEK_CUR);
                counter = 0;
            }
            bit = bit_process((uint8_t) encrypt_hiding_byte, i);
//            printf("[carrier] Read : %ld\n", ftell(carrier));
            fread(&byte, sizeof(uint8_t), 1, carrier);
            if (bit == 1 && (get_LSB(byte) == 0)) byte |= 1;
            if (bit == 0 && (get_LSB(byte) == 1)) byte &= ~1;
//            printf("[result] Write: %ld\n", ftell(result));
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


void interpret_result_image(struct options_image *opts) {
    uint8_t byte;
    int bits[8] = {0};
    uint8_t krypto_length = 0;
    char* krypto_file_name;
    uint8_t decrypt_byte;
    uint32_t counter = 0;
    uint32_t krypto_size = 0;
    uint32_t krypto_size_checker = 0;


    FILE *result = fopen(opts->result_name, "rb");
    if (result == NULL) {
        fprintf(stderr, "[create_result_image_file] Could not open %s.\n", opts->result_name);
        exit(EXIT_FAILURE);
    }


    validate_file_size(opts, 'd');
    fseek(result, opts->result_offset, SEEK_SET);

    /* decrypt bmp file name length */
    for (int i = 7; i >= 0; i--) {
        if (counter == opts->result_pixel_data_size) {
            fseek(result, opts->result_padding, SEEK_CUR);
            counter = 0;
        }
//        printf("Read : %ld\n", ftell(result));
        fread(&bits[i], sizeof(uint8_t), 1, result);
        bits[i] = (uint8_t) get_LSB((uint8_t) bits[i]);
        byte = (uint8_t) ((byte << 1) | bits[i]);
        counter++;
    }
    krypto_length = (uint8_t) encrypt_decrypt(byte);
    printf("%d\n", krypto_length);
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
    printf("%s\n", krypto_file_name);


    FILE *kryptos = fopen("kkk.bmp", "wb");
    if (result == NULL) {
        fclose(result);
        fprintf(stderr, "[create_result_image_file] Could not create %s.\n", krypto_file_name);
        exit(EXIT_FAILURE);
    }
    printf("Read : %ld\n", ftell(result));


    /* decrypt bmp file header */
    for (int i = 0; i < 54; i++) {
        for (int j = 7; j >= 0; j--) {
            fread(&bits[j], sizeof(uint8_t), 1, result);
            bits[j] = get_LSB((uint8_t) bits[j]);
            byte = (uint8_t) ((byte << 1) | bits[j]);
            counter++;
        }

        for (int j = 0; j < 8; j++) {
            printf("%u", bits[j]);
        }
        printf("\n");
//        fwrite(&decrypt_byte, sizeof(uint8_t), 1, kryptos);
    }


    krypto_size = check_file_size(kryptos);
    printf("%d\n", krypto_size);

    while(krypto_size_checker < krypto_size) {
        for (int j = 7; j >= 0; j--) {
            if (counter == opts->result_pixel_data_size) {
                fseek(result, opts->result_padding, SEEK_CUR);
                counter = 0;
            }
            fread(&bits[j], sizeof(uint8_t), 1, result);
            bits[j] = get_LSB((uint8_t) bits[j]);
            byte = (uint8_t) ((byte << 1) | bits[j]);
            counter++;
            krypto_size_checker++;
        }
        byte = (uint8_t) encrypt_decrypt(byte);
        fwrite(&byte, sizeof(uint8_t), 1, kryptos);
        memset(bits, 0, 8);
    }
    fseek(kryptos, 0, SEEK_END);

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


void validate_file_size(struct options_image *opts, char type) {
    char name[20] = {0};

    if (type == 'e') {
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
    if (type == 'd') {
        printf("result width = %d\n", opts->result_width);
        printf("result height = %d\n", opts->result_height);
        printf("result bpp = %d bits (%d bytes)\n", opts->result_bpp, opts->result_bpp / 8);
        printf("result row_size = %d bytes\n", opts->result_row_size);
        printf("result pixel_data_size = %d\n", opts->result_pixel_data_size);
        printf("result padding = %d\n", opts->result_padding);
        printf("result offset = %u\n", opts->result_offset);
    }
}


uint8_t reverse_byte(uint8_t byte) {
    uint8_t high_nibble = (byte & 0xF0) >> 4; // Extract the high nibble and shift it to the right
    uint8_t low_nibble = (byte & 0x0F);       // Extract the low nibble
    return (low_nibble << 4) | high_nibble;  // Shift the low nibble to the left and combine with the high nibble
}



