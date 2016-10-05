
#include <math.h>
#include "complex.h"
/* Complex number manipulation: add, subtract, multiply, raise to	*/
/*   powers, etc.							*/
/*
 **********************************************************************
 * HISTORY
 * 
 * 28-Feb-89  Sarah Gibson at CMU
 *      Changed c_wgnoise() to use random number generators rand()
 *      and srand() instead of random() and srandom() so that it runs on VMS
 *
 * 19-Feb-87  Andy Gruss (gruss) at Carnegie-Mellon University
 *	Added c_wgnoise().
 *
 * 15-Feb-87  Andy Gruss (gruss) at Carnegie-Mellon University
 *	Use formula for c_div() instead of calling c_inv() and c_mul().
 *	Use a seperate <complex.h> instead of <math.h>.
 *
 * 06-Jan-84  Jon Webb (webb) at Carnegie-Mellon University
 *	Created.
 *
 **********************************************************************
 */

/* Form a complex number from real/imaginary parts			*/
complex c_cartc(r, i)
double r, i;
{
  complex c;
  c.r = r;
  c.i = i;
  return(c);
}


/* Standard uniary operations on complex numbers			*/
complex c_neg(x)
complex x;
{
  complex c;
  c.r = -x.r;
  c.i = -x.i;
  return (c);
}

complex c_conj(x)
complex x;
{
  complex c;
  c.r = x.r;
  c.i = -x.i;
  return (c);
}

complex c_inv(x)		/* Returns 1/x				*/
complex x;
{
  complex c;
  double d = x.r*x.r + x.i*x.i;
  c.r = x.r/d;
  c.i = -x.i/d;
  return(c);
}

/* Standard binary operations on complex numbers			*/
complex c_add(x1, x2)
complex x1, x2;
{
  complex c;
  c.r = x1.r + x2.r;
  c.i = x1.i + x2.i;
  return (c);
}

complex c_sub(x1, x2)
complex x1, x2;
{
  complex c;
  c.r = x1.r - x2.r;
  c.i = x1.i - x2.i;
  return (c);
}

complex c_mul(x1, x2)
complex x1, x2;
{
  complex c;
  c.r = x1.r*x2.r - x1.i*x2.i;
  c.i = x1.r*x2.i + x2.r*x1.i;
  return (c);
}

complex c_div(x1, x2)
complex x1, x2;
{
  complex c;
  double d = x2.r*x2.r + x2.i*x2.i;
  c.r = (x1.r*x2.r + x1.i*x2.i)/d;
  c.i = (x2.r*x1.i - x1.r*x2.i)/d;
  return (c);
}

/* Miscellaneous operations on complex numbers				*/
complex c_exp(p)		/* 'e' to the complex power p		*/
complex p;
{
  complex c;
  double r = exp(p.r);
  c.r = r*cos(p.i);
  c.i = r*sin(p.i);
  return(c);
}

complex c_log(x)		/* Natural logarithm of x		*/
complex x;
{
  complex c;
  c.r = log(c_mag(x));
  c.i = c_arg(x);
  return(c);
}

complex c_pow(x, p)		/* Raises x to the complex power p	*/
complex x, p;
{
  complex c, l;
  double r;
  l = c_mul(c_log(x), p);
  r = exp(l.r);
  c.r = r*cos(l.i);
  c.i = r*sin(l.i);
  return(c);
}

/* This is a special algorithm for real roots of complex numbers.  It	*/
/*   is needed because odd roots of negative reals should be negative	*/
/*   reals, at least in the formulae for cubics and quartics, but they	*/
/*   are complex numbers in the definition of principal roots.		*/
complex c_root(x, p)
complex x;
int p;
{
  complex c;
  if (x.r < 0.0 && x.i == 0.0 && p%2 == 1)
  {
    c.i = 0.0;
    c.r = -pow(-x.r, 1.0/(double)p);
    return(c);
  }
  else return(c_pow(x, c_realc(1.0/(double)p)));
}

/*
 * c_wgnoise() - each invocation generates a complex random number.
 *	The real and imaginary parts of the value returned are independent,
 *	zero-mean, and gaussian.  Successive invocations will generate a
 *	complex white process.
 *
 * Use a call to srand(int seed) to set the seed for the rand() calls, if
 *	desired.
 */
/* Defined in the standard C library					*/
extern int rand();
extern void srand();

complex c_wgnoise(sigma)
double sigma;
{
  /* Transforms two independent and [ 0, 1 ) uniformly distributed 	*/
  /*   random variables M, N into two independent, zero-mean, gaussian 	*/
  /*   random variables X, Y.  In polar coordinates, W is 2.0*PI*N	*/
  /*   (i.e. - [ 0, 2.0*PI ) uniform) and R is sigma*sqrt(-2.0*ln(M))	*/
  /*   (Rayleigh distribution).  It can be shown that R*cos(W) and	*/
  /*   R*sin(W) satisfy the requirements of X, Y above.			*/

  /* _HUGELONG is one more than the biggest integer a "long" can hold	*/
# define _HUGELONG	((double)(((unsigned long)1 << (sizeof(long)*8 - 1)) - 1) + 1.0)

  double R = sigma*sqrt(-2.0*log((double)rand()/_HUGELONG));
  double W = 2.0*M_PI*(double)rand()/_HUGELONG;

  return (c_polarc(R, W));
# undef	_HUGELONG
}

