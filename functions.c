#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

/**
 * Function to concatenate strings
 * return: concatanated string
*/ 
char* concat(const char *s1, const char *s2) {
    const size_t len1 = strlen(s1); // Lenght of first string
    const size_t len2 = strlen(s2); // Lenght of second string
    char *result = malloc(len1 + len2 + 1); // null terminator 
    memcpy(result, s1, len1); //loads it to memory
    memcpy(result + len1, s2, len2 + 1);
    return result;
}

/**
 * Function to convert int to string
 * return: string representing number
*/
char* int2str(int number) {
    int n = snprintf(NULL, 0, "%d", number);
    char* sNumber = malloc(n+1);
    snprintf(sNumber, n+1, "%d", number);
    return sNumber;
}