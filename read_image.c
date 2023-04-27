#include <stdio.h>
#include <string.h>
#include <png.h>
#include <stdlib.h>

#include "matrix.c"
#include "image.c"


Image readImage(char* filepath) {
    FILE *pFile = fopen(filepath, "rb");
    if(!pFile) {
        printf("Error al leer el archivo %s\n", filepath);
        exit(1);
    }

    int len = 8; 
    char header[len]; 
    fread(header, 1, len, pFile); 
    int is_png = !png_sig_cmp(header, 0, len); 
    if (!is_png) {
        printf("Archivo %s no es una imagen en formato png\n", filepath);
        fclose(pFile);
        remove(filepath); 
        exit(1);
    }


    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if(setjmp(png_jmpbuf(png_ptr))) {
        printf("Error al obtener la informacion del archivo %s\n", filepath);
        fclose(pFile);
        exit(1);
    }


    png_init_io(png_ptr, pFile);
    png_set_sig_bytes(png_ptr, len); 
    png_read_info(png_ptr, info_ptr);
    int width = png_get_image_width(png_ptr, info_ptr);   
    int height = png_get_image_height(png_ptr, info_ptr); 
    int channels = png_get_channels(png_ptr, info_ptr);  
    int number_of_passes = png_set_interlace_handling(png_ptr);
    png_read_update_info(png_ptr, info_ptr);


    if (setjmp(png_jmpbuf(png_ptr))) {
        printf("Error durante la lectura de los pixeles\n");
        fclose(pFile);
        exit(1);
    }

    png_bytep *row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
    for (int i = 0; i < height; i++) {
        row_pointers[i] = (png_byte*) malloc(png_get_rowbytes(png_ptr,info_ptr));
    }

    int** data = createMatrix(width, height);
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