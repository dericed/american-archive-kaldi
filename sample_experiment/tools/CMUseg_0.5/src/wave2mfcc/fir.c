/*

29 Oct 1997 msiegler@cs.cmu.edu
Matthew A. Siegler

FIR front end filter

*/

#include "wave2mfcc.h"

#define firMAXlen 65536
/* longest filter permitted -- kinda long */


int fir_init(suitcase_t *S)
     /* Initialize the memory for overlap-and-add, 
	need enough for the overflow.  save hlen-1 points  */
{
  int i;
  FIRcase_t *FIR;
  FIR = S->FIR;

  ERR_reset("fir_init");

  if ( (FIR->ymem = (float32 *)malloc((FIR->hlen - 1) * sizeof(float32))) == NULL)
    ERR_return("Malloc error");

  /* clear out buffers etc.. */
  FIR->x = FIR->X = FIR->h = FIR->H = NULL;

  for (i=0;i<FIR->hlen -1;i++)
    FIR->ymem[i] = 0.0;

  return (0);
}


/* 

   Smart about computing H only when the buffer size changes (or first time)
   x,h and X,H are given equal sized DFTS (duh) so that the multiply can happen

 */


int fir_int16(suitcase_t *S,int16 *xin,int xlen)
{
  int i,size_change=0;
  complex xtmp;
  FIRcase_t *FIR;
  FIR = S->FIR;

  ERR_reset("fir_int16");

  /* first step, make sure we have some memory (blocks can get bigger all of a sudden) */
  if (FIR->x == NULL)
    {
      /* make the fftlen a power of two that is larger than the convloved data */
      FIR->fftlen = 1;
      while (FIR->fftlen < xlen + FIR->hlen - 1)
	FIR->fftlen <<= 1;
      
      if ( (FIR->x = (complex *)malloc(FIR->fftlen * sizeof(complex))) == NULL || 
	   (FIR->X = (complex *)malloc(FIR->fftlen * sizeof(complex))) == NULL ||
	   (FIR->h = (complex *)malloc(FIR->fftlen * sizeof(complex))) == NULL ||
	   (FIR->H = (complex *)malloc(FIR->fftlen * sizeof(complex))) == NULL)
	ERR_return("Malloc error");

      size_change=1;
    }
  
  if (FIR->fftlen < xlen + FIR->hlen - 1)
    {
      while (FIR->fftlen < xlen + FIR->hlen - 1)
	FIR->fftlen <<= 1;

      if ( (FIR->x = (complex *)realloc(FIR->x,FIR->fftlen * sizeof(complex))) == NULL || 
	   (FIR->X = (complex *)realloc(FIR->X,FIR->fftlen * sizeof(complex))) == NULL ||
	   (FIR->h = (complex *)realloc(FIR->h,FIR->fftlen * sizeof(complex))) == NULL ||
	   (FIR->H = (complex *)realloc(FIR->H,FIR->fftlen * sizeof(complex))) == NULL)
	ERR_return("Realloc error");
      
      size_change=1;
    }

  if (S->verbose && size_change)
    fprintf(stderr,"change: fftlen=%d xlen=%d\n",FIR->fftlen,xlen);

  /* real->complex pad with zeros */
  for (i=0;i<FIR->fftlen;i++)
    {
      FIR->x[i].r = (i<xlen)?(float32)xin[i]:0.0;
      FIR->x[i].i = 0.0;
    }
  fft(FIR->x,FIR->X,FIR->fftlen,1);
  

  /* do this only when we have to */
  if (size_change)
    {
      for (i=0;i<FIR->fftlen;i++)
	{
	  FIR->h[i].r = (i<FIR->hlen)?FIR->hin[i]:0.0;
	  FIR->h[i].i = 0.0;
	}
      fft(FIR->h,FIR->H,FIR->fftlen,1);
    }

  /* perform the multiply (inplace) */
  for (i=0;i<FIR->fftlen;i++)
    {
      xtmp = c_mul(FIR->X[i],FIR->H[i]);
      FIR->X[i] = xtmp;
    }

  /* invert fft in place */
  fft(FIR->X,FIR->x,FIR->fftlen,-1);

  /* here is the add part of the overlap and add (also convert back into int16) */

  for (i=0;i<xlen;i++)
    if (i<FIR->hlen-1)
      xin[i] = (int16)rint(FIR->x[i].r + FIR->ymem[i]);
    else
      xin[i] = (int16)rint(FIR->x[i].r);

  /* save for the next overlap */
  for (i=0;i<FIR->hlen-1;i++)
    FIR->ymem[i] = FIR->x[i+xlen].r;

  return(0);
}

int fir_readfilter(suitcase_t *S)
     /* read in a filter file, returns 0 if succesfull.
	sets count to the number of entries, mallocs the filter
	binary float32 for the filter, NATIVE byteorder!
	*/

{
  FILE *fp;
  int i;
  float32 val;

  FIRcase_t *FIR;
  FIR = S->FIR;

  ERR_reset("fir_readfilter");
  fp = fopen(FIR->fn,"r");
  if (fp == NULL)
    ERR_return("Couldn't open the FIR filter");

  FIR->hlen=0;
  while (!feof(fp) && fread(&val,sizeof(float32),1,fp))
    (FIR->hlen)++;

  if (FIR->hlen > firMAXlen)
    {
      fclose(fp);
      ERR_return("FIR filter is too long");
    }
  else if (FIR->hlen)
    {
      rewind(fp);
      if( (FIR->hin = (float32 *)malloc(FIR->hlen * sizeof(float32)) ) == NULL)
	ERR_return("Malloc error");
      fread(FIR->hin,sizeof(float32),FIR->hlen,fp);
    }
  
  fclose(fp);
  return(0);
}


int destruct_fir(suitcase_t *S)
  {
    FIRcase_t *FIR;
    FIR = S->FIR;

    free(FIR->x);
    free(FIR->X);
    free(FIR->h);
    free(FIR->H);
    free(FIR->ymem);
    free(FIR->hin);
    free(FIR->fn);    
    return(0);
  }
