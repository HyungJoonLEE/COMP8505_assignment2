#ifndef COMP8505_ASSIGNMENT2_COMMON_H
#define COMP8505_ASSIGNMENT2_COMMON_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <getopt.h>
#include <regex.h>
#include <inttypes.h>


struct options_image {
    /* carrier */
    char carrier_name[32];
    char carrier_type[5];
    unsigned int carrier_size;
    unsigned int carrier_pixel;
    unsigned int carrier_actual;

    /* hiding */
    char hiding_name[32];
    char hiding_type[5];
    unsigned int hiding_size;
    unsigned int hiding_pixel;

    /* result */
    char result_name[32];
    char result_type[5];
    unsigned int result_size;
    unsigned int result_pixel;

    /* flag for encode/decode */
    char flag;
};



#endif //COMP8505_ASSIGNMENT2_COMMON_H
