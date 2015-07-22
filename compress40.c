/* 
 *      compress40.c
 *      by: Becky Culter, Julia Rowe
 *      February 27, 2015
 * 
 *      Calls the correct compress or decompress function depending on the user 
 *      input. Prints the final structure once it has been compressed or
 *       decompress.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "assert.h"
#include "uarray2.h"
#include "uarray2b.h"
#include "pnm.h"
#include "a2methods.h"
#include "a2blocked.h"
#include "compress40.h"
#include "ConvertRgbCv.h"
#include "chroma.h"
#include "luminance.h"
#include "bitpack.h"
#include "math.h"
#include "inttypes.h"

#define word_size 2 * sizeof(int)
#define block_size 2
#define denom 255 

#define w_a 6
#define w_bcd 6
#define w_index 4

#define lsb_a 26
#define lsb_b 20 
#define lsb_c 14
#define lsb_d 8 
#define lsb_pb 4 
#define lsb_pr 0

#define maxf 0.3
#define minf -0.3
#define maxs 31
#define mins -31

/*closure that we is used by the pack apply function. It contains the 
 * compressed image array and an array of length 4 to hold the 4 pixels upon
 * which we will operate to compress a 2x2 block into one 32-bit word.
 */
typedef struct closure{
        UArray2_T compress;
        Pnm_cv CvArray;
        int length;
} *closure;

/* helper functions */
void print(UArray2_T compressed); 
int float_to_signed(float val);
float signed_to_float(signed val);

/* Apply function for compression. We are mapping over a decompressed image, 
 * stored as a  UArray2b. The closure contains the compressed UArray2 of words,
 * an array of length 4 to handle the block to one word compression,and a length
 * of this array.  We get the component video values out of the element and find
 * the average Pb and Pr values and the cosine coefficients.  These values are 
 * then packed into a 32 bit word, which is added to the compressed image.
 */
void pack(int i, int j, A2Methods_UArray2 array, void *elem, void *cl)
{
        (void)array;

        closure cls = cl;
        Pnm_cv CV = elem;
        Pnm_cv cvarray = cls->CvArray;
        int length = cls->length;
        cls->CvArray[length] = *CV;
        
        float Pr_avg, Pb_avg;
        struct cosin_vals coef;
        unsigned a;
        int b, c, d;
        uint64_t word = 0;

        /* Creating the array of 4 pixels to perform the DCT and average the Pb
         * and Pr values. Getting the values and packing the word
         */
        if (length == 3) {
                Pb_avg = avgChroma(cvarray[0].Pb, cvarray[1].Pb, 
                                   cvarray[2].Pb, cvarray[3].Pb);
                Pr_avg = avgChroma(cvarray[0].Pr, cvarray[1].Pr, 
                                   cvarray[2].Pr, cvarray[3].Pr); 
                coef = getCosine(cvarray[0].Y, cvarray[2].Y, 
                                 cvarray[1].Y, cvarray[3].Y);
                
                a = round(63*coef.a);
                b = float_to_signed(coef.b);
                c = float_to_signed(coef.c);
                d = float_to_signed(coef.d);

                word = Bitpack_newu(word, w_a, lsb_a, a);
                word = Bitpack_news(word, w_bcd, lsb_b, b);
                word = Bitpack_news(word, w_bcd, lsb_c, c);
                word = Bitpack_news(word, w_bcd, lsb_d, d);
                word = Bitpack_newu(word, w_index, lsb_pb, getIndex(Pb_avg));
                word = Bitpack_newu(word, w_index, lsb_pr, getIndex(Pr_avg));
                
                unsigned *temp = UArray2_at(cls->compress, i/2, j/2);
                *temp = word;
                cls->length = 0;
        } else { /* too few elements so we add it to the array */
               cls->length++;
        }

        (*(closure)cl) = *cls; /* update the closure */

}

/* Apply function for decompression. We are mapping over a compressed image, 
 * stored as a UArray2, to a decompressed image stored as a UArray2b (stored in
 * the closure).Since we are going from one word to four pixels we use a counter
 * to place the correct Y value from the inverse discrete cosine tranformation
 * in the corresponding index in the block array.
 */
void unpack(int i, int j, UArray2_T array, void *elem, void *cl)
{
        (void)array;
        
        UArray2b_T decompress = cl;
        int counter = 0;
     
        uint64_t *word = elem;
        unsigned a = Bitpack_getu(*word, w_a, lsb_a);
        int b = Bitpack_gets(*word, w_bcd, lsb_b);
        int c = Bitpack_gets(*word, w_bcd, lsb_c);
        int d = Bitpack_gets(*word, w_bcd, lsb_d);
        unsigned Pb_index = Bitpack_getu(*word, w_index, lsb_pb);
        unsigned Pr_index = Bitpack_getu(*word, w_index, lsb_pr);

        /* inserve DCT */
        struct Y_vals y_vals = getYvalues((float)a / 63, 
                                           signed_to_float(b),
                                           signed_to_float(c), 
                                           signed_to_float(d));

        struct Pnm_cv new;
        new.Pb = getChroma(Pb_index);
        new.Pr = getChroma(Pr_index);

        for (int l = 0; l < 2; l++) {
                for (int k = 0; k < 2; k++) {
                        /* changing the Y value for each item in a block */
                        counter++;
                        if (counter == 1) new.Y = y_vals.Y1;
                        if (counter == 2) new.Y = y_vals.Y2;
                        if (counter == 3) new.Y = y_vals.Y3;
                        if (counter == 4) new.Y = y_vals.Y4;
                        Pnm_cv temp = UArray2b_at(decompress, 2 * i + k, 
                                                  2 * j + l);
                        *temp = new;
                }
        }
        cl = decompress;
}

