#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <stdlib.h>

#include <mfc_io.h>

#define SZfn 1024
#define SZline 4096

typedef struct suitcase_s
{
  int argc;
  char **argv;

  int done;

  FILE *report_fp,*ctl_fp;
  mfcFILE *mfc_fp;

  int verbose;
  char ctl_fn[SZfn];
  char ctl_dir[SZfn];
  char ctl_ext[SZfn];
  char data_fn[SZfn];
  char data_basen[SZfn];

/* for partial cepstra reads */
  
  int frame_start,frame_end;

  char report_fn[SZfn];
  float *data;

  int win_len;
  int smooth_len;
  int scan_len;

  int data_dim;
  int data_size;

  float *Kin;
  float *Ksmooth;
  int num_segp;
  int *frame_segp;


  float Kthresh;
  float var_floor;

} suitcase_t;

/* error trapping routines */

#define SZ_err 4096
char ERR_problem[SZ_err],ERR_param[SZ_err],ERR_function[SZ_err];
#define ERR_return(x)   {strcpy(ERR_problem,x);*ERR_param=0;return(-1);}
#define ERR_return2(x,y){strcpy(ERR_problem,x);strcpy(ERR_param,y);return(-1);}
#define ERR_reset(x)    {strcpy(ERR_function,x);*ERR_problem=0;*ERR_param=0;}
#define ERR_bomb        {fprintf(stderr,"ERROR in %s:\n %s\n %s\n",ERR_function,ERR_problem,ERR_param);exit(-1);}
#define ERR_trap        {if(*ERR_problem) ERR_bomb;}

#define Free(x) {if(x!=NULL)free(x);x=NULL;}

int main (int,char *[]);
int parse_args (suitcase_t *);
int prep_files(suitcase_t *);
int get_nextfile(suitcase_t *);
int load_cep (suitcase_t *);

int find_segp(suitcase_t *);
int Kdist_fast(suitcase_t *);
int K_peaks(suitcase_t *);

int makewindow(float *,int);
int convolve(float *,float *,int,int,float *);

int report (suitcase_t *);


