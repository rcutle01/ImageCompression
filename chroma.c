/*
 *      chroma.c
 *      by: Becky Cutler (rcutle01) and Julia Rowe (jrowe01)
 *      February 20, 2015
 *      Assignment 4
 *
 *      Depending on the compression, it either averages the chroma values and
 *      returns an index,or takes and index and returns the 4 chroma values
 */


#include <stdio.h>
#include <stdlib.h>
#include "arith40.h"

#define blocksize 2

/* Takes in 4 floats and returns the average */
float avgChroma(float a, float b, float c, float d)
{
        float sum = a + b + c + d;
        return sum / 4;
}

/* Takes the average chroma value (Pb or Pr) and returns the quantized
 * representation of the given value
 */
unsigned getIndex(float x)
{
       return Arith40_index_of_chroma(x); 
}

/* Takes in the 4-bit chroma index and returns the average Pb or Pr value */
float getChroma(unsigned x)
{
        return Arith40_chroma_of_index(x);
}
