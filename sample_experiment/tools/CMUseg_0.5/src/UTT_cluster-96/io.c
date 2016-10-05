/* input and output routines */

#include "UTT_cluster.h"


int load_cep (suitcase_t *S)
/* load the cepstral vector as pointed to */
{
  char fn[szFN],dummy[szFN];
  int toread;

  ERR_reset("load the cepstral file");
  
  if (*S->data_dir)
    sprintf(dummy,"%s/%s",S->data_dir,S->file);
  else
    strcpy(dummy,S->file);

  if (*S->data_ext)
    sprintf(fn,"%s.%s",dummy,S->data_ext);
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

  if (mfc_fread(S->data,toread,S->mfc_fp) != toread)
    {
      sprintf(dummy,"%d",toread);
      ERR_return2("couldn't read frames from file",dummy);
    }

  S->data_size = toread;

  mfc_fclose(S->mfc_fp);

  return(0);
}
