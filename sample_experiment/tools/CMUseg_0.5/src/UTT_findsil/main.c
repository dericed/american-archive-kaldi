/*

20 Sep 1996
Matthew A. Siegler
msiegler@cs.cmu.edu

Given a control file listing .mfc files, or parts of them, and
 locations within them to find silences.

Given a minimum utterance length, and a maximum utterance length to create.
Generate breakpoints at silences in a report


Control file format:

 <filename> <beginF> <endF> <uttID> [<breakP #1> [ <breakP #2> ] ... ] 


All numbers are in Frames relative to beginF, and
there needn't be any breakpoints designated at all.


Report file format:

 <filename> <uttID> [<silence #1> [ <silence #2>] ... ]


 parse input
 open ctlfile
 foreach item
  insert boundaries (real & fake)
  read cepstra
  find silences
  report
 close all

*/


#include "UTT_findsil.h"



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

      find_silences(&S);
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
  S->report_sildetail = 0;

  S->win_len = 100;
  S->silence_thresh = 20.0;
  S->silence_dn = 10.0;
  S->silence_search_region = S->win_len*2;
  S->silence_search_region_f = S->win_len*5;
  S->silence_window = S->win_len/10;
  S->silence_window_outer = S->win_len*2;
  S->max_length = 0;
  S->min_length = 0;
  S->data_dim = 13;

  S->boundaries_point = NULL;
  S->boundaries_fake = NULL;
  S->silences_point = NULL;
  S->silences_found = NULL;
  S->snr = NULL;
  S->dn = NULL;

  /* read args */

  S->data = NULL;

  *(S->ctl_fn) = 0;
  *(S->ctl_dir) = 0;
  *(S->ctl_ext) = 0;
  *(S->data_fn) = 0;
  *(S->data_basen) = 0;
  *(S->report_fn) = 0;
  
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
	    

	  case 's':
	    if (argp+1 < S->argc)
	      {
		switch(S->argv[argp][2])
		  {
		  case 't':
		    S->silence_thresh = atof(S->argv[++argp]);
		    break;
		  case 'd':
		    S->silence_dn = atoi(S->argv[++argp]);
		    break;

		  case 'n':
		    S->silence_search_region = atoi(S->argv[++argp]);
		    break;
		  case 'f':
		    S->silence_search_region_f = atoi(S->argv[++argp]);
		    break;

		  case 'W':
		    S->silence_window_outer = atoi(S->argv[++argp]);
		    break;
		  case 'w':
		    S->silence_window = atoi(S->argv[++argp]);
		    break;

		  default:
		    argerr=1;
		    break;
		  }
	      }
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

	  case 'D':
	    S->report_sildetail = 1;
	    break;

	  case 'm':
	    if (argp+1 < S->argc)
	      S->max_length = atoi(S->argv[++argp]);
	    else
	      argerr=1;
	    break;

	  case 'n':
	    if (argp+1 < S->argc)
	      S->min_length = atoi(S->argv[++argp]);
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
	      "Power-Based Silence Detection\n"
	      "Usage: %s -c <ctlfile> [-d <in dir> -e <in ext>] -r <report_file> [-v (verbose)]\n"
	      "[-D] output silence details (depth,DNR)\n"
	      "[-st <value>] silence threshold (dB) def=%4.2f\n"
	      "[-sd <value>] silence DN (dB) def=%4.2f\n"

	      "[-sn <value>] width of silence search region (in # frames) def=%d\n"
	      "[-sf <value>] width of silence search region (in # frames) for fakes def=%d\n"

	      "[-sw <value>] width of silence inner window (in # frames) def=%d\n"
	      "[-sW <value>] width of silence outer window (in # frames) def=%d\n"

	      "[-m <value>] maximum length restriction (0=unlimited) def=0\n"
	      "[-n <value>] minimum length restriction (0=unlimited) def=0\n"
	      "Compiled %s %s\n",
	      S->argv[0],S->silence_thresh,S->silence_dn,S->silence_search_region,S->silence_search_region_f,
	      S->silence_window,S->silence_window_outer,
	      __DATE__,__TIME__);

      ERR_return("parse error");
    }


  if (S->verbose)
    fprintf(stderr,"%s: will be verbose\n",S->argv[0]);

  return(0);
}

