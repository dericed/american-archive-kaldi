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


combine(out_r, out_i, in_r, in_i, fft_size)
float *out_r;
float *out_i;
float *in_r;
float *in_i;
int fft_size;
{
    int j;
    float Xr1,Xr2,Xi1,Xi2;

    out_r[0] = 0.0;
    out_i[0] = 0.0;
    for (j = 1; j < fft_size/2; j++) {
	Xr1 = in_r[j];
	Xr2 = in_r[fft_size-j];
	Xi1 = in_i[j];
	Xi2 = in_i[fft_size-j];
	out_r[j] = (Xr1+Xr2)*(Xr1+Xr2)+(Xi1-Xi2)*(Xi1-Xi2);
	out_i[j] = (Xi1+Xi2)*(Xi1+Xi2)+(Xr1-Xr2)*(Xr1-Xr2);
    }
}
