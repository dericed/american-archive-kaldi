
/*
   wave2mfcc
   
   Convert an audio stream into mel-freq cepstral coefficients
   (from adc2mfcc of many many many credits)


   HISTORY
   
   14 August 1996 Matthew A. Siegler (msiegler@cs.cmu.edu)
   Using front-end processing developed for the live system by
   Ravi Mosur (rkm@cs.cmu.edu) and Eric Thayer (eht@cs.cmu.edu)
   instead of the batch-style processing of the old adc2mfcc.

   The intent is to deal with very large files and streams also,
   by not forcing the entire waveform and cepstra files to be memory
   resident.

   This requires all the filtering to have static values, or priors,
   to seed the next block of data.

*/


#include "wave2mfcc.h"


int32 main(int32 argc,char *argv[])
{
  suitcase_t *S;
  
  S = (suitcase_t *)malloc(sizeof(suitcase_t));
  S->F = (parmfile_t *)malloc(sizeof(parmfile_t));

  S->argv = argv;
  S->argc = argc;

  if (parse_cmdline(S))
    return(-1);

  fe_defaults();
  
  if (S->verbose)
    tell_all(S);

  if(convert_files(S))
    return(-1);
  
  free(S->F);
  free(S);

  return(0);
}

int32 parse_cmdline(suitcase_t *S)
{
/* msiegler 15 Aug 1996
   I am not very happy about the way I did this, so perhaps
   another can fix the massive if-then-else-if structure.
*/

  int argp=1,argerr=0,argget=0;
  char *arg;
  parmfile_t *F;

  /* mfc parameters */
  SAMPLE_RATE = FILTER_DEFAULT;
  DFT_POINTS = FILTER_DEFAULT;
  LOG2_DFT_POINTS = FILTER_DEFAULT;
  FRAME_RATE = FILTER_DEFAULT;
  FRAME_SPACING = FILTER_DEFAULT;
  LEN_WINDOW = FILTER_DEFAULT;
  NO_OF_FILTERS = FILTER_DEFAULT;
  NUMBER_OF_LOG_TRIANGLES = FILTER_DEFAULT;
  CEP_VECLEN = FILTER_DEFAULT;
  LOG_SPACING = FILTER_DEFAULT;
  LINEAR_SPACING = FILTER_DEFAULT;
  LOWER_EDGE_OF_FILTERS = FILTER_DEFAULT;
  UPPER_EDGE_OF_FILTERS = FILTER_DEFAULT;
  HAMMING_WINDOW_LENGTH = FILTER_DEFAULT;
  PREEMPH_ALPHA = FILTER_DEFAULT;

  /* other parameters */
  S->verbose = 0;
  S->dither = 0;
  S->sphinx = 0;
  S->addnoise = 0;
  S->babble = 0;
  S->resample = 0;
  S->logspec = 0;
  S->showfilt = 0;
  /* Added by JGF */
  S->dcremove = 1;
  S->verbose = 0;
  /***/

  /* makes notation easier */
  F = S->F;

  /* clear filepointers */
  F->fp_in = F->fp_ctl = NULL;
  F->fp_out = NULL;
  F->sp_in = NULL;

  /* clear filenames */
  F->fn_ctl = F->dir_in = F->ext_in = F->dir_out = F->ext_out = NULL;
  *(F->fn_in) = *(F->fn_out) = *(F->bn_in) = *(F->bn_out) = 0;

  /* clear the buffer */
  F->buf_in0 = NULL;
  
  while (argp < S->argc && !argerr)
    {
      if (S->argv[argp][0] == '-')
	{
	  arg = S->argv[argp]+1;

	  /*arguments requiring no parameters */
	  if (!strcmp(arg,"verbose"))
	    S->verbose = 1;
	  else if (!strcmp(arg,"logspec"))
	    S->logspec = 1;
	  else if (!strcmp(arg,"showfilt"))
	    S->showfilt = 1;
	  else if (!strcmp(arg,"sphere"))
	    F->format_in = FORMAT_IN_nist;
	  else if (!strcmp(arg,"adc"))
	    {
	      fprintf(stderr,"%s: ERROR\n\t ADC format is not implemented yet.\n",S->argv[0]);
	      argerr = 1;
	    }
	  else if (!strcmp(arg,"sphinx"))
	    S->sphinx=1;
	  else if (!strcmp(arg,"dither"))
	    S->dither=1;
	  else if (!strcmp(arg,"DC"))
	    S->dcremove = 1;
	  else if (argp+1 < S->argc)
	    {
	      /* arguments requiring at least one parameter go here */
	      if (!strcmp(arg,"i"))
		strcpy(F->fn_in,S->argv[++argp]);
	      else if (!strcmp(arg,"o"))
		strcpy(F->fn_out,S->argv[++argp]);
	      else if (!strcmp(arg,"c"))
		F->fn_ctl = strdup(S->argv[++argp]);
	      else if (!strcmp(arg,"di"))
		F->dir_in = strdup(S->argv[++argp]);
	      else if (!strcmp(arg,"do"))
		F->dir_out = strdup(S->argv[++argp]);
	      else if (!strcmp(arg,"ei"))
		F->ext_in = strdup(S->argv[++argp]);
	      else if (!strcmp(arg,"eo"))
		F->ext_out = strdup(S->argv[++argp]);
	      
	      else if (!strcmp(arg,"raw"))
		switch(S->argv[++argp][0])
		  {
		  case 'B':
		    F->format_in = FORMAT_IN_big;
		    break;
		  case 'L':
		    F->format_in = FORMAT_IN_little;
		    break;
		  case 'M':
		  case 'U':
		    F->format_in = FORMAT_IN_ulaw;
		    break;
		  default:
		    argerr=1;
		    break;
		  }
	      else if (!strcmp(arg,"alpha"))
		PREEMPH_ALPHA = (float)atof(S->argv[++argp]);
	      else if (!strcmp(arg,"srate"))
		SAMPLE_RATE = (float)atof(S->argv[++argp]);
	      else if (!strcmp(arg,"frate"))
		FRAME_RATE = (float)atof(S->argv[++argp]);
	      else if (!strcmp(arg,"wlen"))
		HAMMING_WINDOW_LENGTH = (float)atof(S->argv[++argp]);
	      else if (!strcmp(arg,"dft"))
		DFT_POINTS = (float)atof(S->argv[++argp]);
	      else if (!strcmp(arg,"nfilt"))
		NO_OF_FILTERS = (int32)atoi(S->argv[++argp]);
	      else if (!strcmp(arg,"nlog"))
		NUMBER_OF_LOG_TRIANGLES = (int32)atoi(S->argv[++argp]);
	      else if (!strcmp(arg,"logsp"))
		LOG_SPACING = (float)atof(S->argv[++argp]);
	      else if (!strcmp(arg,"linsp"))
		LINEAR_SPACING = (float)atof(S->argv[++argp]);
	      else if (!strcmp(arg,"lowerf"))
		LOWER_EDGE_OF_FILTERS = (float)atof(S->argv[++argp]);
	      else if (!strcmp(arg,"upperf"))
		UPPER_EDGE_OF_FILTERS = (float)atof(S->argv[++argp]);
	      else if (!strcmp(arg,"ncep"))
		CEP_VECLEN = (int32)atoi(S->argv[++argp]);
	      else if (!strcmp(arg,"addnoise"))
		{
		  fprintf(stderr,"%s: ERROR\n\t Addnoise is not implemented yet.\n",S->argv[0]);
		  argerr = 1;

		  S->addnoiseSNR = (float)atof(S->argv[++argp]);
		  S->addnoise = 1;
		}

	      else if (argp+1 < S->argc - 1)
		{
		  /* arguments requiring at least 2 parameters */
		  
		  if (!strcmp(arg,"babble"))
		    {
		      fprintf(stderr,"%s: ERROR\n\t Babble is not implemented yet.\n",S->argv[0]);
		      argerr = 1;

		      S->babbleSNR = (float)atof(S->argv[++argp]);
		      S->fn_babble = strdup(S->argv[++argp]);
		      S->babble = 1;
		    }
		  else if (argp+1 < S->argc - 2)
		    {
		      /* arguments requiring at least 3 parameters */
		      if (!strcmp(arg,"resample"))
			{
			  fprintf(stderr,"%s: ERROR\n\t Resampling is not implemented yet.\n",S->argv[0]);
			  argerr = 1;

			  S->resample_up = (int32)atoi(S->argv[++argp]);
			  S->resample_down = (int32)atoi(S->argv[++argp]);
			  S->fn_resampleFIR = strdup(S->argv[++argp]);
			  S->resample = 1;
			}
		      else
			argerr = 1;
		    }
		  else 
		    argerr = 1;
		}
	      else
		argerr = 1;
	    }
	  else 
	    argerr = 1;
	}
      else
	argerr=1;

      /* this is how many arguments we found */
      if (!argerr)
	argget++;

      /* find the next argument */
      argp++;
    }

  if (argerr || !argget)
    {
#include "main.help.h"
      return(-1);
    }

       
   return(0);
}

