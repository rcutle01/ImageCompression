/*
 *      ConvertRgbCv.c
 *      by: Becky Cutler (rcutle01) and Julia Rowe (jrowe01)
 *      February 20, 2015
 *      Assignment 4
 *
 *     Converts elements from RGB to CV or the other way around 
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "ConvertRgbCv.h"
#include "pnm.h"
#include "uarray2b.h"

#define blocksize 2

struct clsr {
        UArray2b_T newPix;
        unsigned denom;
};

/* Converting from RGB to Component Video (CV) colorspace.  Creates a struct to
 * hold the CV values, calculates to values, and stores the struct in the new
 * array
 */
void RGBtoCV(int i, int j, UArray2b_T array, void *elem, void *cl) 
{
        (void) array; 
        
        Pnm_rgb RGB = elem;
        struct Pnm_cv CV;

        struct clsr *clsr = cl;  
        unsigned denom = clsr->denom;
        UArray2b_T CVpix = clsr->newPix;

        /* since we do not change the width and height of the original array
         * which we are mapping over, we must check to ensure that we are
         * inbounds of the new array
         */
        if ((i >= UArray2b_width(CVpix) || 
            (j >= UArray2b_height(CVpix)))) return;

        CV.Y = (0.299 * (RGB->red) + 0.587 * (RGB->green) + 0.114 * (RGB->blue))
                / (double)denom ;
        CV.Pb = (-0.168736 * (RGB->red) - 0.331264 * (RGB->green) + 
                  0.5 * (RGB->blue)) / (double)denom;
        CV.Pr = (0.5 * (RGB->red) - 0.418688 * (RGB->green) - 
                 0.081312 * (RGB->blue)) / (double)denom;

        Pnm_cv temp = UArray2b_at(CVpix, i, j);
        *temp = CV;
}

/* Converting from CV to RGB colorspace.  Creates a struct to hold the RGB
 * values, calculates their values, and stores the struct in the new array.
 */
void CVtoRGB(int i, int j, UArray2b_T array, void *elem, void *cl) 
{
        (void) array;

        Pnm_cv CV = elem;
        struct Pnm_rgb RGB;

        struct clsr *clsr = cl;  
        unsigned denom = clsr->denom;
        UArray2b_T RGBpix = clsr->newPix;

        float r_coef = (1.0 * (CV->Y) + 0.0 * (CV->Pb) + 1.402 * (CV->Pr));
        float g_coef = (1.0 * (CV->Y) - 0.344136 * (CV->Pb) 
                        - 0.714136 * (CV->Pr));
        float b_coef = (1.0 * (CV->Y) + 1.772 * (CV->Pb) + 0.0 * (CV->Pr));

        /* handling values that would create colors that are out of bounds */
        if (r_coef > 1) r_coef = 1;
        if (r_coef < 0) r_coef = 0;        
        if (g_coef > 1) g_coef = 1;        
        if (g_coef < 0) g_coef = 0;        
        if (b_coef > 1) b_coef = 1;
        if (b_coef < 0) b_coef = 0;

        RGB.red = denom * r_coef;
        RGB.green =  denom * g_coef;
        RGB.blue = denom * b_coef;
        
        Pnm_rgb temp = UArray2b_at(RGBpix, i, j);
        *temp = RGB;
}

/* Function that calls the correct helper function for compression or 
 * decompression.  We create a new blocked array with blocksize of 2 and the
 * trimmed width and height to hold the new structs, map from the old to the new
 * and free the old.
 */
UArray2b_T convert(UArray2b_T pixels, char op, unsigned denom)
{
        int width = UArray2b_width(pixels);
        int height = UArray2b_height(pixels);
        
        UArray2b_T newPixels;
        /* trimming odd width or height if necessary */ 
        if (width % 2 != 0) width--;
        if (height % 2 != 0) height--;

        /* closure to be passed to the mapping functions: contains the new array
         * and the denominator 
         */
        struct clsr cl;
        cl.denom = denom;
                
        if (op == 'c') { 
                newPixels = UArray2b_new(width, height, sizeof(struct Pnm_cv),
                                         blocksize);
                cl.newPix = newPixels;
                UArray2b_map(pixels, RGBtoCV, &cl);
        } else {
                newPixels = UArray2b_new(width, height, sizeof(struct Pnm_rgb),
                                         blocksize);
                cl.newPix = newPixels;
                UArray2b_map(pixels, CVtoRGB, &cl);
        }
        
        UArray2b_free(&pixels);
        return newPixels;

}
