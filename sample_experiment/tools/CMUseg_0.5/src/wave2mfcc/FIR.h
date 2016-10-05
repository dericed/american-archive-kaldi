#include "complex.h"

typedef struct FIRcase_s
{
  /* FIR arguments */
  char *fn;
  float32 *hin;
  int32 hlen;
  
  complex *h;
  complex *H;
  complex *x;
  complex *X;

  float32 *ymem;
  int32 fftlen;
  
} FIRcase_t;