int32 tell_all(suitcase_t *S)
/* tell what flags/parms are set */
{
  parmfile_t *F;
  char *NADA = "(nothing)";
  fprintf (stderr,"%s: Verbose Diagnostics\n",S->argv[0]);

#define IS(x) ((x != NULL && *x != 0)? x : NADA)
#define YAH(x) ((x)? "yes":"no")
#define RAW(x) ((x==FORMAT_IN_big)?"Big Endian":(x==FORMAT_IN_little)?"Little Endian":(x==FORMAT_IN_ulaw)?"Mu-Law":"none")

  F = S->F;
  if (F->fn_ctl == NULL)
    fprintf(stderr,"\t input file:%s  output file:%s\n",IS(F->fn_in),IS(F->fn_out));
  else
    fprintf(stderr,
	    "\t control file:%s\n"
	    "\t input dir:%s  input ext:%s\n"
	    "\t output dir:%s  output ext:%s\n",
	    IS(F->fn_ctl),IS(F->dir_in),IS(F->ext_in),IS(F->dir_out),IS(F->ext_out));

  fprintf(stderr,"\t Input is sphere:%s  Input is raw:%s (%s)\n",
	  YAH(F->format_in==FORMAT_IN_nist),YAH(F->format_in!=FORMAT_IN_nist),RAW(F->format_in));

  fprintf(stderr,"\t Sphinx-compatible output:%s\n",YAH(S->sphinx));

  fprintf(stderr,"FILTER PARAMETERS\n");
  
  fprintf(stderr,
	  "\t Premphasis alpha=%.3f  Sampling Rate=%.3f  Frame Rate=%d\n"
	  "\t Hamming Window Length=%.4f  DFT size=%d\n"
	  "\t Number of filters=%d  Number of log-spaced filters=%d\n"
	  "\t Log Spacing=%.5f  Linear Spacing=%.5f\n"
	  "\t Lower Edge of filters=%.5f  Upper Edge of filters=%.5f\n"
	  "\t Number of cepstral coefficients to retain=%d\n",
	  PREEMPH_ALPHA, SAMPLE_RATE, FRAME_RATE, HAMMING_WINDOW_LENGTH, DFT_POINTS,
	  NO_OF_FILTERS, NUMBER_OF_LOG_TRIANGLES, LOG_SPACING, LINEAR_SPACING,
	  LOWER_EDGE_OF_FILTERS, UPPER_EDGE_OF_FILTERS,
	  CEP_VECLEN);  

  fprintf(stderr,"INPUT PROCESSING\n");

  fprintf(stderr,"\t DC offset removal:%s  Dither:%s\n",
	  YAH(S->dcremove),YAH(S->dither));

  fprintf(stderr,"\t Resample:%s ",YAH(S->addnoise));
  if (S->resample)
    fprintf(stderr," Up factor=%d  Down factor=%d  FIR file=%s\n",
	    S->resample_up,S->resample_down,S->fn_resampleFIR);
  else
    fprintf(stderr,"\n");

  fprintf(stderr,"\t Add Noise:%s ",YAH(S->addnoise));
  if (S->addnoise)
    fprintf(stderr," SNR=%.2f\n",S->addnoiseSNR);
  else
    fprintf(stderr,"\n");

  fprintf(stderr,"\t Add Babble:%s ",YAH(S->addnoise));
  if (S->babble)
    fprintf(stderr," SNR=%.2f File:%s\n",S->addnoiseSNR,S->fn_babble);
  else
    fprintf(stderr,"\n");
  

  return(0);
}


