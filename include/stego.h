#ifndef COMP8505_ASSIGNMENT2_STEGO_H
#define COMP8505_ASSIGNMENT2_STEGO_H

// TODO: the main function that will contain the general functionality
//  such as parsing command line arguments, checking file sizes, file formats, etc.
#include "common.h"

void options_init_image(struct options_image *opts);
void parse_command_line(int argc, char *argv[], struct options_image *opts);
void get_file_info(struct options_image *opts);
void check_file_format(FILE *image_file, struct options_image *opts, char flag);
void check_image_size(FILE* image_file, struct options_image *opts, char flag);
void check_file_size(FILE* image_file, struct options_image *opts, char flag);
void check_width(FILE* image_file, struct options_image *opts, char flag);
void check_height(FILE* image_file, struct options_image *opts, char flag);
void check_bpp(FILE* image_file, struct options_image *opts, char flag);
void check_offset(FILE* image_file, struct options_image *opts, char flag);
void check_additional_info(struct options_image *opts, char flag);
void process_img_info(FILE* image_file, struct options_image *opts, char flag);

#endif //COMP8505_ASSIGNMENT2_STEGO_H
