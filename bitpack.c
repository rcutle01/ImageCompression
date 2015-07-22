/* Implementation of compress40.h
 * written by: Becky Cutler (rcutle01) and Julia Rowe (jrowe01)
 * 2/20/15
 * Calls the correct compress or decompress function depending on the user 
 * input. Prints the final structure once it has been compressed or
 * decompress.
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <except.h>
#include <assert.h>
#include "bitpack.h"

#define num_of_bits 64

Except_T Bitpack_Overflow = { "Overflow packing bits" };


/* tells whether the arguments n can be represented in width bits*/
bool Bitpack_fitsu(uint64_t n, unsigned width)
{
        if (width >= 64) return true;
        uint64_t temp = 1 << width;
        if (n < temp) return true; /* using right bit shifting to make 
                                    * sure n
                                    * fits in the width */
        else return false;
}

bool Bitpack_fitss(int64_t n, unsigned width)
{
        if (width >= 64) return true;

        /* testing to see if the value can fit in the given width */
        if ((n < ((1 << width) /2) || n <= -1 * ((1 << width)/2))) return true;

        else return false;
}

/* gets unsigned values from a word (which is always unsigned) */
uint64_t Bitpack_getu(uint64_t word, unsigned width, unsigned lsb)
{
        assert((width <= 64) && ((width + lsb) <= 64));

        uint64_t mask = ~0;
        /* shifting the mask to get the correct position */
        mask = mask >> (num_of_bits - width) << lsb;

        return (word & mask) >> lsb;
}

/* gets signed values from the word */
int64_t Bitpack_gets(uint64_t word, unsigned width, unsigned lsb)
{
        assert((width <= 64) && ((width + lsb) <= 64));

        int64_t mask = ~0;
        mask = mask >> (num_of_bits - width) << lsb;
        int64_t val = (word & mask);

        /* left shift to get leading term to propogate*/
        val = val << (num_of_bits - (lsb + width)); 

        return val >> (num_of_bits - width); 
}

/* creating a new word using the provided value, width, and lsb */
uint64_t Bitpack_newu(uint64_t word, unsigned width, unsigned lsb, 
                      uint64_t value)
{
        assert((width <= 64) && ((width + lsb) <= 64));
        /* making sure the value fits in the space */
        if (!Bitpack_fitsu(value, width)) RAISE(Bitpack_Overflow);

        uint64_t mask = ~0;
        /* shifting the mask to get the correct position */
        mask = mask >> (num_of_bits - width) << lsb;
        mask = ~mask;

        word = (word & mask);
        return (word | (value << lsb));
}

/* creaing a new word using the provided value, width and lsb */
uint64_t Bitpack_news(uint64_t word, unsigned width, unsigned lsb, 
                      int64_t value)
{
        assert((width <= 64) && ((width + lsb) <= 64));
        /* making sure the value fits in the space */
        if (!Bitpack_fitss(value, width)) RAISE(Bitpack_Overflow);

        uint64_t val = value;
        uint64_t mask = ~0;
        /* shifting the mask to get the correct position */
        mask = mask >> (num_of_bits - (width));
        mask = (mask & val);
        mask = mask << lsb;

        return (word | mask); 
}