/* display filters, error trap for input files, verbose display */
int32 convert_setup(suitcase_t *S,parmfile_t *F)
{
  int i;
  /* print out the filters to stdout */
  if (S->showfilt)
    {
      for (i=0;i<NO_OF_FILTERS;i++)
	fprintf(stdout,"filter #%d %.3f %.3f %.3f\n",
		i+1,mel_filters[i].left_edge,mel_filters[i].center_freq,mel_filters[i].right_edge);
    }

  if (F->fn_ctl == NULL && (*(F->fn_in) == NULL || *(F->fn_out) == NULL))
    {
      fprintf(stderr," %s: ERROR\n\tspecify either a control file or input/output files\n",S->argv[0]);
      return(-1);
    }

  if (F->fn_ctl !=NULL && *(F->fn_in) != NULL)
    {
      fprintf(stderr," %s: WARNING\n\tignoring control file, using input/output files instead\n",S->argv[0]);
      free(F->fn_ctl); F->fn_ctl = NULL;
    }

  /* for seeking to particular locations in a file, may be different than for filters (!) */
  if (!S->resample)
    F->rate_sample = (int32)(SAMPLE_RATE);
  else
    F->rate_sample = (int32)(SAMPLE_RATE * (float)S->resample_up / (float)S->resample_down + 0.5);

  if (S->verbose)
    fprintf (stderr," %s: INFO\n\t using sample rate of\n\t\t %d Hz for input files,\n\t\t %d Hz for output files\n",
	     S->argv[0],(int32)F->rate_sample,(int32)SAMPLE_RATE);

  /* do a second of speech at a time or 4 windows worth */
  F->size_bufin = MAX((int32)(F->rate_sample),LEN_WINDOW*4);
  F->size_bufin_minread = MIN((int32)(F->rate_sample),LEN_WINDOW*2);
  F->count_numfiles = 0;
  
  if (S->verbose)
    fprintf(stderr," %s: INFO\n\t input buffer is %d samples\n",S->argv[0],F->size_bufin);

  return(0);
}

