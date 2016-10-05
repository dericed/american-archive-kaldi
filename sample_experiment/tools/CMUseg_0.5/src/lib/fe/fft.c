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


#include "log2.h"

/* This routine unshuffles the FFT coefficients after they have been calculated */
void reverse_digits(real_part,imag_part,sample_size)
float *real_part,*imag_part;
int sample_size;
{
    int i,j,n1,k,index,half_sample_size;
    float xt;

    index = 1;
    n1 = sample_size-1;
    half_sample_size = sample_size/2;
    for (i=0;i<n1;i++) {
	j = index -1;
	if (i<index) {
	    xt = real_part[j];
	    real_part[j] = real_part[i];
	    real_part[i] = xt;
	    xt = imag_part[j];
	    imag_part[j] = imag_part[i];
	    imag_part[i] = xt;
	}
	k = half_sample_size;
	while (k<index) {
	    index -= k;
	    k /=2;
	}
	index += k;
    }
}

void calculate_FFT(sample_stream,sample_size,
		   cos_table,sin_table,temp_imaginary)
float *sample_stream,*cos_table,*sin_table,*temp_imaginary;
int sample_size;
{
    int i,j,k,n1,n2,l,ie,ia,no_levels,one_less;
    float temp,ty,c,s,n_level;

    n2 = sample_size;
    no_levels = (int) log2((double) sample_size);
    one_less = no_levels-1;
    for (k=0;k<no_levels;k++) {
	n1 = n2;
	n2 /=2;
	for (i=0;i<sample_size;i += n1) {
	    l = i+n2;
	    temp = sample_stream[i] - sample_stream[l];
	    sample_stream[i] += sample_stream[l];
	    sample_stream[l] = temp;
	    temp = temp_imaginary[i]-temp_imaginary[l];
	    temp_imaginary[i] += temp_imaginary[l];
	    temp_imaginary[l] = temp;
	}
	if (k != one_less) {
	    ie = sample_size/n1;
	    for (ia=0,j=1;j<n2;j++) {
		ia += ie;
		c = cos_table[ia];
		s = sin_table[ia];
		for (i=j;i<sample_size;i+=n1) {
		    l = i+n2;
		    temp = sample_stream[i] - sample_stream[l];
		    sample_stream[i] += sample_stream[l];
		    ty = temp_imaginary[i]-temp_imaginary[l];
		    temp_imaginary[i] += temp_imaginary[l];
		    sample_stream[l] = c*temp + s*ty;
		    temp_imaginary[l] = c*ty - s*temp;
		}
	    }
	}
    }
    reverse_digits(sample_stream,temp_imaginary,sample_size);
}
