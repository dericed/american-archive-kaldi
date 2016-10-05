/* input and output routines */

#include "mfc_io.h"
#include "UTT_findsil.h"



int prep_files (suitcase_t *S)
{

  ERR_reset("prep files");
  if (  (S->report_fp = fopen(S->report_fn,"w")) == NULL)
    ERR_return2("failed to open report for writing",S->report_fn);
  if (  (S->ctl_fp = fopen(S->ctl_fn,"r")) == NULL)
    ERR_return2("failed to open control file for reading",S->ctl_fn);
  return(0);
}


int close_files (suitcase_t *S)
{

  ERR_reset("close files");
  if ( fclose(S->report_fp))
    ERR_return("failed to close report");
  if ( fclose(S->ctl_fp))
    ERR_return("failed to close control file");
  return(0);
}

int get_nextfile (suitcase_t *S)
/* get next item in control file, figure out if it's valid */
{

  int  nbounds,nitems,size,bn,*bounds,point;
  int  insert,ninserts,gap;

  char *here;
  char line[SZline];
  ERR_reset("get the next file");

  *line=(char)0;
  if (feof(S->ctl_fp) || fgets(line,SZline,S->ctl_fp)==NULL)
    {
      S->done=1;
      return(-1);
    }
  /* added by Jon Fiscus */
  if (*(line + strlen(line) - 1) != '\n')
    ERR_return2("ctline line too large for buffer ",line);

  nbounds = countwords(line) - 4;
  
  if (nbounds < 0)
    /* crappy input format */
    ERR_return2("ctlfile line does not conform to spec",line);  

  here = line;
  
  if ( sscanf(here,"%s%d%d%s",S->data_basen,&S->frame_start,&S->frame_end,S->data_fn) < 4)
    /* crappy input format */
    ERR_return2("ctlfile line does not conform to spec",line);  

  /* skip head */


  /* clear old garbage */  
  if (S->boundaries_point != NULL)
    free(S->boundaries_point);
  if (S->boundaries_fake != NULL)
    free(S->boundaries_fake);

  size = S->frame_end - S->frame_start + 1;
  S->data_size = size;

  if (size <= 0)
    ERR_return("ctl item has end before start");
  
  if (S->frame_start < 0)
    ERR_return("ctl item has a negative start value");


  S->boundaries_point = (int *)calloc(size,sizeof(int));
  S->boundaries_fake = (int *)calloc(size,sizeof(int));

  if (S->max_length && size < S->max_length || S->min_length && size < S->min_length)
    {
      /* utt is just fine without any mods or boundaries */
      S->num_boundaries = 0;
      return(0);
    }

  here = nextword(here);
  here = nextword(here);
  here = nextword(here);
  here = nextword(here);

  /* if we actually found some in the ctl file */
  if (nbounds)
    {
      bounds = (int *)calloc(sizeof(int),nbounds+1);
    
      for (nitems = 0; nitems < nbounds; nitems++)
	{
	  sscanf(here,"%d",&(bounds[nitems]));
	  bounds[nitems] -= S->frame_start;
	  here = nextword(here);
	}
      /* last one is the end of utt */
      bounds[nitems] = size-1;
      nbounds++;

      S->boundaries_point[0] = 0;
      S->boundaries_fake[0] = 0;
      bn = 1;
      
      nitems = 0;
      
      while (nitems < nbounds)
	{
	  point = bounds[nitems];

	  if (S->max_length && ( point - S->boundaries_point[bn-1] > S->max_length))
	    /* insert some new ones */
	    {

	      ninserts  = (point - S->boundaries_point[bn-1]) / S->max_length ;
	      /* this is how many inserts to make */

	      gap = (point - S->boundaries_point[bn-1]) / (ninserts + 1);
	      /* this is how big to make them */

	      for (insert=0;insert<ninserts;insert++)
		{
		  S->boundaries_point[bn] = S->boundaries_point[bn-1] + gap;
		  S->boundaries_fake[bn] = 1;
		  bn++;
		}
	    }

	  if (!S->min_length || (point - S->boundaries_point[bn-1] >= S->min_length))
	    /* don't ignore the current one if it's not too close */
	    {
	      S->boundaries_point[bn] = point;
	      S->boundaries_fake[bn] = 0;
	      bn++;
	    }

	  nitems++;
	}

      S->num_boundaries = bn;

    }      
  else
    /* no boundaries specified */
    {
      S->boundaries_point[0] = 0;
      S->boundaries_fake[0] = 0;
      bn = 1;

      point = size-1;

      if (S->max_length)
	{
	  ninserts  = (point - S->boundaries_point[bn-1]) / S->max_length ;
	  /* this is how many inserts to make */
	  
	  gap = (point - S->boundaries_point[bn-1]) / (ninserts + 1);
	  /* this is how big to make them */
	  
	  for (insert=0;insert<ninserts;insert++)
	    {
	      S->boundaries_point[bn] = S->boundaries_point[bn-1] + gap;
	      S->boundaries_fake[bn] = 1;
	      bn++;
	    }
	  
	  if (!S->min_length || (point - S->boundaries_point[bn-1] >= S->min_length))
	    /* don't ignore the current one if it's not too close */
	    {
	      S->boundaries_point[bn] = point;
	      S->boundaries_fake[bn] = 0;
	      bn++;
	    }
	}
      else
	{
	  S->boundaries_point[bn] = point;
	  S->boundaries_fake[bn] = 0;
	  bn++;
	}

      S->num_boundaries = bn;
    }
  
  return(0);
}