/* Takes an empty UArray2 to store the compressed image file.  Builds words from
 * the input file (passed in the closure) one character at a time using four
 * characters for one word. Then adds these words to the array.
 */
void getword(int i, int j, UArray2_T array, void *elem, void *cl)
{
        (void)elem;
        uint64_t word = 0;
        FILE *fp = cl;
        for(int k = 3; k >= 0; k--) {
                word = Bitpack_newu(word, 8, k * 8, getc(fp));
        }
        uint64_t *temp = UArray2_at(array, i, j);
        *temp = word;
}

/* Creates the data structures to hold the compressed and decompressed images.
 * Maps over these structures to apply the necessary transformations.  Prints
 * the final compressed image and frees all allocated memory.
 */
extern void compress40(FILE *input)
{
        Pnm_ppm decompress; /* to hold the decompressed image */
        A2Methods_T methods = uarray2_methods_blocked;
        assert(methods);
        decompress = Pnm_ppmread(input, methods);

        /* sets pixels to an array of component video structs, obtained from 
         * converting the RGB values to component video values */
        decompress->pixels = convert(decompress->pixels, 'c', 
                                     decompress->denominator);

        /* to hold compressed images as an array of words */
        UArray2_T compress = UArray2_new((decompress->width)/2, 
                                         (decompress->height)/2, word_size);

        /* creating the necessary closure for the pack function */
        closure cl = malloc(sizeof(*cl));
        Pnm_cv CvArray = malloc(4 * (sizeof(*CvArray)));
        cl->compress = compress;
        cl->CvArray = CvArray;
        cl->length = 0;

        /* mapping over the array to create and pack words into the compressed
         * image 
         */
        methods->map_default(decompress->pixels, pack, cl);       
        print(compress);

        Pnm_ppmfree(&decompress);
        UArray2_free(&compress);
        free(CvArray);
        free(cl);
        return;
}

/* Creates the data structures to hold the compressed and decompressed images.
 * Maps over these structures to apply the necessary transformations.  Prints
 * the final decompressed image and frees all allocated memory.
 */
extern void decompress40(FILE *input)
{
        unsigned height, width;
        A2Methods_T methods = uarray2_methods_blocked;

        int read = fscanf(input, "COMP40 Compressed image format 2\n%u %u",
                          &width, &height);
        assert(read == 2);
        int c = getc(input);
        assert(c == '\n'); 

        /* to hold the compressed image */
        UArray2_T  compress = UArray2_new(width / 2, height / 2, word_size);

        /* to hold the decompressed image */
        UArray2b_T pixels = UArray2b_new(width, height, 
                                         sizeof(struct Pnm_rgb), 2);

        /* initializing the array to hold the 320bit words */
        UArray2_map_row_major(compress, getword, input);

        /* mapping over the array to unpack words into 4 pixels each*/
        UArray2_map_row_major(compress, unpack, pixels);

        /* converting from CV to RGB colorspace */
        pixels = convert(pixels, 'd', denom);
        
        struct Pnm_ppm decompress = { .width = width, .height = height,
                                      .denominator = denom, .pixels = pixels, 
                                      .methods = methods
        };
        Pnm_ppmwrite(stdout, &decompress);

        UArray2b_free(&pixels);
        UArray2_free(&compress);
        return;   
}

/* Apply function for printing the compressed image */
void print_compressed(int i, int j, UArray2_T array, void* elem, void* cl) 
{
        (void) i;
        (void) j;
        (void) array;
        (void) cl;
        
        uint64_t *word = elem;

        for (int i = 3; i >= 0; i--) {
               char character = Bitpack_getu(*word, 8, i * 8);
               putchar(character);
        }
}

/* Function to print the compressed image, by mapping over the array */
void print(UArray2_T compressed)
{

        int width = 2 * UArray2_width(compressed);
        int height = 2 * UArray2_height(compressed);
        printf("COMP40 Compressed image format 2\n%u %u\n", width,
               height);
        UArray2_map_row_major(compressed, print_compressed, NULL);
}

/* Conversion from float point values to signed integer values. Takes a
 * floating point value within a set range, and converts it to a signed value in
 * a different set range.
 */

int float_to_signed(float val)
{
        if (val > maxf) val = maxf;
        if (val < minf) val = minf;

        int scale_factor = maxs / maxf;
        int sval = round(val * scale_factor);
       
        return sval;
}

/* Conversion from signed integer values to floating point values. Takes a
 * signed value within a set range, and converts it to a floating point value in
 * a different set range.
 */
float signed_to_float(signed val)
{
        if (val > maxs) val = maxf;
        if (val < mins) val = minf;
 
        float scale_factor = maxf / maxs;
        return val * scale_factor;
}
