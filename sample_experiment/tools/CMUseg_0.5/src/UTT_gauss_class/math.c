/*

Mixture Gaussian Classifier

Given a set of distribution files, P(x|w_i),
a set of priors P(w_i),

For a sequence of n i.i.d. observarions, X = {x1,x2,...,xn}
Compute:

argmax(i) P(w_i|x) = 

argmax(i) P(x|w_i)*P(w_i)/P(x) = 

argmax(i) P(x|w_i)*P(w_i)




Mon May 20 18:31:57 EDT 1996
msiegler

Core routines:


likelihood:
Compute P(x|w_i) where w_i is a mixture gaussian, diagonal covariance matrix.

*/

#include <math.h>
#include "UTT_gauss_class.h"

/* return log probability log(Pr(X|w_i))  where X = {x1,x2,...,xn}
   return 0 if there was some problem */


float likelihood_fast (class_t *c,float *data,int data_dim)
{


  float sum=0.0,like;
  int k,l;

#ifdef DEBUG
  extern int Gctr;
  Gctr+=(c->num_mix*data_dim);
#endif

  for (k=0;k<c->num_mix;k++)
    {
      like = 1.0;
      for (l=0;l<data_dim;l++)
	  like *= c->hack[k][l] * (float)exp((double)((data[l] - c->means[k][l]) * (data[l] - c->means[k][l]) / (c->vars[k][l] * -2)));
      like *= c->weights[k];

      sum += like;
    }
  return((float)log((double)sum));
}










