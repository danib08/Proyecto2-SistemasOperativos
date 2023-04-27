#include <stdio.h>
#include <stdlib.h>


int** createMatrix(int rows, int cols) {
    int len = sizeof(int *) * rows + sizeof(int) * cols * rows; 
    int **matrix = (int **)malloc(len); 
    int *ptr = (int *)(matrix + rows);

    for(int i = 0; i < rows; i++) 
        matrix[i] = (ptr + cols * i); 

    for(int i = 0; i < rows; i++)
        for(int j = 0; j < cols; j++)
            matrix[i][j] = 0;
    
    return matrix;
}