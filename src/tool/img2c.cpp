#define STB_IMAGE_IMPLEMENTATION
#include "../stb_image.h"
#include <stdio.h>
#include <string.h>

int width, height, nrChannels, size;

unsigned char *loadImage(char *path) {
    unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data) {
        size = width * height * nrChannels;
        return data;
    } else {
        fprintf(stderr, "error: cannot load image file: %s\n", path);
        width = height = nrChannels = size = 0;
        stbi_image_free(data);
    }
    return NULL;
}

int main ( int argc, char** argv ) {
    int i = 0;            
    char ch = '\0';        

    if ( 2 != argc ) {
        printf( "\n    Usage: img2c <file> \n\n" );
        return -1;
    }

    unsigned char * data = loadImage(argv[1]);
    if (!data) {
        return 1;
    }

    // get file name for array's name
    while ( '.' != argv[1][i++] );
    argv[1][ i-1 ] = 0;

    //get file data and change to const unsigned char array's data
    i = 1;
    printf( "const unsigned char %s[] = { \n\t", argv[1] );

    for (size_t i = 0; i < size; i++) {
        printf( "0x%02X,%s\t", data[i], ( (i + 1) % 8 ) == 0 ? "\n" : "" );
    }

    stbi_image_free(data);

    printf( "\n};\n" );

    printf( "#define LEN_%s %d\n", argv[1], size );

    printf( "#define WIDTH_%s %d\n", argv[1], width );

    printf( "#define HEIGHT_%s %d\n", argv[1], height );

    printf( "#define CHANNEL_%s %d\n", argv[1], nrChannels );

    return 0;
}
