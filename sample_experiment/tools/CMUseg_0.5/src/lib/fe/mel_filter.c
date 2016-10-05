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

 Modified 
 Mon Aug 12 1996
 Matthew A. Siegler
 Using global variables in mel_filter.h
 

 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mel_filter.h"


/* This routine calculates the slope of a line for the mel-filters.  The first 
   frequency is passed as f1, the second frequency is passed as center_freq,
   the total width of the ENTIRE mel-filter (right_edge - left_edge) is passed
   as the width, and the slope of the specific side with f1 as the starting frequency
   and center_freq as the ending frequency */

void calculate_slope_of_line (f1,center_freq,width,slope)
float f1,center_freq,width;
float *slope;
{
    float height;

    height = 2 / width; /* For normalized area */
    *slope = height/(center_freq-f1); /* slope is rise/run */
}

/* This routine actually sets up the mel-filters.  The resulting filters are passed
   back in the structure mel_filters. */

void calculate_mel_filters(mel_filters,sampling_rate,dft_points,up_edge,
			   log_spacing,no_filters,number_log_triangles,
			   low_edge,linear_spacing)
MEL_FILTER *mel_filters;
int dft_points,number_log_triangles,no_filters;
float up_edge,low_edge,log_spacing,linear_spacing,sampling_rate;
{
    float sample_pts,max_length,height,left_edge,center_freq,right_edge,slope1;
    float y_intercept1,slope2,y_intercept2,width,freq;
    int max_no_pts,no_of_linear_filters,x,i,start_pt,z;

    sample_pts = sampling_rate /(float) (2*dft_points); /* This is the spacing between FFT points (in frequency) */
    max_length = (float) up_edge - ((float)up_edge/ (float)(2*log_spacing)); 
    max_no_pts = (int) (max_length/sample_pts) + 1;/* Max length of a filter */
    no_of_linear_filters = no_filters - number_log_triangles;
    center_freq = low_edge;
    right_edge = low_edge + linear_spacing;
    /* This sets up for the loop below.  center shifts to left, and right shifts to
       center for the first run through the loop */
    for (x = 0; x < no_filters; x++) {
	mel_filters[x].filter_pts = (float *) calloc (max_no_pts,sizeof(float));
	if (mel_filters[x].filter_pts != NULL) {
	    left_edge = center_freq; /* the new left edge = old center freq */
	    center_freq = right_edge; /* the new center freq = old right edge */
	    /* 
	     * modified by Aki, to handle the special case in which linear spacing
	     * is desired for all the channels.
	     */
	    /*
	      if (x < (no_of_linear_filters-1))
	      */
	    if (x<(no_of_linear_filters-1)||(number_log_triangles==0))
		right_edge = center_freq + linear_spacing; /* new right edge is shifted out */
	    else
		right_edge = center_freq * log_spacing; /* new right edge for log scale */
	    mel_filters[x].left_edge = left_edge;
	    mel_filters[x].center_freq = center_freq;
	    mel_filters[x].right_edge = right_edge;
	    width = right_edge - left_edge;
	    calculate_slope_of_line(left_edge,center_freq,width,&slope1);
	    calculate_slope_of_line(right_edge,center_freq,width,&slope2);
	    start_pt = left_edge/(float) sample_pts;
	    /* using the slopes calculated above, determine the exact values of the mel-
	       filters at the specific locations which the FFT will fall */
	    for (freq=(float)start_pt*sample_pts,i=0;freq<=right_edge;freq += sample_pts) {
		if ((freq>=left_edge) && (freq<=center_freq))
		    mel_filters[x].filter_pts[i++] = slope1*(freq-left_edge);
		else
		    if ((freq>center_freq) && (freq<=right_edge))
			mel_filters[x].filter_pts[i++] = slope2*(freq-right_edge);
	    }
	    mel_filters[x].filter_size = i;
	}
	else
	    fprintf(stderr,
		    "Error couldn't allocate enough memory for mel filter #%d\n",
		    x);
    }
}

void pass_1_thru_mel_filters(fft_coeffs_real, fft_coeffs_imag,
			     mel_filters, mfs_coeffs,
			     no_filters,
			     sample_spacing)
MEL_FILTER *mel_filters;
float *fft_coeffs_real,*fft_coeffs_imag;
float *mfs_coeffs,sample_spacing;
int no_filters;
{
    int x,i,start_pt,filter_size;
    float sum,freq,current_pt;

    for (x = 0; x < no_filters; x++) {
	start_pt = mel_filters[x].left_edge / sample_spacing;

	if ((sample_spacing * (float) start_pt) < mel_filters[x].left_edge)
	    start_pt++;

	filter_size = mel_filters[x].filter_size;

	for (sum = 0.0, i = 0; i < filter_size; i++, start_pt++)
	    sum += (mel_filters[x].filter_pts[i]) *
		((fft_coeffs_real[start_pt] * fft_coeffs_real[start_pt])
		 +  (fft_coeffs_imag[start_pt] * fft_coeffs_imag[start_pt]));

	if (sum <= 0)
	    sum = 1.0;

	mfs_coeffs[x] = (float) 10*log10(sum);
    }
}


void pass_2_thru_mel_filters(fft_coeffs1, fft_coeffs2,
			     mel_filters,
			     mfs_coeffs1, mfs_coeffs2,
			     no_filters, sample_spacing)
MEL_FILTER *mel_filters;
float *fft_coeffs1,*fft_coeffs2;
float *mfs_coeffs1,*mfs_coeffs2,sample_spacing;
int no_filters;
{
    int x, i, start_pt, filter_size;
    float sum1, sum2, freq, current_pt;

    for (x = 0; x < no_filters; x++) {
	start_pt = mel_filters[x].left_edge / sample_spacing;
	if ((sample_spacing * (float) start_pt) < mel_filters[x].left_edge)
	    start_pt++;
	filter_size = mel_filters[x].filter_size;
	for (sum1=0.0, sum2=0.0, i=0; i < filter_size; i++, start_pt++) {
	    sum1 += (mel_filters[x].filter_pts[i]) * fft_coeffs1[start_pt];
	    sum2 += (mel_filters[x].filter_pts[i]) * fft_coeffs2[start_pt];
	}
	if (sum1 <= 0)
	    sum1 = 1.0;
	mfs_coeffs1[x] = (float) 10 * log10(sum1) - LOG_FACTOR;
	
	if (sum2 <= 0)
	    sum2 = 1.0;
	mfs_coeffs2[x] = (float) 10 * log10(sum2) - LOG_FACTOR;
    }
}
