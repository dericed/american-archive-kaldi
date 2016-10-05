/* input and output routines */

#include "mfc_io.h"
#include "UTT_Kseg.h"



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


  if ( sscanf(line,"%s%d%d%s",S->data_basen,&S->frame_start,&S->frame_end,S->data_fn) < 4)  
    /* crappy input format */
    ERR_return2("ctlfile line does not conform to spec",line);  

  S->data_size = S->frame_end - S->frame_start + 1;
  if (S->data_size <= 0)
    ERR_return("ctl item has end before start");
  if (S->frame_start < 0)
    ERR_return("ctl item has a negative start value");

  Free(S->Kin);
  Free(S->Ksmooth);
  Free(S->frame_segp);


  S->Kin = (float *)calloc(S->data_size,sizeof(float));
  S->Ksmooth = (float *)calloc(S->data_size,sizeof(float));
  S->frame_segp = (int *)calloc(S->data_size,sizeof(int));
  S->num_segp = 0;


  return(0);
}

int load_cep (suitcase_t *S)
/* load the cepstral vector as pointed to */
{
  char fn[SZfn],dummy[SZfn];
  int toread,sizeread;

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
  
  toread = (S->frame_end) - (S->frame_start) + 1;
  sizeread = ((S->mfc_fp)->size_vector)*((S->mfc_fp)->size_el);

  S->data = 0;
  S->data = (float *)calloc(sizeread,toread);

  if (S->data == NULL)
    {
      sprintf(dummy,"%d",sizeread * toread);
      ERR_return2("couldn't allocate space for data buffer",dummy);
    }

  if (mfc_fread(S->data,toread,S->mfc_fp) != toread)
    {
      sprintf(dummy,"%d",toread);
      ERR_return2("couldn't read the number of frames from the file",dummy);
    }

  mfc_fclose(S->mfc_fp);

  return(0);
}

int report (suitcase_t *S)
/* report the resuilts */
{
  int i;

  ERR_reset("report on findings");

  fprintf(S->report_fp,"%s %d %d %s",S->data_basen,S->frame_start,S->frame_end,S->data_fn);
  for (i=0;i<S->num_segp;i++)
    fprintf(S->report_fp," %d",S->frame_segp[i]);
  fprintf(S->report_fp,"\n");
  fflush(S->report_fp);

  return(0);
}
