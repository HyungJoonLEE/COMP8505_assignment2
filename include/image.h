#ifndef COMP8505_ASSIGNMENT2_IMAGE_H
#define COMP8505_ASSIGNMENT2_IMAGE_H

// TODO:  contain all of the functions for the image processing and manipulation.
#include "common.h"
#include "stego.h"


void create_result_image_file(struct options_image *opts);
void interpret_result_image(struct options_image *opts);
int encrypt_decrypt(int input);
uint8_t bit_process(uint8_t value, int index);
int get_LSB(uint8_t value);
uint32_t get_width(FILE* img_file);
uint32_t get_height(FILE* img_file);
void validate_file_size(struct options_image *opts, char type);




#endif //COMP8505_ASSIGNMENT2_IMAGE_H
