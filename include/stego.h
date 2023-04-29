#ifndef COMP8505_ASSIGNMENT2_STEGO_H
#define COMP8505_ASSIGNMENT2_STEGO_H

// TODO: the main function that will contain the general functionality
//  such as parsing command line arguments, checking file sizes, file formats, etc.
#include "common.h"

void options_init_image(struct options_image *opts);
void parse_command_line(int argc, char *argv[], struct options_image *opts);


#endif //COMP8505_ASSIGNMENT2_STEGO_H