int load_cep (suitcase_t *S)
/* load the cepstral vector as pointed to */
{
  char fn[SZfn],dummy[SZfn];
  int toread,nread;

  ERR_reset("load the cepstral file");
  
  if (*S->ctl_dir)
    sprintf(dummy,"%s/%s",S->ctl_dir,S->data_basen);
  else
    strcpy(dummy,S->data_basen);

  if (*S->ctl_ext)
    sprintf(fn,"%s.%s",dummy,S->ctl_ext);
  else
    strcpy(fn,dummy);

  if ((S->mfc_fp = mfc_fopen(fn,"r")) == NULL)
    ERR_return2("couldn't open the file for reading",fn);
  
  if (mfc_fread_hdr(S->mfc_fp,header_SPHINX))
    ERR_return2("couldn't read the header",fn);

  if (mfc_fseek(S->mfc_fp,S->frame_start))
    {
      sprintf(dummy,"%d",S->frame_start);
      ERR_return2("couldn't seek to starting frame",dummy);
    }

  if (S->data != NULL)
    free(S->data);
  
  toread = S->frame_end - S->frame_start + 1;

  S->data = calloc((S->mfc_fp)->size_vector*(S->mfc_fp)->size_el,toread);
  if (S->data == NULL)
    {
      sprintf(dummy,"%d",(S->mfc_fp)->size_vector*(S->mfc_fp)->size_el * toread);
      ERR_return2("couldn't allocate space for data buffer",dummy);
    }

  nread = mfc_fread(S->data,toread,S->mfc_fp);

  if (nread != toread)
    {
      sprintf(dummy,"read:%d wanted:%d",toread,nread);
      ERR_return2("Read the wrong number of frames from from the file.",dummy);
    }

  mfc_fclose(S->mfc_fp);

  return(0);
}

int report (suitcase_t *S)
/* report the results */
{
  int i,first,last;

  ERR_reset("report on findings");
  
  fprintf(S->report_fp,"%s",S->data_fn);

  for (i=0;i<S->num_silences;i++)
    {
      if (S->silences_found[i])
	fprintf(S->report_fp," %d",S->silences_point[i]+S->frame_start);
      if (S->report_sildetail)
	fprintf(S->report_fp," %.2f %.2f",S->silences_depth[i],S->silences_dn[i]);

    }

  fprintf(S->report_fp,"\n");
  fflush(S->report_fp);

  return(0);
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
