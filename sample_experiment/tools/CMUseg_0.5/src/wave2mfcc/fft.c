
#include <stdio.h>
#include <math.h>
#include "complex.h"
/*
 * Given N a power of 2, complex array in[0..N-1], compute its DFT in
 * complex array out[0..N-1], using the FFT algorithm.
 * 
 * Let <s,k> denote the sequence { f(s,k;z) | z = w^0, w^k, ... , w^(N-k) },
 * where f(s,k;z) = in[s]*z^0 + in[s+k]*z^k + ... + in[s+N-k]*z^(N-k), k is a
 * power of 2, k <= N, and w = exp(-2*PI*i/N).  The DFT of in is <0,1>.  Then
 *   <s,k> =  <s,2k> + {w^0,w^k,...,w^(N/2-k)} * <s+k,2k>
 *         || <s,2k> - {w^0,w^k,...,w^(N/2-k)} * <s+k,2k>,
 * where || denotes concatenation, and +,-,* denote term-by-term operations
 * on sequences.  The strategy is to compute, in the following order,
 *    <s,N>, s = 0, 1, ..., N, copied from in;
 *    <s,N/2>, s = 0, 1, ..., N/2-1;
 *     ...
 *    <s,1>, s = 0, left in out.
 * We store <s,k> in locations s, s+k, ..., s+N-k of from and to, and we
 * store w^k in w[k], k = 0, 1, ..., N/2-1.
 *
 * HISTORY
 *
 * 3-Nov-97 Matthew A. Siegler (msiegler) at CMU
 *      memory leak caused by calloc on the static pointer.       
 *
 * 17-Feb-87  Andy Gruss (gruss) at Carnegie-Mellon University
 *	Instead of dividing by N when FFT is computed, renormalization
 *	is now done when the inverse FFT is done.
 *
 * 03-Jul-85  Ellen Walker (elw) at Carnegie-Mellon University
 *	Changed to use complex data type from math library.
 *	The manual entry is complex(3m).
 *
 * 11-Jan-82  Bruce Lucas (bdl) at Carnegie-Mellon University
 *	Created.  Timed at 60-70 N log2 N microseconds on a VAX 11/780.
 *
 */
fft(in, out, N, invert)
complex in[], out[];		/* complex input, result		*/
int N;				/* must be a power of 2			*/
int invert;			/* 1 for FFT, -1 for inverse FFT	*/
{
  static int
    s, k,			/* as above				*/
    lgN;			/* log2(N)				*/
  register complex
    *f1, *f2,			/* pointers into from array		*/
    *t1, *t2,			/* pointers into to array		*/
    *ww;			/* pointer into w array			*/
  static complex
    *w, *from, *to,		/* as above				*/
    wwf2,			/* temporary for ww*f2			*/
    *buffer,			/* from and to flipflop btw out and buffer */
    *exch,			/* temporary for exchanging from and to	*/
    *wEnd;			/* to keep ww from going off end	*/
  static double
    div,			/* amount to divide result by: N or 1	*/
    x;				/* misc.				*/

  /* check N, compute lgN						*/
  for (k = N, lgN = 0; k > 1; k /= 2, lgN++)
  {
    if (k%2 != 0 || N < 0)
    {
      fprintf(stderr, "fft: N must be a power of 2 (is %d)\n", N);
      return(-1);
    }
  }

  /* check invert, compute div						*/
  if (invert == 1)
    div = 1.0;
  else if (invert == -1)
    div = N;
  else
  {
    fprintf(stderr, "fft: invert must be either +1 or -1 (is %d)\n", invert);
    return(-1);
  }

  /* get the to, from buffers right, and init				*/
  buffer = (complex *)calloc(N, sizeof(complex));
  if (lgN%2 == 0)
  {
    from = out;
    to = buffer;
  }
  else
  {
    to = out;
    from = buffer;
  }
  for (s = 0; s<N; s++)
  {
    from[s].r = in[s].r/div;
    from[s].i = in[s].i/div;
  }

  /* w = exp(-2*PI*i/N), w[k] = w^k					*/
  w = (complex *) calloc(N/2, sizeof(complex));
  for (k = 0; k < N/2; k++)
  {
    x = -6.28318530717958647*invert*k/N;
    w[k].r = cos(x);
    w[k].i = sin(x);
  }
  wEnd = &w[N/2];

  /* go for it!								*/
  for (k = N/2; k > 0; k /= 2)
  {
    for (s = 0; s < k; s++)
    {
      /* initialize pointers						*/
      f1 = &from[s]; f2 = &from[s+k];
      t1 = &to[s]; t2 = &to[s+N/2];
      ww = &w[0];
      /* compute <s,k>							*/
      while (ww < wEnd)
      {
        /* wwf2 = ww*f2							*/
        wwf2.r = f2->r*ww->r - f2->i*ww->i;
        wwf2.i = f2->r*ww->i + f2->i*ww->r;
        /* t1 = f1+wwf2							*/
        t1->r = f1->r + wwf2.r;
        t1->i = f1->i + wwf2.i;
        /* t2 = f1-wwf2							*/
        t2->r = f1->r - wwf2.r;
        t2->i = f1->i - wwf2.i;
        /* increment							*/
        f1 += 2*k; f2 += 2*k;
        t1 += k; t2 += k;
        ww += k;
      }
    }
    exch = from; from = to; to = exch;
  }

  free(w);
  free(buffer);
  return(0);
}

