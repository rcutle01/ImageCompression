/*
 *      ConvertRgbCv.h
 *      by: Becky Cutler (rcutle01) and Julia Rowe (jrowe01)
 *      February 20, 2015
 *      Assignment 4
 *      
 *      Headerfile for ConvertRgbCv
 */

#ifndef CONVERTRGBCV_INCLUDED
#define CONVERTRGBCV_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include "uarray2.h"
#include "uarray2b.h"

/* struct to hold component video pixels */
typedef struct Pnm_cv {
        float Y, Pb, Pr;
} *Pnm_cv;
        
/*main implementation function that calls helper function depending on the
 * compression 
 */
UArray2b_T convert(UArray2b_T pixels, char op, unsigned denom);

#endif
