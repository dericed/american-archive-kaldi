/* 
Tue Jun  4 08:22:26 EDT 1996 (msiegler)

Gaussian Classifier for determining the best class for entire
utterances from a control file.  Uses ML estimate.

Compute best classes for sets of files.
*/

#include "UTT_gauss_class.h"
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <stdlib.h>

#ifdef DEBUG
int Gctr;
#endif

int main (int argc,char *argv[])
{
  char *errors[] = 
    {
      "none",
      "Parse error",
      "Trouble with classes file",
      "Trouble with distributions",
      "Trouble with classification",
      };

  suitcase_t S;

#ifdef DEBUG
  extern int Gctr;
  Gctr = 0;
#endif

  S.argc = argc;
  S.argv = argv;
  /* added by JGF */
  S.data = (float *)0;

#define quit(x) {fprintf(stderr,"%s: ABORT %s\n",argv[0],errors[x]); exit(-1); }
  
  if(parse_args(&S))
    quit(1);
	   
  if(get_files(&S))
    quit(2);

  if(load_dists(&S))
    quit(3);

  if (classify_all(&S))
    quit(4);

#ifdef DEBUG
  if (S.verbose)
    fprintf(stderr,"Called %d Gaussians\n",Gctr);
#endif

  return(0);
}

int parse_args (suitcase_t *S)
{
  int argp=1,argerr=0;

  /* set defaults */


  S->verbose = 0;
  strcpy(S->data_fn,"test.ctl");
  *(S->data_ext) = 0;
  *(S->data_dir) = 0;
  strcpy(S->class_fn,"test.classes");
  strcpy(S->report_fn,"test.report");
  S->data_dim = 0;
  *(S->dist_dir) = 0;

  /* read args */

  if (S->argc < 2)
    argerr =1;

  while (argp < S->argc && !argerr)
    {
      if (S->argv[argp][0] == '-')
	switch(S->argv[argp][1])
	  {
	  case 'i':
	    if (argp+1 < S->argc)
	      {
		switch (S->argv[argp][2])
		  {
		  case '\0':
		    strcpy(S->data_fn,S->argv[++argp]);
		    break;
		  case 'd':
		    strcpy(S->data_dir,S->argv[++argp]);
		    break;
		  case 'e':
		    strcpy(S->data_ext,S->argv[++argp]);
		    break;
		  default:
		    argerr=1;
		    break;
		  }
	      }
	    else
	      argerr=1;
	    break;
	  case 'c':
	    if (argp+1 < S->argc)
	      {
		strcpy(S->class_fn,S->argv[++argp]);
		break;
	      }
	    else
	      argerr=1;
	    break;
	  case 'd':
	    if (argp+1 < S->argc)
	      strcpy(S->dist_dir,S->argv[++argp]);
	    else
	      argerr=1;
	    break;
	  case 'r':
	    if (argp+1 < S->argc)
	      strcpy(S->report_fn,S->argv[++argp]);
	    else
	      argerr=1;
	    break;
	  case 'v':
	    S->verbose = 1;
	    break;
	  default:
	    argerr = 1;break;
	  }
      else
	argerr = 1;

      argp++;
    }
  
  if (argerr)
    {
      fprintf(stderr,
	      "Utterance-based Gaussian Classifier\n"
	      "\tUsage: %s\n\t-i <ctl_file> -id <mfc_dir> -ie <mfc_ext> \n"
	      "\t-c <class_file> -r <report_file> \n"
	      "\t[-d <dist dir>] [-v (verbose)]\n"
	      "\t <ctl_file> is of form <mfcfile> <stframe> <eframe> <id>\n"
	      "\tCompiled %s %s\n",
	      S->argv[0],
	      __DATE__,__TIME__);
      return(-1);
    }

  if (S->verbose)
    {
      fprintf(stderr,"%s",S->argv[0]);
      for (argp=1;argp<S->argc;argp++)
        fprintf(stderr," %s",S->argv[argp]);
      fprintf(stderr,"\n\n");

      fprintf(stderr,"%s: will be verbose\n",S->argv[0]);
    }

  return(0);
}



/* 

   class file is of format
   
   <number independent classifiers>
   <classifier #1: NAME (string)> <classifier #1: number of classes>
   <classifier #1: class #1: NAME (string)> <#1,#1 prior prob> <#1,#1 distribution file>
   <classifier #1: class #2: NAME (string)> <#1,#2 prior prob> <#1,#2 distribution file>
   .
   .
   <classifier #2: NAME (string)> <classifier #2: number of classes>
   <classifier #2: class #1: NAME (string)> <#2,#1 prior prob> <#2,#1 distribution file>
   <classifier #2: class #2: NAME (string)> <#2,#2 prior prob> <#2,#2 distribution file>
   .
   .


*/

