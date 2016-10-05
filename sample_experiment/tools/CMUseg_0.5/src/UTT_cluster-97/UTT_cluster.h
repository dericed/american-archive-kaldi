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
#define MAX_feat 40

#define MAX(x,y) ((x)>(y)?(x):(y))
#define MIN(x,y) ((x)>(y)?(y):(x))



/* Statistics for a cluster, including # points  */
typedef struct cluster_s
{
  float mean[MAX_feat];
  float var[MAX_feat];
  int   count;
} cluster_t;

/* Statistics for a member, including # points, and cluster it belongs to */
typedef struct member_s
{
  char *name;
  float mean[MAX_feat];
  float var[MAX_feat];
  int   count;
  int   cluster;
  int   cluster2;
} member_t;

typedef struct suitcase_s
{
  int argc;
  char **argv;
  
  mfcFILE *mfc_fp;
  
  int verbose;

  char file[szFN];

  char basen[szFN];

  char data_fn[szFN];
  char data_dir[szFN];
  char data_ext[szFN];

  char report_fn[szFN];

  int nclusters;
  cluster_t *clusters;

  int nmembers;
  member_t *members;

  /* what is the maximum permissible distance between centroid and cluster item */
  float dist_thr;

  /* minimum amount of data in a cluster to consider the VAR estimate reasonable */
  int cluster_mincount;

  /* maximum allowed in a cluster, don't add any if this big */
  int cluster_maxcount;


  float *data;
  int data_dim;
  int data_size;
  int frame_start;
  int frame_end;
  
  

} suitcase_t;




#define SZ_err 4096
char ERR_problem[SZ_err],ERR_param[SZ_err],ERR_function[SZ_err];
#define ERR_return(x)   {strcpy(ERR_problem,x);*ERR_param=0;return(-1);}
#define ERR_return2(x,y){strcpy(ERR_problem,x);strcpy(ERR_param,y);return(-1);}
#define ERR_reset(x)    {strcpy(ERR_function,x);*ERR_problem=0;*ERR_param=0;}
#define ERR_bomb        {fprintf(stderr,"ERROR in %s:\n %s\n %s\n",ERR_function,ERR_problem,ERR_param);exit(-1);}
#define ERR_trap        {if(*ERR_problem) ERR_bomb;}

#define Free(x) {if(x!=NULL)free(x);}


int main(int,char *[]);
int parse_args (suitcase_t *);
int load_files(suitcase_t *);
int cluster(suitcase_t *);
int report(suitcase_t *);


int get_member_stat(suitcase_t *,int);
int init_clusters(suitcase_t *,float **);
int update_cluster_distances(suitcase_t *,float **);

float dist_member2cluster(suitcase_t *,int,cluster_t *);
int stat_combine_clusters(suitcase_t *,int,int,cluster_t *);
float dist_combine_clusters(suitcase_t *,int,int);
int combine_clusters(suitcase_t *,int,int);
int reset_cluster_distances(suitcase_t *,float **,int,int);