int32 prep_file(suitcase_t *S,parmfile_t *F)
{
  if (S->verbose)
    {
      fprintf (stderr," %s: INFO\n\t reading from %s\n",
	       S->argv[0],F->fn_in);
      
      if (F->samples_in_total > 0)
	fprintf (stderr, "\t Total of %ld samples available in %s\n",F->samples_in_total,F->fn_in);
      else
	fprintf (stderr, "\t Unknown number of samples available in %s\n",
		 F->fn_in);
      
      if (F->samples_in_gonnaread > 0)
	fprintf (stderr, "\t Will read a total of %ld samples\n",F->samples_in_gonnaread,F->fn_in);
      else
	fprintf (stderr, "\t Unknown number of samples to read\n",
		 F->fn_in);
    }
  else
    if (F->fn_ctl != NULL)
      fprintf(stderr,"%s\n",F->bn_in);
  
  
  /* massive hack 
     always write even numbers of frames by definition:   ceil(x/2)*2
     always write only as much data as we have:           floor((samples - len_window)/frame spacing)
     */
  
  F->vectors_out_gonnawrite = 
    MAX(0,(int32)ceil(floor(((double)F->samples_in_gonnaread - LEN_WINDOW) / FRAME_SPACING) / 2) * 2);
  
  if (S->verbose)
    {
      if (F->vectors_out_gonnawrite > 0)
	fprintf (stderr, "\t Will write a total of %ld vectors\n",F->vectors_out_gonnawrite,F->fn_in);
      else
	fprintf (stderr, "\t Unknown number of vectors to write\n",F->fn_in);
    }
  
  /* get an mfc file pointer */
  
  if (F->fp_out != NULL)
    mfc_fclose(F->fp_out);
  
  F->fp_out = mfc_fopen(F->fn_out,"w");
  
  if (F->fp_out == NULL)
    {
      fprintf(stderr," %s:ERROR\n\t Couldn't open %s for writing\n",S->argv[0],F->fn_out);
      return(-1);
    }
  
  /* set some vector parameters for the write */
  F->fp_out->avail_vectors = F->vectors_out_gonnawrite;
  
  if (!S->logspec)
    F->fp_out->size_vector = CEP_VECLEN;
  else
    F->fp_out->size_vector = NO_OF_FILTERS;
  
  F->fp_out->size_el = sizeof(float);
  
  /* write a header if necessary */
  if (S->sphinx)
    {
      F->fp_out->endian = ENDIAN_BIG;
      mfc_fwrite_hdr(F->fp_out,header_SPHINX);
      if (S->verbose)
	fprintf(stderr," %s:INFO\n\t Wrote a sphinx header to the file (forced BigEndian)\n",S->argv[0]);
    }
  else
    F->fp_out->endian = ENDIAN_NATIVE;

  return(0);
}


