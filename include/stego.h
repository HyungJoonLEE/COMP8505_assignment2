#ifndef COMP8505_ASSIGNMENT2_STEGO_H
#define COMP8505_ASSIGNMENT2_STEGO_H

// TODO: the main function that will contain the general functionality
//  such as parsing command line arguments, checking file sizes, file formats, etc.
#include "common.h"

void options_init_image(struct options_image *opts);
void parse_command_line(int argc, char *argv[], struct options_image *opts);
void get_file_info(struct options_image *opts);
char* check_file_format(char* file_name);
unsigned int check_pixel_num(FILE *image_file);
unsigned int check_file_size(FILE *image_file);


#endif //COMP8505_ASSIGNMENT2_STEGO_H
