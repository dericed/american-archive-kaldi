/*

mfc_io.c

Input and Output routines for working with cepstra files.
Works with old and new formats, any byteorder, streams if necessary

Functions:

 mfc_fopen
 mfc_feof
 mfc_fclose
 mfc_fseek
 mfc_fread
 mfc_fwrite
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>

/* added by JGF */
#include <sys/fcntl.h>

#include "mfc_io.h"

#ifndef SEEK_SET
#define SEEK_SET 0
#endif

void SWAP(size_t b,void *x)
{
  if (b==2)
    SWAP16((int16 *)x);
  else if (b==4)
    SWAP32((int32 *)x);
}

/* swap a bunch a bytes */
void SWAPmany(size_t b,void *x,int32 num)
{
  int n;

  if (b==2)
    for (n=0;n<num;n++)
      SWAP16((int16 *)x+n);
  else if (b==4)
    for (n=0;n<num;n++)
      SWAP32((int32 *)x+n);
}


mfcFILE *mfc_fopen(char *filename,char *mode)
/* like fopen */
{
  int32 ifd;
  struct stat fstatbuf;
  /* for fstat */

  mfcFILE *temp;

  temp = (mfcFILE *)malloc(sizeof(mfcFILE));

  /* return NULL as bogus just as if file doesn't exist */
  if (temp == NULL)
    return(NULL);

  /* set the counters to zeros */
  temp->size_vector=0;
  temp->size_el=0;
  temp->ptr_vector=0;
  temp->avail_vectors=0;

  temp->hdr_type=header_NONE;
  temp->size_header=0;

  temp->size_file=0;
  temp->endian=ENDIAN_UNKNOWN;
  
  
  /* do an fstat if possible */
  
  if ((ifd = open(filename,O_RDONLY)) >=0)
    /* try to read the filename */
    {
      fstat(ifd,&fstatbuf);

      temp->size_file = fstatbuf.st_size;


      close(ifd);
    }

  /* use fopen just as you would yourself */
  temp->stream = fopen(filename,mode);

  if (temp->stream == NULL)
    return(NULL);

  return(temp);
}

int32 mfc_fread_hdr(mfcFILE *fp,int32 which_hdr)
{
  int32 file_hdr;
  int32 file_hdr_shouldbe;

  if (which_hdr == header_SPHINX)
    /* forces Big-Endian, first input is an int32 containing number of floats */
    {
      /* by def, sphinx features are float32 */
      fp->size_el = sizeof(float32);

      /* we epexct fp->size_vector to be > 0, otherwise use the default */      
      if (!fp->size_vector)
	fp->size_vector = mfc_VECLEN_def;

      /* this is the default sphinx header */
      fp->hdr_type = header_SPHINX;
      fp->size_header = sizeof(int32);

      /* so go get the header */
      if (fread(&file_hdr,sizeof(int32),1,fp->stream) != 1)
	return(-1);

      /* sphinx header is just the number of floats to expect
	 We expect Big Endian, but we autodetect if possible here */

      /* can we determine the size of the file? */
      if (fp->size_file) /* autodetect byteorder */
	{
	  fp->endian = ENDIAN_NATIVE;
	  file_hdr_shouldbe = (fp->size_file - fp->size_header) / fp->size_el;

	  /* header and filesize don't jive, try swapping */
	  if (file_hdr != file_hdr_shouldbe)
	    {
	      SWAP32(&file_hdr);
	      fp->endian = ENDIAN_NONNATIVE;
	    }
	    
	  /* if they still don't jive, we must ignore the header! */
	  if (file_hdr != file_hdr_shouldbe)
	    file_hdr = 0;
	}
      else /* filesize unknown => use Big Endian and trust the info */
	{
	  fp->endian = ENDIAN_BIG;
	  if (ENDIAN_NATIVE == ENDIAN_LITTLE)
	    SWAP32(&file_hdr);
	}

      /* this is how much stuff we can read */
      fp->avail_vectors = file_hdr / fp->size_vector;
    }

  return(0);
}

int32 mfc_fwrite_hdr(mfcFILE *fp,int32 which_hdr)
{
  int32 file_hdr;

  if (which_hdr == header_SPHINX)
    {
      fp->endian = ENDIAN_BIG;
      fp->size_el = sizeof(float32);

      /* we epexct fp->size_vector to be > 0, otherwise use the default */      
      if (!fp->size_vector)
	fp->size_vector = mfc_VECLEN_def;

      /* sphinx header is number of floats to follow */
      file_hdr = fp->avail_vectors * fp->size_vector;

      if (ENDIAN_NATIVE != ENDIAN_BIG)
	SWAP32(&file_hdr);

      if(fwrite(&file_hdr,sizeof(int32),1,fp->stream) != 1)
	return(-1);

      if (ENDIAN_NATIVE != ENDIAN_BIG)
	SWAP32(&file_hdr);
    }

  return(0);
}

int32 mfc_fclose(mfcFILE *fp)
{
  int32 err;

  err = fclose(fp->stream);
  free(fp);

  return(err);
}

int32 mfc_feof(mfcFILE *fp)
{
  return(feof(fp->stream));
}

/* seek a particular vector out */
int32 mfc_fseek(mfcFILE *fp,int32 ptr_vector)
{
  int32 err;

  err = 
    fseek(fp->stream,ptr_vector * fp->size_vector * fp->size_el + fp->size_header,SEEK_SET);
  
  if (!err)
    /* we did it, update the internal pointer */
    fp->ptr_vector = ptr_vector;
  else
    /* return to previous place (good way to be) */
    fseek(fp->stream,fp->ptr_vector * fp->size_vector * fp->size_el + fp->size_header,SEEK_SET);

  return(err);
}

int32 mfc_fread(void *buffer,int32 ask_vecs,mfcFILE *fp)
/* a lot like fread */
{
  /* how many vectors did we read */
  int32 did_vecs,n,n2;
  
  if (!ask_vecs)
    return(0);

  /* each item is one vector, we want a certain number */
  did_vecs = fread(buffer,fp->size_vector * fp->size_el,ask_vecs,fp->stream);

  /* swap if necessary */
  if (fp->endian != ENDIAN_NATIVE)
    SWAPmany(fp->size_el,buffer,fp->size_vector*did_vecs);
  
  /* update the internal pointer (works if we failed too!) */
  fp->ptr_vector += did_vecs;
    
  return(did_vecs);
}


int32 mfc_fwrite(void *buffer,int32 ask_vecs,mfcFILE *fp)
/* a lot like fwrite */
{
  /* how many vectors did we write */
  int32 did_vecs,n,n2;
  
  if (!ask_vecs)
    return(0);

  /* swap if necessary */
  if (fp->endian != ENDIAN_NATIVE)
    SWAPmany(fp->size_el,buffer,fp->size_vector*ask_vecs);


  /* each item is one vector, we want a certain number */
  did_vecs = fwrite(buffer,fp->size_vector * fp->size_el,ask_vecs,fp->stream);

  /* update the internal pointer (works if we failed too!) */
  fp->ptr_vector += did_vecs;
    
  return(did_vecs);
}




