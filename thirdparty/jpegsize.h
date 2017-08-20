#pragma once
#include <stdio.h>
// http://carnage-melon.tom7.org/stuff/jpegsize.html

int scanhead(FILE *infile, int *image_width, int *image_height);
