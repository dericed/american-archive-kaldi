/*
 * 
 * 
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1996 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * 
 */

#include <math.h>

float cep_scale_factor(nchan)
int nchan;
{
    float ln_to_dB;
    float f;

    ln_to_dB = 1.e1 / log((double)1.e1); /* 4.342944819 */
    f = ((float) 1 / (float) nchan)/ln_to_dB;

    return f;
}

/* the following function will scale MFCC coefficients by MIT-LCS
 * down so that they are compatible--not necessarily correct by
 * definition--to SPHINX front-end.
 *    1) MFCC's are devided by the number of channels, which ought
 *       to take care of what MIT-LCS left off.
 *    2) Scale everything down from dB to natural logarithm to
 *       meet what SPHINX can handle.
 */
void   scale_parm(inout, ncep, factor)
float   *inout;
int     ncep;
float	factor;
{
    int i;

    for(i = 0; i < ncep; i++)
	inout[i] = inout[i] * factor;
}
