#ifndef COMP8505_ASSIGNMENT2_STEGO_H
#define COMP8505_ASSIGNMENT2_STEGO_H

// TODO: the main function that will contain the general functionality
//  such as parsing command line arguments, checking file sizes, file formats, etc.
#include "common.h"

void options_init_image(struct options_image *opts);
void parse_command_line(int argc, char *argv[], struct options_image *opts);
void get_file_info(struct options_image *opts);
void check_file_format(FILE *image_file, char* file_type);
unsigned int check_pixel_num(FILE *image_file);
unsigned int check_image_size(FILE *image_file);
unsigned int check_file_size(FILE *image_file);
uint32_t get_width(FILE* image_file);
uint32_t get_height(FILE* image_file);
uint16_t get_bbp(FILE* image_file);
uint32_t get_offset(FILE* image_file);

#endif //COMP8505_ASSIGNMENT2_STEGO_H
