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


 9 Aug 1996
 Modified by msiegler to accept globals for the constants


 */

#include <stdio.h>
#include <stdlib.h>
#include "const.h"
#include "ham.h"
#include "mel_filter.h"

static HAMMING_WINDOW hw_struct, *hw = &hw_struct;

/*
moved to mel_filter.h
static MEL_FILTER     *mel_filters = NULL;
*/
static float *wbuf_r = NULL, *wbuf_i = NULL, *wbuf = NULL;
static float *fft_r = NULL, *fft_i = NULL;
static float *cos_table = NULL, *sin_table = NULL;

static float sample_spacing;
static int32 fft_size;

static float **rot;
extern float **create_rotation_vectors();

float cep_scale;
float cep_scale_factor();

void melcep_init()
{
  /* FFT related */
  fft_size      = 2 * DFT_POINTS;
  
  wbuf_r   = (float *)calloc(fft_size, sizeof(float));
  wbuf_i   = (float *)calloc(fft_size, sizeof(float));
  wbuf     = (float *)calloc(2*fft_size, sizeof(float));
  
  fft_r    = (float *)calloc(DFT_POINTS, sizeof(float));
  fft_i    = (float *)calloc(DFT_POINTS, sizeof(float));
  
  calculate_tables(&cos_table, &sin_table, fft_size);
  calculate_hamming_window(hw, LEN_WINDOW);
  
  mel_filters = (MEL_FILTER *) calloc(NO_OF_FILTERS,sizeof(MEL_FILTER));
  calculate_mel_filters(&mel_filters[0],
			SAMPLE_RATE,DFT_POINTS,UPPER_EDGE_OF_FILTERS,LOG_SPACING,NO_OF_FILTERS,
			NUMBER_OF_LOG_TRIANGLES,LOWER_EDGE_OF_FILTERS,LINEAR_SPACING);
  
  sample_spacing = (float)SAMPLE_RATE / fft_size;
  rot = create_rotation_vectors(NO_OF_FILTERS, CEP_VECLEN);
  cep_scale = cep_scale_factor(NO_OF_FILTERS);
  FRAME_SPACING = (int)(SAMPLE_RATE / FRAME_RATE);
}



/*
 *
 */
void
melcep_frame(cep0, cep1, samp)
     float *cep0;
     float *cep1;
     short *samp;
{
  static short prior = 0;

  float *spec0,*spec1;

  spec0 = (float *)calloc(sizeof(float),NO_OF_FILTERS);
  spec1 = (float *)calloc(sizeof(float),NO_OF_FILTERS);

  
  window_and_alias(wbuf_r, hw, fft_size, samp, prior,PREEMPH_ALPHA);
  window_and_alias(wbuf_i, hw, fft_size, &samp[FRAME_SPACING], samp[FRAME_SPACING-1],PREEMPH_ALPHA);
  
  prior = samp[FRAME_SPACING * 2 - 1];
  
  calculate_FFT(wbuf_r, fft_size, cos_table, sin_table, wbuf_i);
  combine(fft_r, fft_i, wbuf_r, wbuf_i, fft_size);
  
  pass_2_thru_mel_filters(fft_r, fft_i, mel_filters,
			  spec0, spec1, NO_OF_FILTERS,
			  sample_spacing);
  
  rotate_vector(spec0, NO_OF_FILTERS, rot, cep0, CEP_VECLEN);
  rotate_vector(spec1, NO_OF_FILTERS, rot, cep1, CEP_VECLEN);
  
  scale_parm(cep0, CEP_VECLEN, cep_scale);
  scale_parm(cep1, CEP_VECLEN, cep_scale);

  free(spec0);
  free(spec1);
}




/*


 22 Aug 1996 
 Matthew A. Siegler

 Just returns logspectra.  Forgets about cepstra alltogether
 (like melcep_frame)


*/

/*
 *
 */
void
melspec_frame(spec0, spec1, samp)
     float *spec0;
     float *spec1;
     short *samp;
{
  static short prior = 0;

  
  window_and_alias(wbuf_r, hw, fft_size, samp, prior,PREEMPH_ALPHA);
  window_and_alias(wbuf_i, hw, fft_size, &samp[FRAME_SPACING], samp[FRAME_SPACING-1],PREEMPH_ALPHA);
  
  prior = samp[FRAME_SPACING * 2 - 1];
  
  calculate_FFT(wbuf_r, fft_size, cos_table, sin_table, wbuf_i);
  combine(fft_r, fft_i, wbuf_r, wbuf_i, fft_size);
  
  pass_2_thru_mel_filters(fft_r, fft_i, mel_filters,
			  spec0, spec1, NO_OF_FILTERS,
			  sample_spacing);
}
