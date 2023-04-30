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

struct options_image {
    char carrier_name[32];
    unsigned int carrier_size;
    char hiding_name[32];
    unsigned int hiding_size;
    char result_name[32];
    unsigned int result_size;
    char flag;
};

void print_info_option_image(struct options_image *opts);

#endif //COMP8505_ASSIGNMENT2_COMMON_H
