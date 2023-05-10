#ifndef COMP8505_ASSIGNMENT2_IMAGE_H
#define COMP8505_ASSIGNMENT2_IMAGE_H

// TODO:  contain all of the functions for the image processing and manipulation.
#include "common.h"
#include "stego.h"


void create_result_image(struct options_image *opts);
void interpret_result_image(struct options_image *opts);
int encrypt_decrypt(int input);
uint8_t bit_process(uint8_t value, int index);
int get_LSB(uint8_t value);




#endif //COMP8505_ASSIGNMENT2_IMAGE_H
