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


