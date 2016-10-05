/*
 * fe.c -- Front end: raw samples -> cep
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
 * 12-Aug-96    M A Siegler (msiegler@cs.cmu.edu)
 *              Major revision to system parameters.  Now it is less hard-coded. Far fewer copies of variables.
 * 
 * 21-May-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Substantially modified to be driven with externally provided data, rather
 * 		than explicitly reading an A/D source.
 * 		Removed logging functions (now done at a higher level, eg, by the caller).
 * 
 * 29-Nov-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Incorporated Steve Reed's changes.
 * 
 * 01-Apr-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created.

*/


#if (WIN32)
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include <err.h>


#include <mel_filter.h>
#include <fe.h>


#ifndef TRUE
/* For those infuriating HPs */
#define TRUE	1
#define FALSE	0
#endif


/*
 * Two frames of cepstra computed at a time from (LEN_WINDOW+FRAME_SPACING) samples.
 * But each call can provide arbitrary #samples to be processed.  Also, samples for
 * successive frames are overlapped.  Therefore, we need an internal buffer to manage
 * partial frame samples and overlap between successive calls.  This is adbuf.
 */
static int16 *adbuf;
static int32 adbuf_data;	/* #Valid samples in adbuf to be prefixed to next call.
				   Updated at end of each call */

#define BPS	(sizeof(int16))

int32 fe_initdefaults (void)
{
  SAMPLE_RATE = FILTER_DEFAULT;
  DFT_POINTS = FILTER_DEFAULT;
  LOG2_DFT_POINTS = FILTER_DEFAULT;
  FRAME_RATE = FILTER_DEFAULT;
  FRAME_SPACING = FILTER_DEFAULT;
  LEN_WINDOW = FILTER_DEFAULT;
  NO_OF_FILTERS = FILTER_DEFAULT;
  NUMBER_OF_LOG_TRIANGLES = FILTER_DEFAULT;
  CEP_VECLEN = FILTER_DEFAULT;
  LOG_SPACING = FILTER_DEFAULT;
  LINEAR_SPACING = FILTER_DEFAULT;
  LOWER_EDGE_OF_FILTERS = FILTER_DEFAULT;
  UPPER_EDGE_OF_FILTERS = FILTER_DEFAULT;
  HAMMING_WINDOW_LENGTH = FILTER_DEFAULT;
  PREEMPH_ALPHA = FILTER_DEFAULT;
}

int32 fe_defaults (void)
{

  if (DFT_POINTS == FILTER_DEFAULT)
    DFT_POINTS = __DFT_POINTS;
  
  if (LOG2_DFT_POINTS == FILTER_DEFAULT)
    LOG2_DFT_POINTS = __LOG2_DFT_POINTS;
      
  if (SAMPLE_RATE == FILTER_DEFAULT)
    SAMPLE_RATE = __SAMPLE_RATE;
  
  if (FRAME_RATE == FILTER_DEFAULT)
    FRAME_RATE = __FRAME_RATE;
  
  if (FRAME_SPACING == FILTER_DEFAULT)
    FRAME_SPACING = __FRAME_SPACING;
  
  if (HAMMING_WINDOW_LENGTH == FILTER_DEFAULT)
    HAMMING_WINDOW_LENGTH = __HAMMING_WINDOW_LENGTH;
  
  if (LEN_WINDOW == FILTER_DEFAULT)
    LEN_WINDOW = __LEN_WINDOW;
  
  if (NO_OF_FILTERS == FILTER_DEFAULT)
    NO_OF_FILTERS = __NO_OF_FILTERS;

  if (NUMBER_OF_LOG_TRIANGLES == FILTER_DEFAULT)
    NUMBER_OF_LOG_TRIANGLES = __NUMBER_OF_LOG_TRIANGLES;
  
  if (CEP_VECLEN == FILTER_DEFAULT)
    CEP_VECLEN = __CEP_VECLEN;
  
  if (LOG_SPACING == FILTER_DEFAULT)
    LOG_SPACING = __LOG_SPACING;
  
  if (LINEAR_SPACING == FILTER_DEFAULT)
    LINEAR_SPACING = __LINEAR_SPACING;
  
  if (LOWER_EDGE_OF_FILTERS == FILTER_DEFAULT)
    LOWER_EDGE_OF_FILTERS = __LOWER_EDGE_OF_FILTERS;
  
  if (UPPER_EDGE_OF_FILTERS == FILTER_DEFAULT)
    UPPER_EDGE_OF_FILTERS = __UPPER_EDGE_OF_FILTERS;
  
  if (PREEMPH_ALPHA == FILTER_DEFAULT)
    PREEMPH_ALPHA = __PREEMPH_ALPHA;

  return(0);
}

/* One-time initialization for entire run */
int32 fe_init ( void )
{
  int32 sz;

  /* fix bad parameters */
  fe_defaults();
  
  for (sz = (FRAME_SPACING<<1); sz < (LEN_WINDOW-FRAME_SPACING); sz += (FRAME_SPACING<<1));
  sz += LEN_WINDOW+FRAME_SPACING;
    
  if ((adbuf = (int16 *) calloc (sz, BPS)) == NULL) {
    fprintf(stderr,"calloc(%d,%d) failed\n", sz, BPS);
    return -1;
  }
  adbuf_data = 0;
  melcep_init();

  return 0;
}


