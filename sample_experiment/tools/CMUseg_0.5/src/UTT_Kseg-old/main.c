/*

20 Sep 1996
Matthew A. Siegler
msiegler@cs.cmu.edu

Given a control file listing .mfc files, or parts of them, do auto segmenting
using KL2 distance - or "cross entropy"

Control file format:

 <source filename> <uttID> <beginF> <endF> [<breakP #1> [ <breakP #2> ] ... ] 

All numbers are in Frames relative to beginF, and
there needn't be any breakpoints designated at all.

Report file format:

 <source filename> <uttID> <endF> [<silence #1> [ <silence #2>] ... ]


 parse input
 open ctlfile
 foreach item
  read cepstra
  locate the silnces
  report
 close all

*/


#include "UTT_Kseg.h"



int main (int argc,char *argv[])
{
  suitcase_t S;

  S.argc = argc;
  S.argv = argv;

  parse_args(&S);
  ERR_trap;

  prep_files(&S);
  ERR_trap;

  S.done = 0;

  get_nextfile(&S);
  ERR_trap;

  while (!S.done)
    {
      load_cep(&S);
      ERR_trap;

      if (S.verbose)
	fprintf(stderr,"%s: segmenting %s,%s\n",S.argv[0],S.data_basen,S.data_fn);

      find_segp(&S);
      ERR_trap;

      report(&S);
      ERR_trap;

      get_nextfile(&S);
      ERR_trap;
    }

  close_files(&S);
  ERR_trap;

  return(0);
}


int parse_args (suitcase_t *S)
{
  int argp=1,argerr=0;


  ERR_reset("parse arguments");

  /* set defaults */


  S->verbose = 0;
  S->data = NULL;
  S->data_dim = 13;
  S->win_len = 250;
  S->smooth_len = 100;
  S->scan_len = 500;
  S->Kin = NULL;
  S->Ksmooth = NULL;
  S->Kthresh = 5.0/(float)S->win_len;
  S->var_floor = 0.0;
  S->frame_segp = NULL;
  S->num_segp = 0;


  *(S->ctl_fn) = 0;
  *(S->ctl_dir) = 0;
  *(S->ctl_ext) = 0;
  *(S->data_fn) = 0;
  *(S->data_basen) = 0;

  
  if (S->argc < 2)
    argerr =1;

  while (argp < S->argc && !argerr)
    {
      if (S->argv[argp][0] == '-')
	switch(S->argv[argp][1])
	  {
	  case 'c':
	    if (argp+1 < S->argc)
	      strcpy(S->ctl_fn,S->argv[++argp]);
	    else
	      argerr=1;
	    break;
	  case 'e':
	    if (argp+1 < S->argc)
	      strcpy(S->ctl_ext,S->argv[++argp]);
	    else
	      argerr=1;
	    break;
	  case 'd':
	    if (argp+1 < S->argc)
	      strcpy(S->ctl_dir,S->argv[++argp]);
	    else
	      argerr=1;
	    break;
	  case 'r':
	    if (argp+1 < S->argc)
	      strcpy(S->report_fn,S->argv[++argp]);
	    else
	      argerr=1;
	    break;
	  case 'v':
	    S->verbose = 1;
	    break;


	  case 'w':
	    /* window length for Kseg */
	    if (argp+1 < S->argc)
	      S->win_len = atoi(S->argv[++argp]);
	    else
	      argerr=1;
	    break;

	  case 's':
	    /* scanning length for peaks */
	    if (argp+1 < S->argc)
	      S->scan_len = atoi(S->argv[++argp]);
	    else
	      argerr=1;
	    break;

	       
	  case 'h':
	    /* smoothing window length */
	    if (argp+1 < S->argc)
	      S->smooth_len = atoi(S->argv[++argp]);
	    else
	      argerr=1;
	    break;

	  case 'm':
	    /* threshold of segmenter */
	    if (argp+1 < S->argc)
	      S->Kthresh = atof(S->argv[++argp]);
	    else
	      argerr=1;
	    break;

	  case 'f':
	    /* var floor for stability */
	    if (argp+1 < S->argc)
	      S->var_floor = atof(S->argv[++argp]);
	    else
	      argerr=1;
	    break;


	  default:
	    argerr = 1;break;
	  }
      else
	argerr = 1;

      argp++;
    }
  
  if (argerr)
    {
      fprintf(stderr,
	      "Cross Entropy Based Segmentation\n"
	      "Usage: %s -c <ctlfile> [-d <in dir> -e <in ext>] -r <report_file> [-v (verbose)]\n"
	      "[-w <value>] width of comparison windows (in # frames) def=%d\n"
	      "[-s <value>] scanning length for peaks (in # frames) def=%d\n"
	      "[-h <value>] length of smoothing window (Hamming, in # frames) def=%d\n"
	      "[-m <value>] minimum Kseg threshold def=%.3f\n"
	      "[-f <value>] variance floor def=%.3f\n"
	      "Compiled %s %s\n",
	      S->argv[0],
	      S->win_len,S->scan_len,S->smooth_len,S->Kthresh,S->var_floor,
	      __DATE__,__TIME__);

      ERR_return("parse error");
    }


  if (S->verbose)
    {
      fprintf(stderr,S->argv[0]);
      for (argp=1;argp<S->argc;argp++)
	fprintf(stderr," %s",S->argv[argp]);
      fprintf(stderr,"\n\n");

      fprintf(stderr,"%s: will be verbose\n",S->argv[0]);
    }

  return(0);
}

