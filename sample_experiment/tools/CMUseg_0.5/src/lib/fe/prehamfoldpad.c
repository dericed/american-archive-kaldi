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

 Modified
 Mon Aug 12 1996
 Matthew A. Siegler
 pre-emphasize factor is now an argument for window_and_alias out
 */

#include <stdlib.h>
#include "ham.h"
#include <math.h>


/* This sets up the haming window in the structure hamming_window.
   The window size is in points */

void calculate_hamming_window(hamming_window,window_size)
HAMMING_WINDOW *hamming_window;
int window_size;
{
    int x;
    float radians_per_sample;

    hamming_window->window_pts = (float *) calloc(window_size,sizeof(float));
    radians_per_sample = 2* PI/(float) window_size;
    for (x = 0; x < window_size; x++)
	hamming_window->window_pts[x] = (float)(0.54 - (0.46 * cos((double) x * radians_per_sample)));
    hamming_window->window_size = window_size;
}

void window_and_alias(out,
		      hamming_window, fft_size,
		      in,
		      prior,
		      alpha_factor
		      )
float *out;
HAMMING_WINDOW *hamming_window;
int fft_size;
short *in;
short prior;
float alpha_factor;
{
    int out_pos, i;
    int window_size;
    float t1, t2;
    
    window_size = hamming_window->window_size;

    t1 = prior;
    if (window_size > fft_size) {
	for (i = 0; i < fft_size; i++) {
	    t2 = in[i];
	    out[i] = ((float)t2 - alpha_factor * (float)t1) * hamming_window->window_pts[i];
	    t1 = t2;
	}
	for (out_pos = 0; i < window_size; i++, out_pos++) {
	    t2 = in[i];
	    out[out_pos] += ((float)t2 - alpha_factor * (float)t1) * hamming_window->window_pts[i];
	    t1 = t2;
	}
    }
    else {
	for (i = 0; i < window_size; i++) {
	    t2 = in[i];
	    out[i] = ((float)t2 - alpha_factor * (float)t1) * hamming_window->window_pts[i];
	    t1 = t2;
	}
	for (; i < fft_size; i++)
	    out[i] = 0.0;
    }
}