/* Called once at start of each utterance */
int32 fe_start ( void )
{
    adbuf_data = 0;
    return 0;
}


/*
 * Compute cepstra for the given input speech samples.  Also use previously saved sampls
 * in adbuf, if any.  At the end, save new overlap with next call in adbuf.
 * Caller must provide cep buffer long enough to hold all output cep vectors.
 * Return value: #cep frames computed, possibly 0.  -ve if error.
 */
int32 fe_raw2cep (int16 *raw, int32 rawlen, float **cep)
{
  int32 orig_data;
  int32 roff, soff;	/* offsets into receiving and sending buffers */
  int32 i, k, c;
  
  if (adbuf_data >= LEN_WINDOW + FRAME_SPACING)
    {
      fprintf(stderr,"2-frames of data previously pending; fe_start not called?\n");
      return -1;
    }
  
  c = 0;
    
  /* First process frames overlapped with previously processed frames, if any */
  orig_data = adbuf_data;
  roff = 0;
  soff = 0;
  for (roff = 0; roff < orig_data; roff += (FRAME_SPACING<<1))
    {
      k = LEN_WINDOW+FRAME_SPACING - (adbuf_data-roff);	/* #samples needed to compute 2-frames of cep */

      if (k > rawlen-soff)
	{
	  /* Not enough samples for 2-frames of cep; save overlap and return */
	  k = rawlen - soff;
	  
	  if (roff > 0)
	    {
	      for (i = roff; i < adbuf_data; i++)
		adbuf[i-roff] = adbuf[i];
	      adbuf_data -= roff;
	    }
      
	  if (k > 0)
	    {
	      memcpy (adbuf+adbuf_data, raw+soff, k*BPS);
	      adbuf_data += k;
	    }
	  
	  assert (adbuf_data < LEN_WINDOW+FRAME_SPACING);
      
	  return c;
	}
      else 
	{
	  memcpy (adbuf+adbuf_data, raw+soff, k*BPS);
	  adbuf_data += k;
	  soff += k;
	  
	  /* Compute 2-frames of cep */
	  melcep_frame (cep[c], cep[c+1], adbuf+roff);
	  c += 2;
	}
    }
  
  /* Process data in raw buf */
  for (soff = roff - orig_data; rawlen-soff >= LEN_WINDOW+FRAME_SPACING; soff += (FRAME_SPACING<<1))
    {
      melcep_frame (cep[c], cep[c+1], raw+soff);
      c += 2;
    }
  
  /* Save overlap with next block */
  k = rawlen - soff;
  if (k > 0)
    {
      memcpy (adbuf, raw+soff, k*BPS);
      adbuf_data = k;
    }
  else
    adbuf_data = 0;
  assert (adbuf_data < LEN_WINDOW+FRAME_SPACING);
  
  return c;
}


/* Called once at end of each utterance */
int32 fe_stop ( void )
{
    adbuf_data = 0;
    return 0;
}



/* just get log spec, like raw2cep */

int32 fe_raw2spec (int16 *raw, int32 rawlen, float **spec)
{
  int32 orig_data;
  int32 roff, soff;	/* offsets into receiving and sending buffers */
  int32 i, k, c;
  
  if (adbuf_data >= LEN_WINDOW + FRAME_SPACING)
    {
      fprintf(stderr,"2-frames of data previously pending; fe_start not called?\n");
      return -1;
    }
  
  c = 0;
    
  /* First process frames overlapped with previously processed frames, if any */
  orig_data = adbuf_data;
  roff = 0;
  soff = 0;
  for (roff = 0; roff < orig_data; roff += (FRAME_SPACING<<1))
    {
      k = LEN_WINDOW+FRAME_SPACING - (adbuf_data-roff);	/* #samples needed to compute 2-frames of spec */

      if (k > rawlen-soff)
	{
	  /* Not enough samples for 2-frames of spec; save overlap and return */
	  k = rawlen - soff;
	  
	  if (roff > 0)
	    {
	      for (i = roff; i < adbuf_data; i++)
		adbuf[i-roff] = adbuf[i];
	      adbuf_data -= roff;
	    }
      
	  if (k > 0)
	    {
	      memcpy (adbuf+adbuf_data, raw+soff, k*BPS);
	      adbuf_data += k;
	    }
	  
	  assert (adbuf_data < LEN_WINDOW+FRAME_SPACING);
      
	  return c;
	}
      else 
	{
	  memcpy (adbuf+adbuf_data, raw+soff, k*BPS);
	  adbuf_data += k;
	  soff += k;
	  
	  /* Compute 2-frames of spec */
	  melspec_frame (spec[c], spec[c+1], adbuf+roff);
	  c += 2;
	}
    }
  
  /* Process data in raw buf */
  for (soff = roff - orig_data; rawlen-soff >= LEN_WINDOW+FRAME_SPACING; soff += (FRAME_SPACING<<1))
    {
      melspec_frame (spec[c], spec[c+1], raw+soff);
      c += 2;
    }
  
  /* Save overlap with next block */
  k = rawlen - soff;
  if (k > 0)
    {
      memcpy (adbuf, raw+soff, k*BPS);
      adbuf_data = k;
    }
  else
    adbuf_data = 0;
  assert (adbuf_data < LEN_WINDOW+FRAME_SPACING);
  
  return c;
}

