/*

   All the includes necessary for the program go here
   Prototypes and datastructures and defines also

*/


#ifndef _h_wave2mfcc
#define _h_wave2mfcc

/* library code */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <malloc.h>

/* library code for mfc */
#include <FE.h>

/* local code */
#include "file.h"
#include "FIR.h"

/* da suitcase */

typedef struct suitcase_t
{
  int32 argc;
  char **argv;
  
  int32 verbose;
  int32 dither;
  int32 sphinx;
  int32 dcremove;
  int32 logspec;
  int32 showfilt;

  int32 addnoise;
  float addnoiseSNR;

  int32 babble;
  float babbleSNR;
  char *fn_babble;
  
  
  int32 resample;
  char *fn_resampleFIR;
  int32 resample_up;
  int32 resample_down;

  /* fe structure */
  parmfile_t *F;

  /* FIR structure */
  FIRcase_t *FIR;

  
} suitcase_t;



int32 main(int argc,char *argv[]);
int32 parse_cmdline(suitcase_t *S);
int32 tell_all(suitcase_t *S);
int32 convert_setup(suitcase_t *S,parmfile_t *F);
int32 convert_files(suitcase_t *S);

#endif
