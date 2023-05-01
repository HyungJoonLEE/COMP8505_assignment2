#include "image.h"

void create_temp_result_image_file(struct options_image *opts) {
    char *original = opts->carrier_name;
    char *result = "result.bmp";

    FILE *inptr = fopen(original, "rb");
    if (inptr == NULL) {
        fprintf(stderr, "Could not open %s.\n", original);
        exit(EXIT_FAILURE);
    }

    FILE *outptr = fopen(result, "wb");
    if (outptr == NULL) {
        fclose(inptr);
        fprintf(stderr, "Could not create %s.\n", result);
        exit(EXIT_FAILURE);
    }


    int c;
    while ((c = fgetc(inptr)) != EOF)
        fputc(c, outptr);
    fseek(outptr, 0, SEEK_END);

    fclose(inptr);
    fclose(outptr);
}
