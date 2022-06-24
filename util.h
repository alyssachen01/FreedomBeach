//#include <GL/glew.h>
#include <glad/glad.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

void *file_contents(const char *filename, GLint *length);
void *read_tga(const char *filename, int *width, int *height);