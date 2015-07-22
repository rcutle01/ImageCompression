/*
 *      luminance.c
 *      by: Becky Cutler (rcutle01) and Julia Rowe (jrowe01)
 *      February 20, 2015
 *      Assignment 4
 *
 *      Depending on the compression, it either transforms the Y values
 *      into cosine coefficients or computes the Y values from a, b, c and d
 */

#include <stdio.h>
#include <stdlib.h>
#include "luminance.h"

/* Takes four Y values and preforms a discrete cosine transform to obtain four 
 * cosine coefficients. These coefficients are stored in a struct and that
 * struct is returned.
 */
struct cosin_vals getCosine(float Y1, float Y2, float Y3, float Y4)
{
        struct cosin_vals vals;
        vals.a = (Y4 + Y3 + Y2 + Y1) / 4.0;
        vals.b = (Y4 + Y3 - Y2 - Y1) / 4.0;
        vals.c = (Y4 - Y3 + Y2 - Y1) / 4.0;
        vals.d = (Y4 - Y3 - Y2 + Y1) / 4.0;

        return vals;
}

/* Takes four coefficients and uses the inverse of the discrete cosine transform
 * to compute the four Y values, which are stored and returned as a struct. 
 */
struct Y_vals getYvalues(float a, float b, float c, float d)
{
        struct Y_vals vals;
        vals.Y1 = a - b - c + d;
        vals.Y2 = a - b + c - d;
        vals.Y3 = a + b - c - d;
        vals.Y4 = a + b + c + d;

        return vals;
}
