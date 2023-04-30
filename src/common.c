#include "common.h"

void print_info_option_image(struct options_image *opts) {
    if (strlen(opts->carrier_name) > 0 && strlen(opts->hiding_name)) {
        puts("[ Encoding Mode ]");
        printf("Carrier Image = %s\n", opts->carrier_name);
        printf("Hiding Image  = %s\n", opts->hiding_name);
    }
    else {
        puts("[ Decoding Mode ]");
        printf("Result Image File = %s\n", opts->result_name);
    }
}
