#include <pthread.h>
#include <ifaddrs.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

/**
 * Method used to get the extension of a file and verify its either png or jpg or jpeg
 * @param filename The direction to the file
 * @return 1 for a png, 2 for a jpeg or jpg and -1 if neither
 */

int get_file_extension(char *filename) {

    int filenameSize = strlen(filename);
    char fileeExtension[] = {filename[filenameSize - 3], filename[filenameSize - 2], filename[filenameSize - 1], '\0'};
    char pngExtension[] = "PNGpng";
    char jpgExtension[] = "JPGjpgJPEGjpeg"; 
    if (strstr(pngExtension, fileeExtension) != NULL) {
        return 1;
    }
    if (strstr(jpgExtension, fileeExtension) != NULL) {
        return 2;
    } else {
        return -1;
    }
}

int check_file_existence(char *filePath) {
    FILE *fp;
    if ((fp = fopen(filePath, "rb")) == NULL) {
        return -1;
    } else {
        fclose(fp);
        return 1;
    }
}