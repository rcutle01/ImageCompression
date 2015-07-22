/*
 *      Chroma.h
 *      by: Becky Cutler (rcutle01) and Julia Rowe (jrowe01)
 *      February 20, 2015
 *      Assignment 4
 *
 *      Headerfile for chroma value conversions  
 */


#include <stdio.h>
#include <stdlib.h>

/* Takes in 4 ints, and returns the average */
float avgChroma(float a, float b, float c, float d);

/* Takes the average chroma value (Pb or Pr) and returns the quantized
 * representation of the given value
 */
unsigned getIndex(float x);

/* Takes in the 4-bit chroma index and returns the average Pb or Pr value */
float getChroma(unsigned x);
