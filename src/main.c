#include "stego.h"
#include "image.h"
#include "utils.h"

int main(int argc, char* argv[]) {
    struct options_image opts;

    options_init_image(&opts);
    parse_command_line(argc, argv, &opts);
    print_info_option_image(&opts);

}