#define szITEM 1024
int get_files(suitcase_t *S)
{
  FILE *fp;
  int g,d;
  float sum;

  char dummyS[szITEM];

  if ( (fp = fopen(S->class_fn,"r"))== NULL)
    {
      fprintf(stderr,"%s: could not read the class file\n",S->argv[0]);
      return(-1);
    }

  fscanf(fp,"%d",&(S->num_groups));
  if ( S->num_groups < 1 || S->num_groups > MAX_groups)
    {
      fprintf(stderr,"%s: inappropriate number of groups %d\n",S->argv[0],S->num_groups);
      return(-1);
    }
  if (S->verbose)
    fprintf(stderr,"%s: %d groups were found\n",S->argv[0],S->num_groups);

  for (g=0;g<S->num_groups;g++)
    {
      fscanf(fp,"%s",dummyS);
      S->group_names[g] = strdup(dummyS);
      if (S->verbose)
	fprintf(stderr,"%s: class #%d: %s\n",S->argv[0],g,S->group_names[g]);

      fscanf(fp,"%d",&(S->num_dists[g]));
      if (S->num_dists[g] < 1 || S->num_dists[g] > MAX_dists)
	{
	  fprintf(stderr,"%s: inappropriate number of distributions %d\n",S->argv[0],S->num_dists[g]);
	  return(-1);
	}	

      S->dist_names[g] = (char **)calloc(S->num_dists[g],sizeof(char *));
      S->dist_fnames[g] = (char **)calloc(S->num_dists[g],sizeof(char *));
      S->dist_prior[g] = (float *)calloc(S->num_dists[g],sizeof(float));
      S->dist[g] = (class_t *)calloc(S->num_dists[g],sizeof(class_t));

      sum = 0.0;
      for (d=0;d<S->num_dists[g];d++)
	{
	  fscanf(fp,"%s",dummyS);
	  S->dist_names[g][d] = strdup(dummyS);
	  if (S->verbose)
	    fprintf(stderr,"%s: class #%d: dist #%d %s\n",S->argv[0],g,d,S->dist_names[g][d]);
	  
	  fscanf(fp,"%f",&(S->dist_prior[g][d]));
	  if (S->verbose)
	    fprintf(stderr,"%s: class #%d: dist #%d %f\n",S->argv[0],g,d,S->dist_prior[g][d]);
	  sum = sum + S->dist_prior[g][d];

	  fscanf(fp,"%s",dummyS);
	  S->dist_fnames[g][d] = strdup(dummyS);
	  if (S->verbose)
	    fprintf(stderr,"%s: class #%d: dist #%d file:%s\n",S->argv[0],g,d,S->dist_fnames[g][d]);	  
	}

      for (d=0;d<S->num_dists[g];d++)
	S->dist_prior[g][d] /= sum;

    }

  fclose (fp);

  return(0);
}

/* load the distribution files */
int load_dists(suitcase_t *S)
{
  FILE *fp;
  int g,d;

  for (g=0;g<S->num_groups;g++)
      for (d=0;d<S->num_dists[g];d++)
	if(load_a_dist(&(S->dist[g][d]),S->dist_fnames[g][d],S))
	  return(-1);

  return(0);
}


/*
Distribution file format
<num mix> <data dim>
(foreach mixture)
<mix w #i>
<mean #i,#1> <mean #i,#2> ... <mean #i,#datadim>
<var #i,#1> <var #i,#2> ... <var #i,#datadim>
*/
int load_a_dist(class_t *c,char *fname,suitcase_t *S)
{
  char file[szFN];
  int m,l;
  float dummyF;
  FILE *fp;

  if (*(S->dist_dir))
    sprintf(file,"%s/%s",S->dist_dir,fname);
  else
    strcpy(file,fname);

  if ( (fp = fopen(file,"r"))==NULL)
    return(-1);
  
  
  fscanf(fp,"%d",&c->num_mix);
  if (c->num_mix < 1 || c->num_mix > MAX_mix)
    return(-1);

  c->means = (float **)calloc(c->num_mix,sizeof(float *));
  c->vars = (float **)calloc(c->num_mix,sizeof(float *));
  c->hack = (float **)calloc(c->num_mix,sizeof(float *));
  c->weights = (float *)calloc(c->num_mix,sizeof(float));

  fscanf(fp,"%f",&dummyF);
  if (!S->data_dim)
    S->data_dim = dummyF;
  else if(S->data_dim != dummyF)
    return(-1);

  for (m=0;m<c->num_mix;m++)
    {
      fscanf(fp,"%f",&(c->weights[m]));

      c->means[m] = (float *)calloc(S->data_dim,sizeof(float));
      for (l=0;l<S->data_dim;l++)
	fscanf(fp,"%f",&(c->means[m][l]));

      c->vars[m] = (float *)calloc(S->data_dim,sizeof(float));
      c->hack[m] = (float *)calloc(S->data_dim,sizeof(float));
      for (l=0;l<S->data_dim;l++)
	{
	  fscanf(fp,"%f",&(c->vars[m][l]));
	  c->hack[m][l] = 1/(sqrt(2 * M_PI * c->vars[m][l]));
	}
    }
  fclose(fp);


  return(0);
}

