#include <stdio.h>
#include <string.h>
#include <png.h>
#include <stdlib.h>

#include "matrix.c"
#include "image.c"

/**
 * Reads a png image and returns a matrix with the image representation
*/ 
Image readImage(char* filepath) {
    FILE *pFile = fopen(filepath, "rb");
    if(!pFile) {
        printf("\033[1;31m");
        printf("Error while reading file %s\n", filepath);
        printf("\033[0m");
        exit(1);
    }
    int len = 8; 
    char header[len]; 
    fread(header, 1, len, pFile); 
    // checks png signature
    int is_png = !png_sig_cmp(header, 0, len); 
    if (!is_png) {
        printf("\033[1;31m");
        printf("File %s is not a png image\n", filepath);
        printf("\033[0m");
        fclose(pFile);
        remove(filepath); 
        exit(1);
    }

    // allocate and initialize a png_struct structure for reading PNG file
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    // allocate and initialize a png_info structure
    png_infop info_ptr = png_create_info_struct(png_ptr);

    if(setjmp(png_jmpbuf(png_ptr))) {
        printf("\033[1;31m");
        printf("Error while reading file %s\n", filepath);
        printf("\033[0m");
        fclose(pFile);
        exit(1);
    }

    // initialize input/output for the PNG file
    png_init_io(png_ptr, pFile);
    png_set_sig_bytes(png_ptr, len);
    // read the PNG image information 
    png_read_info(png_ptr, info_ptr);

    int width = png_get_image_width(png_ptr, info_ptr);   
    int height = png_get_image_height(png_ptr, info_ptr); 
    // get number of color channels in image
    int channels = png_get_channels(png_ptr, info_ptr);  
    // get the number of passes for image interlacing
    int number_of_passes = png_set_interlace_handling(png_ptr);

    // update png_info structure
    png_read_update_info(png_ptr, info_ptr);

    if (setjmp(png_jmpbuf(png_ptr))) {
        printf("\033[1;31m");
        printf("Error while reading pixels\n");
        printf("\033[0m");
        fclose(pFile);
        exit(1);
    }

    png_bytep* row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
    for (int i = 0; i < height; i++) {
        row_pointers[i] = (png_byte*) malloc(png_get_rowbytes(png_ptr,info_ptr));
    }

    int** data = createMatrix(width, height);
    // read the entire image into memory
    png_read_image(png_ptr, row_pointers);
    for (int y = 0; y < height; y++) {
        png_byte* row = row_pointers[y];
        for (int x = 0; x < width; x++) {
            png_byte* pixel = &(row[x*channels]);
            data[x][y] = pixel[0];
        }
    }

    Image image;
    image.data = data;
    image.rows = width;
    image.cols = height;

    fclose(pFile);
    for (int i = 0; i < height; i++) free(row_pointers[i]);
    free(row_pointers);

    return image;
}