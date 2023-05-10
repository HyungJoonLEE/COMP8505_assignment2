#include "stego.h"

void options_init_image(struct options_image *opts) {
    memset(opts, 0, sizeof(struct options_image));
}


void parse_command_line(int argc, char *argv[], struct options_image *opts) {
    int c;
    int option_index = 0;
    int carrier_flag = 0;
    int hiding_flag = 0;
    int result_flag = 0;
    int encode_flag = 0;
    int decode_flag = 0;

    static struct option long_options[] = {
            {"carrier", required_argument, 0, 'c'},
            {"hiding", required_argument, 0, 'h'},
            {"result", required_argument, 0, 'r'},
            {"encode", no_argument, 0, 'e'},
            {"decode", no_argument, 0, 'd'},
            {0, 0, 0, 0}
    };


    while((c = getopt_long(argc, argv, "c:h:r:ed", long_options, &option_index)) != -1) {  // NOLINT(concurrency-mt-unsafe)
        switch(c) {
            case 'c': {
                strcpy(opts->carrier_name, optarg);
                if (strlen(opts->carrier_name) > 0) carrier_flag = 1;
                break;
            }
            case 'h': {
                strcpy(opts->hiding_name, optarg);
                if (strlen(opts->hiding_name) > 0) hiding_flag = 1;

                break;
            }
            case 'r': {
                strcpy(opts->result_name, optarg);
                if (strlen(opts->result_name) > 0) result_flag = 1;
                break;
            }
            case 'e': {
                if (decode_flag) {
                    fprintf(stderr, "Error: Cannot use both -e/--encode and -d/--decode flags.\n");
                    exit(EXIT_FAILURE);
                }
                if (strlen(opts->carrier_name) == 0 || strlen(opts->hiding_name) == 0) {
                    fprintf(stderr, "Error: Must put either carrier image or hiding image file to encode");
                    exit(EXIT_FAILURE);
                }
                encode_flag = 1;
                opts->flag = 'e';
                break;
            }
            case 'd': {
                if (encode_flag) {
                    fprintf(stderr, "Error: Cannot use both -e/--encode and -d/--decode flags.\n");
                    exit(EXIT_FAILURE);
                }
                if (strlen(opts->result_name) == 0) {
                    fprintf(stderr, "Error: Must put result image file to decode");
                    exit(EXIT_FAILURE);
                }
                decode_flag = 1;
                opts->flag = 'd';
                break;
            }
            default:
                printf("Usage: %s "
                       "[-c | --carrier FILENAME] "
                       "[-h | --hiding FILENAME] "
                       "[-r | --result FILENAME] "
                       "[-e | --encode]"
                       "[-d | --decode]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }
    if ((carrier_flag || hiding_flag) && result_flag) {
        fprintf(stderr, "Error: Cannot process carrier, hiding, and result image at the same time.\n");
        exit(EXIT_FAILURE);
    }
}


void get_file_info(struct options_image *opts) {

    if (opts->flag == 'e') {
        if (access(opts->carrier_name, F_OK) == 0 && access(opts->hiding_name, F_OK) == 0) {
            FILE* carrier = fopen(opts->carrier_name, "rb");
            FILE* hiding  = fopen(opts->hiding_name, "rb");

            process_img_info(carrier, opts, 'c');
            process_img_info(hiding, opts, 'h');

            /* if carrier size is not enough to put hiding file name */
            /* len of file name + file name + actual size + hiding header */
            if (opts->carrier_img_size > (opts->hiding_actual_size * 8 + strlen(opts->hiding_name) * 8) + 8 + 32) {
                /* Calculate number of bits */
                puts("\n======= [ Encoding Mode ] =======\n");
                printf("* Carrier Image : %s\n"
                       "* Carrier Format  : %s\n"
                       "* Carrier Size    : %d bytes\n",
                       opts->carrier_name,
                       opts->carrier_type,
                       opts->carrier_actual_size);

                printf("\n\n* Hiding Image : %s\n"
                       "* Hiding Format  : %s\n"
                       "* Hiding Size    : %d bytes\n\n"
                       "=================================\n",
                       opts->hiding_name,
                       opts->hiding_type,
                       opts->hiding_actual_size);
            } else {
                fprintf(stderr, "Error: Carrier FILE is not big enough to hide image\n");
                exit(EXIT_FAILURE);
            }
            fclose(carrier);
            fclose(hiding);
        } else {
            fprintf(stderr, "Error: FILES are not exist in the directory\n");
            exit(EXIT_FAILURE);
        }
    }
    if (opts->flag == 'd') {
        if (access(opts->result_name, F_OK) == 0) {
            FILE* result  = fopen(opts->result_name, "rb");
            process_img_info(result, opts, 'r');

            puts("\n======= [ Decoding Mode ] =======\n");
            printf("Result Image   : %s\n"
                   "* Result Format  : %s\n"
                   "* Result Size    : %d bytes\n\n"
                   "=================================\n",
                   opts->result_name,
                   opts->result_type,
                   opts->result_actual_size);
            fclose(result);
        } else {
            fprintf(stderr, "Error: FILES are not exist in the directory\n");
            exit(EXIT_FAILURE);
        }
    }
}


void check_file_format(FILE *image_file, struct options_image *opts, char flag) {
    uint16_t type;

    fseek(image_file, 0, SEEK_SET);
    fread(&type, sizeof(uint16_t), 1, image_file);

    if (opts->flag == 'e') {
        if (flag == 'c') {
            if (type == 0x4D42) strcpy(opts->carrier_type, "BMP [0x4D42]");
        }
        if (flag == 'h') {
            if (type == 0x4D42) strcpy(opts->hiding_type, "BMP [0x4D42]");
            if (type == 0xD8FF) strcpy(opts->hiding_type, "JPG [0xD8FF]");
            if (type == 0x5089) strcpy(opts->hiding_type, "PNG [0x5089]");
            if (type == 0x5025) strcpy(opts->hiding_type, "PDF [0x5025]");
        }
    }
    if (opts->flag == 'd') {
        if (flag == 'r') {
            if (type == 0x4D42) strcpy(opts->result_type, "BMP [0x4D42]");
        }
    }

    rewind(image_file);
}


void check_image_size(FILE* image_file, struct options_image *opts, char flag) {
    unsigned int img_size;

    fseek(image_file, 34, SEEK_SET);
    fread(&img_size, sizeof(uint32_t), 1, image_file);
    rewind(image_file);

    if (flag == 'c') opts->carrier_img_size = img_size;
    if (flag == 'h') opts->hiding_img_size = img_size;
    if (flag == 'r') opts->result_img_size = img_size;
}


void check_file_size(FILE* image_file, struct options_image *opts, char flag) {
    unsigned int file_size;
    /* BITMAP */
    if (strstr(opts->hiding_type, "BMP") != NULL) {
        fseek(image_file, 2, SEEK_SET);
        fread(&file_size, sizeof(uint32_t), 1, image_file);
    }


    /* OTHERS */
    else {
        fseek(image_file, 0, SEEK_SET);
        fseek(image_file, 0L, SEEK_END);
        file_size = (unsigned int) ftell(image_file);
    }

    rewind(image_file);

    if (flag == 'c') opts->carrier_actual_size = file_size;
    if (flag == 'h') opts->hiding_actual_size = file_size;
    if (flag == 'r') opts->result_actual_size = file_size;
}


void check_width(FILE* image_file, struct options_image *opts, char flag) {
    unsigned int width;

    fseek(image_file, 18, SEEK_SET);
    fread(&width, sizeof(uint32_t), 1, image_file);
    rewind(image_file);

    if (flag == 'c') opts->carrier_width = width;
    if (flag == 'h') {
        if (strstr(opts->hiding_type, "BMP") == NULL) opts->hiding_width = 0;
        else opts->hiding_width = width;
    }
    if (flag == 'r') opts->result_width = width;
}


void check_height(FILE* image_file, struct options_image *opts, char flag) {
    uint32_t height;

    fseek(image_file, 22, SEEK_SET);
    fread(&height, sizeof(uint32_t), 1, image_file);

    rewind(image_file);

    if (flag == 'c') opts->carrier_height = height;
    if (flag == 'h') {
        if (strstr(opts->hiding_type, "BMP") == NULL) opts->hiding_height = 0;
        else opts->hiding_height = height;
    }
    if (flag == 'r') opts->result_height = height;
}


void check_bpp(FILE* image_file, struct options_image *opts, char flag) {
    uint16_t bpp;

    fseek(image_file, 28, SEEK_SET);
    fread(&bpp, sizeof(uint16_t), 1, image_file);

    rewind(image_file);

    if (flag == 'c') opts->carrier_bpp = bpp;
    if (flag == 'h') opts->hiding_bpp = bpp;
    if (flag == 'r') opts->result_bpp = bpp;
}


void check_offset(FILE* image_file, struct options_image *opts, char flag) {
    uint32_t offset;

    fseek(image_file, 10, SEEK_SET);
    fread(&offset, sizeof(uint32_t), 1, image_file);

    rewind(image_file);
    if (flag == 'c') opts->carrier_offset = offset;
    if (flag == 'r') opts->result_offset = offset;
}


void check_additional_info(struct options_image *opts, char flag) {
    if (flag == 'c') {
        opts->carrier_pixel_num = opts->carrier_width * opts->carrier_height * opts->carrier_bpp;
        opts->carrier_row_size = (((opts->carrier_width * opts->carrier_bpp) + 31) / 32) * 4;
        opts->carrier_pixel_data_size = (opts->carrier_width * opts->carrier_bpp) / 8;
        opts->carrier_padding = (uint16_t) (opts->carrier_row_size - (opts->carrier_width * 3));
    }
    if (flag == 'h') {
        opts->hiding_pixel_num = opts->hiding_width * opts->hiding_height * opts->hiding_bpp;
        opts->hiding_row_size = (((opts->hiding_width * opts->hiding_bpp) + 31) / 32) * 4;
        opts->hiding_pixel_data_size = (opts->hiding_width * opts->hiding_bpp) / 8;
        opts->hiding_padding = (uint16_t) (opts->hiding_row_size - (opts->hiding_width * 3));
    }
    if (flag == 'r') {
        opts->result_pixel_num = opts->result_width * opts->result_height * opts->result_bpp;
        opts->result_row_size = (((opts->result_width * opts->result_bpp) + 31) / 32) * 4;
        opts->result_pixel_data_size = (opts->result_width * opts->result_bpp) / 8;
        opts->result_padding = (uint16_t) (opts->result_row_size - (opts->result_width * 3));
    }
}


void process_img_info(FILE* image_file, struct options_image *opts, char flag) {
    if (flag == 'c') {
        check_file_format(image_file, opts, 'c');
        check_image_size(image_file, opts, 'c');
        check_offset(image_file, opts, 'c');
        check_width(image_file, opts, 'c');
        check_height(image_file, opts, 'c');
        check_bpp(image_file, opts, 'c');
        check_file_size(image_file, opts, 'c');
        check_additional_info(opts, 'c');
    }
    if (flag == 'h') {
        check_file_format(image_file, opts, 'h');
        check_image_size(image_file, opts, 'h');
        check_width(image_file, opts, 'h');
        check_height(image_file, opts, 'h');
        check_bpp(image_file, opts, 'h');
        check_file_size(image_file, opts, 'h');
        check_additional_info(opts, 'h');
    }
    if (flag == 'r') {
        check_file_format(image_file, opts, 'r');
        check_image_size(image_file, opts, 'r');
        check_file_size(image_file, opts, 'r');
        check_offset(image_file, opts, 'r');
        check_width(image_file, opts, 'r');
        check_height(image_file, opts, 'r');
        check_bpp(image_file, opts, 'r');
        check_file_size(image_file, opts, 'r');
        check_additional_info(opts, 'r');
    }
}