int classify_all(suitcase_t *S)
{

  int f,nfiles;
  FILE *fpctl,*fpreport;
  char file[szFN];
#define SZline 1024
  char line[SZline];

#define ERR(x) fprintf(stderr,"%s: %s\n",S->argv[0],x)

  if ( (fpctl = fopen(S->data_fn,"r")) == NULL)
    {
      ERR("could not open the control file");
      return(-1);
    }

  nfiles = 0;
  while (!feof(fpctl) && fgets(line,SZline,fpctl) !=NULL){
    nfiles++;
    /* added by Jon Fiscus */
    if (*(line + strlen(line) - 1) != '\n')
      ERR_return2("ctlline line too large for buffer ",line);
  }

  rewind(fpctl);

  if (S->verbose)
    fprintf(stderr,"%s: %d files found\n",S->argv[0],nfiles);

  if ( (fpreport = fopen(S->report_fn,"w")) == NULL)
    {
      ERR("could not open the report file");
      return(-1);
    }

  /* for each file */
  for (f=0;f<nfiles;f++)
    {
      fscanf(fpctl,"%s%d%d%s",S->file,&S->frame_start,&S->frame_end,S->basen);

      if (load_cep(S))
	return(-1);

      if (S->verbose)
	fprintf(stderr,"Working on %s\n",S->basen);

      compute_likes(S);      
      pick_winner(S);
      report(S,fpreport);

      free_data(S);
    }
  
  fclose(fpctl);
  fclose(fpreport);
  return(0);
}


int compute_likes(suitcase_t *S)
{
  float like;
  int g,d,n;

  for (g=0;g<S->num_groups;g++)
    {
      S->dist_like[g] = (float **)calloc(S->num_dists[g],sizeof(float *));
      for (d=0;d<S->num_dists[g];d++)
	{
	  S->dist_like[g][d] = (float *)calloc(S->data_size,sizeof(float));
	  if (S->verbose)
	    fprintf(stderr,"computing likelihoods for %s: %s\n",
		    S->group_names[g],S->dist_names[g][d]);
	  for (n=0;n<S->data_size;n++)
	    {
	      (S->dist_like[g][d])[n] = (float)log((double)S->dist_prior[g][d]);
	      like = likelihood_fast(&(S->dist[g][d]),S->data + n * S->data_dim,S->data_dim);
	      (S->dist_like[g][d])[n] += like;
	    }
	}
    }  

}

typedef struct sort_s
{
  int ptr;
  float like;
} sort_t;


extern int cmp_likes(sort_t *s1,sort_t *s2)
{
  if (s1->like < s2->like)
    return (1);
  else if (s1->like > s2->like)
    return (-1);
  else
    return (0);

}

int pick_winner(suitcase_t *S)
{
  int g,d,fr;
  sort_t sort[MAX_dists];

  for (g=0;g<S->num_groups;g++)
    {
      for (d=0;d<S->num_dists[g];d++)
	{
	  /* get a list of likelihoods for the classes */
	  sort[d].ptr=d;
	  sort[d].like=0.0;
	  
	  /* collect the window averaged likelihoods */
	  
	  for (fr=0;fr < S->data_size;fr++)
	    sort[d].like += (S->dist_like[g][d])[fr];
	  sort[d].like /= (float)S->data_size;
	}
      
      /* sort the items */
      qsort(sort,S->num_dists[g],sizeof(sort_t),cmp_likes);

      S->decision_conf[g] = sort[0].like-sort[1].like;
      S->decision[g] = sort[0].ptr;
    }

}

int free_data(suitcase_t *S)
{
  int g,d;
  for (g=0;g<S->num_groups;g++)
    {
      for (d=0;d<S->num_dists[g];d++)
	free(S->dist_like[g][d]);
      free(S->dist_like[g]);
    }
  free(S->data);
  S->data = NULL;

}



int report(suitcase_t *S,FILE *fp)
{
  int g;
  fprintf(fp,"%s ",S->basen);

  for (g=0;g<S->num_groups;g++)
    fprintf(fp," %s %.4f",S->dist_names[g][S->decision[g]],S->decision_conf[g]);
  fprintf(fp,"\n");
  fflush(fp);
  
  return(0);
}

