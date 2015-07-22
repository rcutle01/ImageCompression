/*
 *      luminance.h
 *      by: Becky Cutler (rcutle01) and Julia Rowe (jrowe01)
 *      February 20, 2015
 *      Assignment 4
 *
 *      Header file for luminance conversions
 */

#ifndef LUMINANCE_INCLUDED
#define LUMINANCE_INCLUDED

#include <stdio.h>
#include <stdlib.h>

typedef struct cosin_vals {
        float a, b, c, d;
} *cosin_vals;

typedef struct Y_vals {
        float Y1, Y2, Y3, Y4;
} *Y_vals;

/* Takes four Y values and preforms a discrete cosine transform to obtain four 
 * cosine coefficients. These coefficients are stored in a struct and that
 * struct is returned.
 */
struct cosin_vals getCosine(float Y1, float Y2, float Y3, float Y4);


/* Takes four coefficients and uses the inverse of the discrete cosine transform
 * to compute the four Y values, which are stored and returned as a struct. 
 */
struct Y_vals getYvalues(float a, float b, float c, float d);

#endif
