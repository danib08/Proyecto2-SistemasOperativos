/**
 * Image struct containing data, rows and columns of the sent image
*/
typedef struct Image {
    int** data;
    int rows, cols;
} Image;