/* this procedure is KIND OF BIG! */
int32 convert_files(suitcase_t *S)
{
  int32 nframes,totframes,nread;
  int32 i,j;
  parmfile_t *F;
  float **cep;
  int32 dc_size;
  double dc_sum;

  F = S->F;

  fe_init();

  /* do lots of stuff, bomb out if failure */
  if (convert_setup(S,F) < 0)
    return(-1);

  /* make a cepstra buffer */
  cep = NULL;
  if (!S->logspec)
    cep = (float **)alloc2d(F->size_bufin / FRAME_SPACING + 1,CEP_VECLEN,sizeof(float));
  else
    cep = (float **)alloc2d(F->size_bufin / FRAME_SPACING + 1,NO_OF_FILTERS,sizeof(float));

  if (cep == NULL)
   { 
      fprintf(stderr," %s: ERROR\n\t could not allocate space for buffer\n",S->argv[0]);
      return(-1);
   }

  while (!file_getnextfile(F))
    {

      /* open,get headers,do verbose outputs*/
      if (prep_file(S,F)<0)
	return(-1);

      /* initialize the front end */
      fe_start();

      dc_size = 0.0;
      dc_sum = 0.0;
      totframes = 0;
      while(nread = file_readblock(F))
	{
	  /*
	    Resample if necessary
	    Add noise and/or babble
	    DC offset removal
	    Dither
	    */

	  /* add 1/2-bit dither noise */
	  if (S->dither)
	    signal_dither(F->buf_in,F->count_bufin_lastread);

	  if (S->dcremove)
	    signal_dcremove(F->buf_in,F->count_bufin_lastread,&dc_sum,&dc_size);

	  F->buf_work = F->buf_in;
	  F->count_bufwork_lastget = F->count_bufin_lastread;
		
	  /* sometimes the processing needs lots more data than the cepstral part 
	     so we need to get some more before doing the cep */
	  if (F->count_bufwork_lastget)
	    {
	      if (!S->logspec)
		nframes = fe_raw2cep(F->buf_work,F->count_bufwork_lastget,cep);
	      else
		nframes = fe_raw2spec(F->buf_work,F->count_bufwork_lastget,cep);

	      mfc_fwrite(*cep,nframes,F->fp_out);
	      totframes += nframes;
	    }
	}      
      if (S->verbose)
	fprintf (stderr," %s: INFO\n\t total of %d samples for %d frames written to %s\n",
		 S->argv[0],F->count_bufin_totread,totframes,F->fn_out);

      fe_stop();
      
      mfc_fclose(F->fp_out);
      F->fp_out = NULL;
    }

  if (*ERR_problem)
    bomb();

  free2d(cep);
  
  return(0);
}




