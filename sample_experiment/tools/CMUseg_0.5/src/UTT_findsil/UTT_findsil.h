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
  int report_sildetail;

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
  int num_windows;

  int data_dim;
  int data_size;

  float silence_thresh;
  /* difference between mean over window and max over region */

  float silence_dn;
  /* difference between max and min over window */

  int silence_search_region;
  int silence_search_region_f;
  /* how wide to search for window */

  int silence_window;
  /* how long a silence to search for */
  int silence_window_outer;
  /* comparison window outside the inner window */

  float *snr;
  float *dn;


  int max_length;
  int min_length;

/* strict vector */
  int *boundaries_point;
  int *boundaries_fake;
  int num_boundaries;

  int *silences_point;
  int *silences_found;
  float *silences_dn;
  float *silences_depth;

  int num_silences;

} suitcase_t;

/* error trapping routines */

#define SZ_err 4096
char ERR_problem[SZ_err],ERR_param[SZ_err],ERR_function[SZ_err];
#define ERR_return(x)   {strcpy(ERR_problem,x);*ERR_param=0;return(-1);}
#define ERR_return2(x,y){strcpy(ERR_problem,x);strcpy(ERR_param,y);return(-1);}
#define ERR_reset(x)    {strcpy(ERR_function,x);*ERR_problem=0;*ERR_param=0;}
#define ERR_bomb        {fprintf(stderr,"ERROR in %s:\n %s\n %s\n",ERR_function,ERR_problem,ERR_param);exit(-1);}
#define ERR_trap        {if(*ERR_problem) ERR_bomb;}

int main (int,char *[]);
int parse_args (suitcase_t *);
int prep_files(suitcase_t *);
int get_nextfile(suitcase_t *);
int load_cep (suitcase_t *);

int find_boundaries(suitcase_t *);
int find_silences(suitcase_t *);
int find_a_silence(suitcase_t *,int,int,float *,float *,int *);

int report (suitcase_t *);


int countwords(char *);
char *nextword(char *);
