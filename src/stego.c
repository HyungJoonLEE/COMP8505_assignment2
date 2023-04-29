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

