#include <stdio.h>
#include <string.h>
#include <png.h>
#include <stdlib.h>

#include "matrix.c"
#include "image.c"

int** conv2(int mRows, int mCols, int** matrix, int kRows, int kCols, int kernel[][kCols]);
void writeImage(char* filename, Image image);
Image sobel_filter(Image image);

/**
 * Computes convolution between matrix and a kernel
 * mRows: number of rows on matrix
 * mCols: number of columns on matrix
 * matrix: matrix to apply convolution
 * kRows: number of rows on kernel
 * kCols: number of columns on kernel
 * kernel: kernel matrix
*/
int** conv2(int mRows, int mCols, int** matrix, int kRows, int kCols, int kernel[][kCols]) {
    int rows = mRows+kRows-1; // number of rows of result
    int cols = mCols+kCols-1; // number of columns of result
    int** result = createMatrix(rows, cols); // result matrix

    // first sum
    for(int j = 1; j <= rows; j++) {
        // initial p
        int p_in = j-kRows+1;
        if(1 > p_in) p_in = 1; 
    
        // final p
        int p_fin = j;
        if(mRows < p_fin) p_fin = mRows;

        for(int p = p_in; p <= p_fin; p++) {
            // second sum
            for(int k = 1; k <= cols; k++) {
                // initial q
                int q_in = k-kCols+1;
                if(1 > q_in) q_in = 1;

                // final q
                int q_fin = k;
                if(mCols < q_fin) q_fin = mCols;

                // convolution calculation
                for(int q = q_in; q <= q_fin; q++) {
                    result[j-1][k-1] = result[j-1][k-1] + (matrix[p-1][q-1] * kernel[j-p][k-q]);
                }
            }
        }
    }
    return result;
}

/**
 * Function that apples Sobel filter to detect vertical borders
 * image: Image type struct with the image to be filtered
 * return: Image type struct with the image to be filtered
*/
Image sobel_filter(Image image) {
    // vertical border kernels
    int kernel[3][3] = {
        {-1, -2, -1},
        { 0,  0,  0},
        { 1,  2,  1}
    };

    // Convolution of kernel with Sobel
    int** conv2Result = conv2(image.rows, image.cols, image.data, 3, 3, kernel);
    
    // Creation of struct with new image
    Image filtered;
    filtered.rows = image.rows-2;
    filtered.cols = image.cols-2;
    filtered.data = createMatrix(image.rows-2, image.cols-2);

    // Goes through image to copy the values of the central pixels
    for (int i = 1; i < image.rows-1; i++) {
        for (int j = 1; j < image.cols-1; j++) {
            int value = conv2Result[i][j];

            // Truncates values
            if(value < 0) value = 0;
            if(value > 255) value = 255;
            filtered.data[i-1][j-1] = value;
        }
    }

    free(conv2Result);
    return filtered;
}

/**
 * Writes an image to a .png file
 * image: Image type struct with info of image to write
*/
void writeImage(char* filename, Image image) {
	FILE *fp = NULL;
	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	png_bytep row = NULL;
	
	// Opens file to write
	fp = fopen(filename, "wb");
	if (fp == NULL) {
        printf("\033[1;31m");
		fprintf(stderr, "Error during creation of file %s to write\n", filename);
        printf("\033[0m");
		exit(1);
	}

	// Initialization of struct for writing
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
        printf("\033[1;31m");
		fprintf(stderr, "Could not allocate write struct\n");
        printf("\033[0m");
		exit(1);
	}

	// Initialize info structure
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
        printf("\033[1;31m");
		fprintf(stderr, "Could not allocate info struct\n");
        printf("\033[0m");
		exit(1);
	}

	// Setup Exception handling
	if (setjmp(png_jmpbuf(png_ptr))) {
        printf("\033[1;31m");
		fprintf(stderr, "Error during png creation\n");
        printf("\033[0m");
		exit(1);
	}

    // initialize input/output for the PNG file
	png_init_io(png_ptr, fp);

	// Write header (8 bit colour depth)
	png_set_IHDR(png_ptr, info_ptr, image.rows, image.cols,
			8, PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	png_write_info(png_ptr, info_ptr);

	// Allocate memory for one row (1 byte per pixel - RGB)
	row = (png_bytep) malloc(image.rows * sizeof(png_byte));

	// Write image data
	for (int y = 0 ; y < image.cols; y++) {
		for (int x = 0; x < image.rows; x++) {
			png_byte* pixel = &(row[x]);
            *pixel = image.data[x][y];
		}
		png_write_row(png_ptr, row);
	}

	// End write
	png_write_end(png_ptr, NULL);

    // Frees and clean memory
	fclose(fp);
	png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
	png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
	free(row);
}