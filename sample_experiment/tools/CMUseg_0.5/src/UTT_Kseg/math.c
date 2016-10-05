#include "UTT_Kseg.h"

#define MIN(x,y) (x<y)?x:y
#define MAX(x,y) (x>y)?x:y

int find_segp(suitcase_t *S)
{
  ERR_reset("Finding segmentation points");
  if(Kdist_fast(S))
    ERR_return("Trouble computing K values");
  if(K_peaks(S))
    ERR_return("Trouble computing Peaks of K values");
  return(0);
}


int Kdist_fast(suitcase_t *S)
/* does all the sliding, uses accum smart! 
   center point is included in the "b" window
*/
{

  int d,fr;
  float k,kt,*xsum_a,*xsum_b,*x2sum_a,*x2sum_b;
  float xl,xc,xr;
  float ma1,mb1,va1,vb1;
  int xleft,xcenter,xright;

#define point(f,d) (((f)<0||(f)>=S->data_size)?0.0:S->data[(f)*S->data_dim+(d)])
/* smart pointer to data, returns zero if out of bounds */

  xsum_a = (float *)calloc(S->data_dim,sizeof(float));
  xsum_b = (float *)calloc(S->data_dim,sizeof(float));
  x2sum_a = (float *)calloc(S->data_dim,sizeof(float));
  x2sum_b = (float *)calloc(S->data_dim,sizeof(float));

  for (d=0;d<S->data_dim;d++)
    xsum_a[d] = xsum_b[d] = x2sum_a[d] = x2sum_b[d] = k = 0.0;

  /* go outside data */
  for (fr = - (S->win_len); fr < S->data_size + S->win_len;fr++)
    {
      xleft = fr - S->win_len - 2;
      xcenter = fr - 1;
      xright = fr + S->win_len - 1;

      k = 0.0;
      for (d=0;d<S->data_dim;d++)
	{
	  xl = point(xleft,d);
	  xc = point(xcenter,d);
	  xr = point(xright,d);

	  xsum_a[d] -= xl;
	  xsum_a[d] += xc;
	  xsum_b[d] -= xc;
	  xsum_b[d] += xr;

	  x2sum_a[d] -= xl*xl;
	  x2sum_a[d] += xc*xc;
	  x2sum_b[d] -= xc*xc;
	  x2sum_b[d] += xr*xr;
	  
	  ma1 = xsum_a[d] / S->win_len;
	  mb1 = xsum_b[d] / S->win_len;
	  va1 = x2sum_a[d] / S->win_len - ma1*ma1;
	  vb1 = x2sum_b[d] / S->win_len - mb1*mb1;

	  va1 = MAX(S->var_floor,va1);
	  vb1 = MAX(S->var_floor,vb1);

	  if (va1 > 0 && vb1 > 0)
	    k += MAX(0.25 * (va1/vb1 + vb1/va1 + (ma1-mb1)*(ma1-mb1)*(1/va1 + 1/vb1) - 2.0),0);
	}


      if (fr >= S->win_len && fr < S->data_size-S->win_len)
	{
	  k = k/(float)S->data_dim;
	  S->Kin[fr] = k;
	}
      else
	/* fixed 15 July 1997, msiegler, to keep from running off end */
	if (fr >=0 && fr < S->data_size)
	  S->Kin[fr] = 0;

      if (S->verbose && !(fr % 6000))
	fprintf(stderr,".");


    }

  if (S->verbose)
    fprintf(stderr,"\n");


  free(xsum_a);
  free(xsum_b);
  free(x2sum_a);
  free(x2sum_b);

  return(0);
}

int K_peaks(suitcase_t *S)
/*
   Do peak-picking for segment boundaries
*/
{
  int i,m;
  int fail;
  float curr;
  float *window=NULL;
  int mend,mstart;
  int nreal;

/* make bigger cause the convolve does some hacky things! */
  nreal = 1;
  while (nreal < S->smooth_len)
    nreal <<= 1;

/* make a 2^n with 0 pad window */
  window = (float *)calloc(nreal * 2,sizeof(float));
  makewindow(window,S->smooth_len);
  for (i=S->smooth_len;i<nreal;i++)
    window[i] = 0.0;

/* smoothing */
  if (S->verbose)
    fprintf(stderr,"smoothing with zero padded %d to %d\n",S->smooth_len,nreal);
  convolve(S->Kin,window,S->data_size,S->smooth_len,S->Ksmooth);


  /*  peakpicking */
  for (i=S->win_len*2;i<S->data_size-S->win_len*2;i++)
    {
      fail = 0;

      /*
	curr = S->Kin[i];
	
	Bug discovered by Hairuo Ma (thanks!)
	Tue Jul  7 18:22:06 EDT 1998
	*/
      
      curr = S->Ksmooth[i];

      mstart = MAX(0,i-S->scan_len);
      mend = MIN(S->data_size,i+S->scan_len);
      for (m=mstart;m<mend;m++)
	if (m!=i && S->Ksmooth[m]>=curr)
	  fail = 1;
      
      /* 
	 if (m!=i && S->Kin[m]>=curr)
	 fail = 1;
	 
	 Bug discovered by Hairuo Ma (thanks!)
	 Tue Jul  7 18:22:06 EDT 1998
	 */
      
      if (!fail && curr > S->Kthresh)
	S->frame_segp[S->num_segp++] = i + S->frame_start;
    }

  Free(window);
  return(0);
}

int makewindow(float *w,int n)
{
  int i;
  for (i=0;i<n;i++)
    w[i] = 0.54 - 0.46 * cos(M_PI * 2 * i / (n-1));
  return(0);
}


int convolve(float *x,float *w,int len,int win_len,float *y)
{
  /* y = x conv w */
  int i,m;
  float *o;
  o = (float *)calloc(len+win_len,sizeof(float));
  for (i=win_len/2;i<len+win_len/2;i++)
    o[i] = x[i-win_len/2];
  /* copy data, pad on each side */

  for (i=0;i<len;i++)
    {
      y[i] = 0;
      for (m=0;m<win_len;m++)
        y[i] += w[m] * o[i+m];
      y[i] /= (win_len / 2);
    }
     
  free(o);
  return(0);
}

