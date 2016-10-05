/*

Fri Jun 27 1997 msiegler
Matthew A. Siegler

Simple signal processing for the data

includes:
 signal_dither
 singal_dcremove

*/

#include <time.h>
#include <stdio.h>

/* adds 1/2-bit noise */
int32 signal_dither(int16 *buffer,int32 szbuffer)
{
  int32 i;
  srand48((long)time(0));
  for (i=0;i<szbuffer;i++)
    buffer[i] += (short)((!(lrand48()%4))?1:0);
  
  return(0);
}

/*
signal removal (causual form) 
 dcremove(i) = sum(points to i)/num points
 unless i < some threshold -> do ramp up to avoid poor estimation at beginning
 essentially this is a hack of the MAP algorithm for mean removal.
 mean estimation variance = signal variance/npoints (assume gaussian signal)
 Worstcase: signal variance = 32767^2 = 2^30 
*/

/*
 maxvar  #points
 0       1
 -10db   10
 -20db   100
 -30db   1000
 -40db   10000
 
 Select (artificially 10000) for happiness
*/


#define DCREMOVE_RAMPLENGTH 10000
int32 signal_dcremove(int16 *buffer,int32 szbuffer,double *osum,int32 *osz)
{
  int32 i,sz;
  double sum;
  
  sum = *osum;
  sz = *osz;

  for (i=0;i<szbuffer;i++)
    {
      sz++;
      sum+=(double)buffer[i];

      
      if (sum != 0)
	{
	  if (sz < DCREMOVE_RAMPLENGTH)
	    buffer[i] -= (short)(sum  / DCREMOVE_RAMPLENGTH);
	  else
	    buffer[i] -= (short)(sum / sz);
	}
    }
    
  *osum = sum;
  *osz = sz;
  /*  fprintf(stderr,"sum = %f size = %d\n",*osum,*osz);
   */
  
  return(0);
}
