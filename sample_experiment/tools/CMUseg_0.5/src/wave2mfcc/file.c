/*

Thu Aug 8 1996 msiegler
Matthew A. Siegler

Building filenames, opening and closing them.
Read blocks, write blocks, all other i/o.

Right now, everything is converted to int16.

*/

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <math.h>

/* for fstat */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>

/* added by JGF */
#include <sys/fcntl.h>

#include "file.h"

/* those pesky hps! */
#ifdef norint
#define rint(x) (int)((double)x + 0.5)
#endif

int32 bomb()
{
  fprintf(stderr,"ERROR in %s:\n %s\n %s\n",ERR_function,ERR_problem,ERR_param);
  exit(-1);
}


int32 file_getnextfile(parmfile_t *F)
/* take the input filename fn_in and output fn_out and
   create filepointers if fn_out is null, creates one autmatically */

{
  char line[SZline];
  char name_in[SZfn],name_out[SZfn],dummy[SZfn];
  int32 dummy_tot,dummy_toread,dummy_read,dummy_eof;

  int32 ifd;
  struct stat statbuf;    /* filestat for input file */

  ERR_reset("file_getnextfile");

  /* close files if they are open */
  if (F->fp_in != NULL)
    fclose(F->fp_in);

  if (F->sp_in != NULL)
    sp_close(F->sp_in);

  /* free some space if we haven't already,
     and set up the buffer pointers so they are ok  */

  if (F->buf_in0 == NULL)
    {
      F->buf_in0 = (int16 *)calloc(F->size_bufin * 2,sizeof(int16));
      /* put the current pointers in the middle */
      F->buf_in = F->buf_in0 + F->size_bufin;
      F->buf_in_prev = F->buf_in0;
      /* previous read does not exist, so we initialize to lots of zeros.
	 This is like having padded to the left so we can use indicies of [-i] */
    }


  if (F->fn_ctl != NULL) /* there is a control file */
    {
      /* open control file in necessary die if fail */
      if (F->fp_ctl == NULL && (F->fp_ctl = fopen(F->fn_ctl,"r"))==NULL)
	ERR_return("Couldn't open the control file");

      /* get a line of control file, or die if the end or we couldn't */
      if (fgets(line,SZline,F->fp_ctl)==NULL)
	return(-1); /* no more to get! */

      /* based on the number of items on the line, do different things */
      switch(countwords(line))
	{
	case 1: 
	  /* only an input name */
	  sscanf(line,"%s",F->bn_in);
	  strcpy(F->bn_out,F->bn_in);
	  F->time_begin = F->time_end = 0;
	  break;
	case 2:
	  sscanf(line,"%s%s",F->bn_in,F->bn_out);
	  F->time_begin = F->time_end = 0;
	  break;
	case 3:
	  sscanf(line,"%s%s%f",F->bn_in,F->bn_out,&F->time_begin);
	  F->time_end = 0;
	  break;
	case 4:
	  sscanf(line,"%s%s%f%f",F->bn_in,F->bn_out,&(F->time_begin),&(F->time_end));
	  break;
	default:
	  ERR_return("Problem parsing the control file");
	  break;
	}

      /* figure out where to begin and end (just before ending) */
      F->samples_begin = (int32)rint((double)F->rate_sample * F->time_begin);
      if (F->time_end != 0.00)
	F->samples_end = (int32)rint((double)F->rate_sample * F->time_end)-1;
      else
	F->samples_end = 0; /* none spec */

      /* construct input and output names */
      if (F->ext_in != NULL)
	sprintf(name_in,"%s.%s",F->bn_in,F->ext_in);
      else
	strcpy(name_in,F->bn_in);

      if (F->ext_out != NULL)
	sprintf(name_out,"%s.%s",F->bn_out,F->ext_out);
      else
	strcpy(name_out,F->bn_out);

      if (F->dir_in != NULL)
	sprintf(F->fn_in,"%s/%s",F->dir_in,name_in);
      else
	strcpy(F->fn_in,name_in);

      if (F->dir_out != NULL)
	sprintf(F->fn_out,"%s/%s",F->dir_out,name_out);
      else
	strcpy(F->fn_out,name_out);



    }
  /* otherwise, just use the files as specified (don't hunt for control file) */
  else
    {
      /* make sure we haven't been here before! Returns a -1 if we have */
      if (F->count_numfiles)
	return(-1);
      /* added by JGF */
      F->samples_begin = 0;
      F->samples_end = 0;

      /*
	 not true anymore.

	ERR_return("More than one file is to be processed when only one is specified");
	*/
    }


  switch (F->format_in)
    {
    case  FORMAT_IN_nist:
    case  FORMAT_IN_little:
    case  FORMAT_IN_big:
      F->bytes_per_sample = sizeof(int16);
      break;
      
    case  FORMAT_IN_ulaw:  
      F->bytes_per_sample = sizeof(char);
      break;
    }


  /* do different things with different file formats, to prepare reads */
  switch (F->format_in)
    {
    case  FORMAT_IN_nist:
      /* nist SPHERE format */
      F->fp_in = NULL;

      F->sp_in = sp_open(F->fn_in,"r");
      /* die if SPHERE call fails */
      if(F->sp_in == NULL || sp_error(F->sp_in))
	ERR_return2("SPHERE error, couldn't open the file for reading",F->fn_in);

      /* get how long the file is */
      if (sp_h_get_field(F->sp_in,"sample_count",T_INTEGER,(SP_INTEGER *)(&(F->samples_in_total))) > 0)
	F->samples_in_total = 0;

      if(sp_error(F->sp_in))
	ERR_return2("SPHERE error, couldn't examine the header",F->fn_in);

      /* pick the right internal representation to convert to */
      if (FORMAT_native == FORMAT_IN_little)
	sp_set_data_mode(F->sp_in,"SE-PCM-2:SBF-01:CH-1");
      else
	sp_set_data_mode(F->sp_in,"SE-PCM-2:SBF-10:CH-1");
      /* check for errors on data mode */
     if(sp_error(F->sp_in))
	ERR_return2("SPHERE error, trouble setting the data mode",F->fn_in);

     if (F->samples_begin && F->samples_begin > F->samples_in_total)
       ERR_return2("input file error, requested start time is after file end",F->fn_in);
     
     if (F->samples_begin && (sp_seek(F->sp_in,F->samples_begin,0) || sp_error(F->sp_in)))
       ERR_return2("SPHERE error, trouble doing a seek",F->fn_in);

     break;


    case FORMAT_IN_big:
    case FORMAT_IN_little:
    case FORMAT_IN_ulaw: /* will do conversion on the fly to native integer */
    default:

      F->sp_in = NULL;
      F->samples_in_total = 0;

      /* use stdin for file = "-" */
      if (strcmp(F->fn_in,"-"))
	{
	  /* try to figure out how long the file is */
	  if (!fstat((ifd = open(F->fn_in,O_RDONLY)),&statbuf))
	    {
	      F->samples_in_total = statbuf.st_size / F->bytes_per_sample;
	      close(ifd);
	    }

	  if ( (F->fp_in = fopen(F->fn_in,"r")) == NULL)
	    ERR_return2("input file error, couldn't open",F->fn_in);
	}
      else
	F->fp_in = stdin;

      /* make sure we aren't asking something unreasonable if we can */
      if (F->samples_in_total && F->samples_begin > F->samples_in_total)
	ERR_return2("input file error, requested start time is after file end",F->fn_in);

      /* fseek if necessary, check for error */
      if (F->samples_begin && fseek(F->fp_in,F->samples_begin * F->bytes_per_sample,0))
	ERR_return2("input file error, trouble doing an (ulaw) fseek",F->fn_in);

      break;

    }

  
  /* how many samples are we gonna read (potentially) */
  if (!F->samples_end)
    F->samples_in_gonnaread = F->samples_in_total - F->samples_begin;
  else
    if (!F->samples_in_total)
      F->samples_in_gonnaread = F->samples_end - F->samples_begin + 1;
    else
      F->samples_in_gonnaread = MIN(F->samples_end,F->samples_in_total-1) - F->samples_begin + 1;

  /* initialize the reading pointers */
  F->count_bufin_lastread = 0;
  F->count_bufin_totread = 0;

  /* how many calls so far */
  F->count_bufin_numreads = 0;
  F->count_numfiles++;

  return(0);
}

