#ifndef _h_mfc_io

#include <stdio.h>

#define _h_mfc_io



typedef
struct mfcFILE_t
{
  FILE    *stream;
  int32   size_vector;   /* how many elements are in a vector */
  size_t  size_el;       /* how many bytes are in an element */
  int32   ptr_vector;    /* which vector # does the stream point at now */
  int32   avail_vectors; /* num vectors available in the file == 0 if unknown */

  int32   hdr_type;      /* what kind of header the file has */
  int32   size_header;   /* how big the header to the file is in bytes */

  int32   size_file;     /* how big the file is in bytes == 0 if unknown */
  int32   endian;        /* which byteorder does the file ave? */
}
mfcFILE;

#define header_NONE  0
#define header_SPHINX 1
#define header_SPHINX_native 2

#define ENDIAN_BIG 0
#define ENDIAN_LITTLE 1
#define ENDIAN_UNKNOWN -1

#if defined(ALPHA) || defined(ALPHA_OSF1) || defined(alpha_osf1) || defined(__alpha) || defined(mips) || defined(intel) || defined (INTEL)
#define ENDIAN_NATIVE ENDIAN_LITTLE
#define ENDIAN_NONNATIVE ENDIAN_BIG
#else
#define ENDIAN_NATIVE ENDIAN_BIG
#define ENDIAN_NONNATIVE ENDIAN_LITTLE
#endif


#define SWAP16(x)        *(x) = ((0xff & (*(x))>>8) | (0xff00 & (*(x))<<8))
/*16-bit*/

#define SWAP32(x)        *(x) = ((0xff & (*(x))>>24) | (0xff00 & (*(x))>>8) |\
                        (0xff0000 & (*(x))<<8) | (0xff000000 & (*(x))<<24))
/*32-bit*/

#define SWAPF32(x) SWAP32((int32 *)x)

void SWAP(size_t b,void *x);
void SWAPmany(size_t b,void *x,int32 num);

#define mfc_VECLEN_def 13

mfcFILE *mfc_fopen(char *,char *);
int32 mfc_fread_hdr(mfcFILE *fp,int32 which_hdr);
int32 mfc_fwrite_hdr(mfcFILE *fp,int32 which_hdr);
int32 mfc_fclose(mfcFILE *);
int32 mfc_feof(mfcFILE *);
int32 mfc_fseek(mfcFILE *,int32);
int32 mfc_fread(void *,int32,mfcFILE *);
int32 mfc_fwrite(void *,int32,mfcFILE *);

#endif



