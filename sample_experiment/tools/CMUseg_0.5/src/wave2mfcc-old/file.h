/*

Thu Aug 8 1996 msiegler
Matthew A. Siegler

 Data structures for the program.
 Building filenames, opening/closeing, partial files.

*/


/* don't do this twice! */
#ifndef _h_file
#define _h_file


#include <sp/sphere.h>
#include <mfc_io.h>

/* maximum length of a filename including path */
#define SZfn 1024
#define SZline 4096


/* possible input formats acceptable */
#define FORMAT_IN_big     0
#define FORMAT_IN_little  1
#define FORMAT_IN_ulaw    2
#define FORMAT_IN_nist    3

/* pick a byteorder that is native to this machine */

#if defined(ALPHA) || defined(ALPHA_OSF1) || defined(alpha_osf1) || defined(__alpha) || defined(mips) || defined(intel) || defined(INTEL)
#define FORMAT_native FORMAT_IN_little 
#else
#define FORMAT_native FORMAT_IN_big
#endif


typedef struct parmfile_s
{
  FILE *fp_in;
  SP_FILE *sp_in; /* for NIST sphere files */

  mfcFILE *fp_out; /* for mfc files */
  FILE *fp_ctl;

  /* single file: input and output name */
  char fn_in[SZfn];
  char fn_out[SZfn];

  /* batch mode: control file, extensions, directories */
  char *fn_ctl;
  char *ext_in;
  char *ext_out;
  char *dir_in;
  char *dir_out;

  /* base filenames */
  char bn_in[SZfn];
  char bn_out[SZfn];

  /* either: begin and end times to convert */
  float time_begin,time_end;
  int32 samples_begin,samples_end;

  /* how big are the samples in # bytes */
  int32 bytes_per_sample;

  /* how many are in this file (== 0 for a device or not found) */
  int32 samples_in_total;

  /* how many do we plan to read (==0 for don't know) */
  int32 samples_in_gonnaread;
  int32 vectors_out_gonnawrite;

  /* dup from the mfc parameters so we can fseek */
  int32 rate_sample;

  /* pointers to the buffer, only one malloc is done for in0,
     the others are inside of it */
  int16 *buf_in0;
  int16 *buf_in_prev;
  int16 *buf_in;

  /* buffer sizes and reads

     The deal is this:
     Specify the size of the buffer in size_bufin,
     The minimum amount you want read before return in size_bufin_minreaed,
     
     It returns, the size of the last read in size_bufin_lastread
     And the total read sofar in this file in size_bufin_totread
     */

  int32 size_bufin_minread;    
  int32 size_bufin;
  int32 count_bufin_lastread;
  int32 count_bufin_totread;
  int32 count_bufin_numreads;


  /* WORKING buffer for processing input files */
  int16 *buf_work;
  int32 count_bufwork_lastget;

  int32 count_numfiles;

  /* one of the file format types for the input */
  int32 format_in;


  float **buf_out;


    
} parmfile_t;

/* macros */
#define SWAP16(x)        *(x) = ((0xff & (*(x))>>8) | (0xff00 & (*(x))<<8))
#define SWAP32(x)        *(x) = ((0xff & (*(x))>>24) | (0xff00 & (*(x))>>8) |\
                        (0xff0000 & (*(x))<<8) | (0xff000000 & (*(x))<<24))

#define SWAPF(x)        SWAP32((int32 *) x)

/* minimum value (why not standard?) */
#ifndef MIN
#define MIN(x,y) ((x)>(y) ? (y):(x))
#endif

#ifndef MAX
#define MAX(x,y) ((y)>(x) ? (y):(x))
#endif

int32 file_getnext(parmfile_t *F);
int32 file_readblock(parmfile_t *F);
void block_swaplin(int16 *buf,int howmany);
void block_ulaw2lin(int16 *buf,int howmany);
int countwords(char *line);
char *nextword(char *here);

/* for bomb */
char ERR_function[1024];
char ERR_problem[1024];
char ERR_param[1024];

#define ERR_return(x)   {strcpy(ERR_problem,x);*ERR_param=0;return(-1);}
#define ERR_return2(x,y){strcpy(ERR_problem,x);strcpy(ERR_param,y);return(-1);}
#define ERR_reset(x)    {strcpy(ERR_function,x);*ERR_problem=0;*ERR_param=0;}

#endif








