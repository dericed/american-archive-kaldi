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
 * 	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created.
 */

#include <stdlib.h>
#include <math.h>
#include "const.h"


/* This routine sets up the sine and cosine tables for the FFT routine */

void calculate_tables(cos_table,sin_table,table_size)
float **cos_table, **sin_table;
int table_size;
{
    float angle;
    float increment;
    float *ct, *st;
    int   x;

    ct = (float *) calloc(table_size, sizeof(float));
    st = (float *) calloc(table_size, sizeof(float));

    increment = 2 * PI/table_size;

    for (x = 0, angle = 0.0; x < table_size; x++, angle += increment) {
	ct[x] = (float) cos((double) angle);
	st[x] = (float) sin((double) angle);
    }

    *cos_table = ct;
    *sin_table = st;
}