int32 file_readblock(parmfile_t *F)
/* reads a block of data from the file (fp_in or sp_in depending) 
   and puts it into buf_in. This is a block read routine that tries
   to satistify the following:

   keep reading bytes until the number read is
   : at least F->size_bufin_minread 
   : less than F->size_bufin
   : or we hit FEOF
   : or we hit the F->samples_end point

   It sets F->count_bufin_lastread and F->count_bufin_totread
   appropriately so we can figure out later how much we got.
   
   returns the number of samples actually read, <0 if error occured

   The memory structure is a FILO queue, with a total length of F->size_bufin * 2;
*/

{
  int end_of_file = 0;
  int nread_did = 0;
  int nread_ask = 0;

  int16 *buf_ptr;


  ERR_reset("file_readblock");

  /* move data out of the active half of the queue by the amount we last read (if any) */
  if (F->count_bufin_lastread)
    {
      /*
	 |-------xxxxxxx|ooooo--------|  bufin_lastread = 5
	 |--xxxxxxxooooo|-------------|
       */

      /* memmove is not on all architectures in the same way */
      f_memmove(F->buf_in0,F->buf_in0 + F->count_bufin_lastread,F->size_bufin * sizeof(int16));
      /* slides over everything, even things that were just zeros before */

      F->count_bufin_lastread = 0;
    }

  /* get some data, return the actual amount in nread_did.
     Remember to at most read how much we have left! Stuff is put in after
     buf_ptr which slides with the read data. */

  buf_ptr = F->buf_in;

    do
      {
	/* read at most however much room we have left or want */
	if (F->samples_end)
	  nread_ask = MAX(
			  MIN((F->size_bufin - F->count_bufin_lastread),
			  (F->samples_end - F->samples_begin - F->count_bufin_totread + 1)),
			  0);
	else
	  nread_ask = F->size_bufin - F->count_bufin_lastread;

	if (nread_ask) /* just in case! */
	  {
	    switch(F->format_in)
	      {
	      case FORMAT_IN_nist:
		nread_did =  sp_read_data(F->buf_in,nread_ask,F->sp_in);
		end_of_file = sp_eof(F->sp_in);
		if (sp_error(F->sp_in))
		  ERR_return2("SPHERE: Input read error",F->fn_in);
		break;
	      case FORMAT_IN_ulaw:
		/* kind of hacky. Read in as chars, and convert them to int16 */
		nread_did = fread(buf_ptr,sizeof(char),nread_ask,F->fp_in);
		end_of_file = feof(F->fp_in);
		
		if (nread_did)
		  block_ulaw2lin(buf_ptr,nread_did);
		break;
	      case FORMAT_IN_big:
	      case FORMAT_IN_little:
		nread_did = fread(buf_ptr,sizeof(int16),nread_ask,F->fp_in);
		end_of_file = feof(F->fp_in);
		
		if (nread_did && F->format_in != FORMAT_native)
		  block_swaplin(buf_ptr,nread_did);
		break;
	      }
	    
	    /* accumulate the amount we have read sofar */
	    F->count_bufin_lastread += nread_did;

	    /* slide over the read pointer so we don't clobber ourselves */
	    buf_ptr += nread_did;
	  }

      }
    while (F->count_bufin_lastread <= F->size_bufin_minread && !end_of_file && nread_ask);
  /* keep going as long as we ask for it, do not hit the eof, and haven't exceeded our minread */


  F->count_bufin_totread += F->count_bufin_lastread;

  if (F->count_bufin_lastread)
    F->count_bufin_numreads++;
  /* don't count feof */

  return(F->count_bufin_lastread);
}




