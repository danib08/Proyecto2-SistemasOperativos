#include <stdio.h>
#include <stdlib.h>


/**
 * Function to create a matrix, assigning a space of memory
 * return: matrix pointer
 */

int** createMatrix(int rows, int cols) {
    int len = sizeof(int *) * rows + sizeof(int) * cols * rows; 
    int **matrix = (int **)malloc(len); 
    int *ptr = (int *)(matrix + rows);

    //Travel matrix and correct pointer to right direction
    for(int i = 0; i < rows; i++) 
        matrix[i] = (ptr + cols * i); 

    // Initialize matrix with zeros
    for(int i = 0; i < rows; i++)
        for(int j = 0; j < cols; j++)
            matrix[i][j] = 0;
    
    return matrix;
}