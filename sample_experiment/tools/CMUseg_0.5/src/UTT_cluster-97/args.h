#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <stdlib.h>

#include <mfc_io.h>

#define szFN 1024
#define MAX_groups 16
#define MAX_dists 16
#define MAX_mix 256

typedef struct class_s
{
  float **means;
  float **vars;
  float **hack;
  float *weights;
  int num_mix;
} class_t;


typedef struct suitcase_s
{
  int argc;
  char **argv;
  
  mfcFILE *mfc_fp;

  int verbose;

  char fncep[szFN];
  char file[szFN];
  char data_fn[szFN];
  char data_dir[szFN];
  char data_ext[szFN];

  char class_fn[szFN];
  char report_fn[szFN];
  char dist_dir[szFN];
  char basen[szFN];

  float *data;

  int num_groups;
  char *group_names[MAX_groups];
  int num_dists[MAX_groups];

  char **dist_names[MAX_groups];
  char **dist_fnames[MAX_groups];
  float *dist_prior[MAX_groups];
  class_t *dist[MAX_groups];
  float **dist_like[MAX_groups];

  int data_dim;
  int data_size;


  int frame_start;
  int frame_end;

/* vector for each group */
  float decision_conf[MAX_groups]; 
  int decision[MAX_groups];

} suitcase_t;

#define SZ_err 4096
char ERR_problem[SZ_err],ERR_param[SZ_err],ERR_function[SZ_err];
#define ERR_return(x)   {strcpy(ERR_problem,x);*ERR_param=0;return(-1);}
#define ERR_return2(x,y){strcpy(ERR_problem,x);strcpy(ERR_param,y);return(-1);}
#define ERR_reset(x)    {strcpy(ERR_function,x);*ERR_problem=0;*ERR_param=0;}
#define ERR_bomb        {fprintf(stderr,"ERROR in %s:\n %s\n %s\n",ERR_function,ERR_problem,ERR_param);exit(-1);}
#define ERR_trap        {if(*ERR_problem) ERR_bomb;}

#define Free(x) {if(x!=NULL)free(x);}



int main (int,char *[]);
int parse_args (suitcase_t *);
int get_files (suitcase_t *);
int load_dists (suitcase_t *);
int load_a_dist(class_t *,char *,suitcase_t *);
int load_cep (suitcase_t *);
int classify_all (suitcase_t *);
int compute_likes (suitcase_t *);
int pick_winner (suitcase_t *);
int free_data (suitcase_t *);
int report ();

float likelihood_fast (class_t *c,float *data,int data_dim);
int win_class (suitcase_t *);




/* error trapping routines */



