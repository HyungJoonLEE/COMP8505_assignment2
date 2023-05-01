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
            {"hiding", required_argument, 0, 's'},
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
                       "[-s | --hiding FILENAME] "
                       "[-s | --result FILENAME] "
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
            FILE* carrier_image = fopen(opts->carrier_name, "rb");
            FILE* hiding_image  = fopen(opts->hiding_name, "rb");

            /* for result */
            opts->carrier_actual = check_file_size(carrier_image);

            /* Check image format */
            check_file_format(carrier_image, opts->carrier_type);
            check_file_format(hiding_image, opts->hiding_type);


            /* Calculate image size */
            opts->carrier_size = check_image_size(carrier_image);
            opts->hiding_size = check_image_size(hiding_image);

            /* if carrier size is not enough to put hiding file name */
//            if (opts->carrier_size > opts->hiding_size * 8 + 120) {
                /* Calculate number of bits */
                opts->carrier_pixel = check_pixel_num(carrier_image);
                opts->hiding_pixel = check_pixel_num(hiding_image);
                puts("[ Encoding Mode ]");
                printf("Carrier Image  : %s\n"
                       "Carrier Format : %s\n"
                       "Carrier Size   : %d bytes\n"
                       "Carrier Number : %d bits\n\n" ,
                       opts->carrier_name, opts->carrier_type, opts->carrier_size, opts->carrier_pixel);

                printf("Hiding Image  : %s\n"
                       "Hiding Format : %s\n"
                       "Hiding Size   : %d bytes\n"
                       "Hiding Number : %d bits\n" ,
                       opts->hiding_name, opts->hiding_type, opts->hiding_size, opts->hiding_pixel);
//            } else {
//                fprintf(stderr, "Error: Carrier FILE is not big enough to hide image\n");
//                exit(EXIT_FAILURE);
//            }
            fclose(carrier_image);
            fclose(hiding_image);
        } else {
            fprintf(stderr, "Error: FILES are not exist in the directory\n");
            exit(EXIT_FAILURE);
        }
    }
    if (opts->flag == 'd') {
        if (access(opts->result_name, F_OK) == 0) {
            FILE* result_image  = fopen(opts->hiding_name, "rb");

            /* Check image format */
            check_file_format(result_image, opts->result_type);

            /* Calculate image size */
            opts->result_size = check_image_size(result_image);

            /* Calculate number of bits */
            opts->result_pixel = check_pixel_num(result_image);
            puts("[ Decoding Mode ]");
            printf("Result Image  : %s\n"
                   "Result Format : %s\n"
                   "Result Size   : %d bytes\n"
                   "Result Number : %d bits\n" ,
                   opts->result_name, opts->result_type, opts->result_size, opts->result_pixel);
            fclose(result_image);
        } else {
            fprintf(stderr, "Error: FILES are not exist in the directory\n");
            exit(EXIT_FAILURE);
        }
    }
}


void check_file_format(FILE *image_file, char* file_type) {
    uint16_t type;

    fseek(image_file, 0, SEEK_SET);
    fread(&type, sizeof(short), 1, image_file);
    rewind(image_file);

    /**
     * Bitmap files have little-endian byte order,
     * which means the least significant byte comes first.
     * When reading a 16-bit value,
     * system seems to be interpreting it as big-endian,
     * which reverses the byte order.
     * **/
    type = (uint16_t) ((type >> 8) | (type << 8));

    if (type == 0x424D) strcpy(file_type, "BM");
}

unsigned int check_pixel_num(FILE *image_file) {
    uint32_t width, height;
    uint16_t bits_per_pixel;

    // Move to 18th byte behind
    fseek(image_file, 18, SEEK_SET);

    // Read the width uint32_t (22)
    fread(&width, sizeof(uint32_t), 1, image_file);

    // Read the height uint32_t (26)
    fread(&height, sizeof(uint32_t), 1, image_file);

    // Read the bits per pixel
    fseek(image_file, 2, SEEK_CUR);
    fread(&bits_per_pixel, sizeof(uint16_t), 1, image_file);
    rewind(image_file);

    return width * height * bits_per_pixel;
}


unsigned int check_image_size(FILE *image_file) {
    unsigned int file_size;

    fseek(image_file, 34, SEEK_SET);
    fread(&file_size, sizeof(uint32_t), 1, image_file);
    rewind(image_file);

    return file_size;
}


unsigned int check_file_size(FILE *image_file) {
    unsigned int file_size;

    fseek(image_file, 2, SEEK_SET);
    fread(&file_size, sizeof(uint32_t), 1, image_file);
    rewind(image_file);

    return file_size;
}