void block_swaplin(int16 *buf,int howmany)
{
  /* byteswap the buffer */
  int i;
  for (i=0;i<howmany;i++)
    SWAP16(buf+i);
}

static int16 table_ulaw2lin[256] = {-32124, -31100, -30076, -29052, 
    -28028, -27004, -25980, -24956, -23932, -22908, -21884, -20860, 
    -19836, -18812, -17788, -16764, -15996, -15484, -14972, -14460, 
    -13948, -13436, -12924, -12412, -11900, -11388, -10876, -10364, 
    -9852, -9340, -8828, -8316, -7932, -7676, -7420, -7164, -6908, 
    -6652, -6396, -6140, -5884, -5628, -5372, -5116, -4860, -4604, 
    -4348, -4092, -3900, -3772, -3644, -3516, -3388, -3260, -3132, 
    -3004, -2876, -2748, -2620, -2492, -2364, -2236, -2108, -1980, 
    -1884, -1820, -1756, -1692, -1628, -1564, -1500, -1436, -1372, 
    -1308, -1244, -1180, -1116, -1052, -988, -924, -876, -844, -812, 
    -780, -748, -716, -684, -652, -620, -588, -556, -524, -492, -460, 
    -428, -396, -372, -356, -340, -324, -308, -292, -276, -260, -244, 
    -228, -212, -196, -180, -164, -148, -132, -120, -112, -104, -96, 
    -88, -80, -72, -64, -56, -48, -40, -32, -24, -16, -8, 0, 32124, 
    31100, 30076, 29052, 28028, 27004, 25980, 24956, 23932, 22908, 
    21884, 20860, 19836, 18812, 17788, 16764, 15996, 15484, 14972, 
    14460, 13948, 13436, 12924, 12412, 11900, 11388, 10876, 10364, 
    9852, 9340, 8828, 8316, 7932, 7676, 7420, 7164, 6908, 6652, 6396, 
    6140, 5884, 5628, 5372, 5116, 4860, 4604, 4348, 4092, 3900, 3772, 
    3644, 3516, 3388, 3260, 3132, 3004, 2876, 2748, 2620, 2492, 2364, 
    2236, 2108, 1980, 1884, 1820, 1756, 1692, 1628, 1564, 1500, 1436, 
    1372, 1308, 1244, 1180, 1116, 1052, 988, 924, 876, 844, 812, 
    780, 748, 716, 684, 652, 620, 588, 556, 524, 492, 460, 428, 396, 
    372, 356, 340, 324, 308, 292, 276, 260, 244, 228, 212, 196, 180, 
    164, 148, 132, 120, 112, 104, 96, 88, 80, 72, 64, 56, 48, 40, 
    32, 24, 16, 8, 0};

void block_ulaw2lin(int16 *buf,int howmany)
{
/* mu-law to PCM conversion table */
  char *temp;
  int32 i;

  temp = (char *)malloc(howmany);
  memcpy(temp,buf,howmany);

  for (i=0;i<howmany;i++)
    buf[i] = table_ulaw2lin[temp[i]];

  free(temp);
}

int countwords(char *line)
/* return number of words */
    {
      register int num,on;
      register char *here;
      num = on = 0;
      here = line;

      do
        if (*here)
          {
            if (*here == ' ' || *here == '\n' || *here == '\t' || *here == 0 )
              on = 0;
            else 
              if (!on)
                {
                  num++;
                  on = 1;
                }
          }
      while (*here++);
      return(num);
    }


char *nextword(char *here)
/* return pointer to beginning of next word (or NULL if last word) */
{
  register int on;
  register char *where;

  on = 1;
  where = here;

  if (!*where)
    return(NULL);

  do
    if (*where) 
      {
        if (*where == ' ' || *where == '\n' || *where == '\t')
          on = 0;
        else if (!on)
          return(where);
      }
  while(*(where++));
  return(NULL);
}
