/* 

Uday Jain + Matthew Siegler

Cluster utterances together based on second-order statistics,
using the Cross-Entropy measure.

Technique for clustering:
1. For every utterance I that has not been clustered yet
2. Look ahead, and find other utterances that satisfy:
 a. are not yet clustered either
 b. are "close" to utterance I in the cross-entropy measure
3. Until all utterances I have been examined.


*/

#include "UTT_cluster.h"


int main (int argc,char *argv[])
{
  suitcase_t S;

  S.argc = argc;
  S.argv = argv;

  if(parse_args(&S))
    ERR_bomb;

  if(load_files(&S))
    ERR_bomb;

  if(cluster(&S))
    ERR_bomb;

  if (report(&S))
    ERR_bomb;

  return(0);
}

int parse_args (suitcase_t *S)
{

  int argp=1,argerr=0;

  ERR_reset ("Parsing command line");

  /* set defaults */


  S->verbose = 0;
  strcpy(S->data_fn,"test.ctl");
  *(S->data_ext) = 0;
  *(S->data_dir) = 0;
  S->data_dim = 13;
  S->data=NULL;
  S->dist_thr = 0.0;
  S->min_clus = 0;

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
	      strcpy(S->data_fn,S->argv[++argp]);
	    else
	      argerr=1;
	    break;

	  case 'd':
	    if (argp+1 < S->argc)
	      strcpy(S->data_dir,S->argv[++argp]);
	    else
	      argerr=1;
	    break;

	  case 'e':
	    if (argp+1 < S->argc)
	      strcpy(S->data_ext,S->argv[++argp]);
	    else
	      argerr=1;
	    break;

	  case 'r':
	    if (argp+1 < S->argc)
	      strcpy(S->report_fn,S->argv[++argp]);
	    else
	      argerr=1;
	    break;

	  case 't':
	    if (argp+1 < S->argc)
	      S->dist_thr = atof(S->argv[++argp]);
	    else
	      argerr=1;
	    break;

	  case 'm':
	    if (argp+1 < S->argc)
	      S->min_clus = atoi(S->argv[++argp]);
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
	      "Cross Entropy Based Clustering\n"
	      "\tUsage: %s\n\t-i <ctl_file> -d <mfc_dir> -e <mfc_ext> \n"
	      "\t-r <report_file> [-t (distance threshold (0-1) def=%.2f)]\n"
	      "\t[-m minimum cluster size in frames def=0 (none)] [-v (verbose)]\n"
	      "\t <ctl_file> is of form <mfcfile> <stframe> <eframe> <id>\n"
	      "\tCompiled %s %s\n",
	      S->argv[0],S->dist_thr,__DATE__,__TIME__);
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



int load_files(suitcase_t *S)
{

#define SZline 1024

  float dist;
  int f,f2,nfiles;
  FILE *fpctl;
  char file[szFN];
  char line[SZline];

  ERR_reset("Loading cepstra files and computing statistics");

  if(S->verbose)
    fprintf(stderr,"Loading files\n");
  
  if ( (fpctl = fopen(S->data_fn,"r")) == NULL)
    {
      ERR_return("could not open the control file");
      return(-1);
    }

  nfiles = 0;
  while (!feof(fpctl) && fgets(line,SZline,fpctl) !=NULL)
    nfiles++;
  rewind(fpctl);

  if (S->verbose)
    fprintf(stderr,"%s: %d files found\n",S->argv[0],nfiles);

  S->nmembers = nfiles;
  S->members = (member_t *)calloc(S->nmembers,sizeof(member_t));
  if (S->members==NULL)
    ERR_return("Memory alloc error");

  /* Get member stats from the file */
  for (f=0;f<S->nmembers;f++)
    {
      fscanf(fpctl,"%s%d%d%s",S->file,&S->frame_start,&S->frame_end,S->basen);

      if (load_cep(S))
	ERR_return2("Couldn't read from",S->file);

      S->members[f].name = strdup(S->basen);
      get_member_stat(S,f);
      free(S->data);
      S->data=NULL;
    }

  fclose(fpctl);
  return(0);
}


int cluster(suitcase_t *S)
{
  int i,j;
  float dist;

  if(S->verbose)
    fprintf(stderr,"Clustering\n");

  init_clusters(S);

  for (i=0;i<S->nmembers;i++)
    S->members[i].cluster = -1;

  for (i=0;i<S->nmembers;i++)
    /* if not yet clustered */
    if (S->members[i].cluster == -1)
      for (j=0;j<S->nmembers;j++)
	if (S->members[j].cluster == -1)
	  {
	    dist = dist_member2cluster(S,i,S->clusters + j);
	    if (dist >= 0 && dist < S->dist_thr)
	      S->members[j].cluster = i;
	  }

  return(0);
}


int report(suitcase_t *S)
{
  FILE *fpreport;
  int f,c,c2,clus,found;

  ERR_reset("Generating a report");

  if ( (fpreport = fopen(S->report_fn,"w")) == NULL)
    ERR_return2("could not open the report file",S->report_fn);


  /* renumber the clusters into consecutive integers */
  /* compute total counts in each cluster */
  /* combine clusters (forward) if they are too small with other small ones */


  /* renumbering */
  c2 = 0;
  for (c=0;c<S->nmembers;c++)
    {
      found = 0;
      for (f = 0;f<S->nmembers;f++)
	if (S->members[f].cluster == c)
	  found = 1;

      if (found)
	{
	  for (f = 0;f<S->nmembers;f++)
	    if (S->members[f].cluster == c)
	      S->members[f].cluster2 = c2;
	  c2++;
	}
    }

  S->nclusters=c2;

  /* clear out cluster counts */
  for (c=0;c<S->nclusters;c++)
    S->clusters[c].count = 0;

  /* accumulate */
  for (f=0;f<S->nmembers;f++)
    S->clusters[S->members[f].cluster2].count += S->members[f].count;




  /* check for sizes */
  for (c=0;c<S->nclusters;c++)
    {
      c2 = c+1;
      while (S->clusters[c].count < S->min_clus &&
	     S->clusters[c].count > 0 &&
	     c < S->nclusters-1 &&
	     c2 <S->nclusters)
	{
	  /* found a small cluster, look for another */
	  if (S->clusters[c2].count < S->min_clus && S->clusters[c2].count > 0)
	    {

	      if (S->verbose)
		fprintf(stderr,"Combining clusters %d(%d) and %d(%d) to %d\n",
			c,S->clusters[c].count,c2,S->clusters[c2].count,c);

	      for (f=0;f<S->nmembers;f++)
		if (S->members[f].cluster2 == c2)
		  {
		    /* accumulate and reassign */
		    S->clusters[c].count += S->members[f].count;
		    S->members[f].cluster2 = c;
		  }
	      /* it's empty now (!) */
	      S->clusters[c2].count = 0;

	    }

	  /* keep looking */
	  c2++;
	}
    }


  /* renumbering */
  c2 = 0;
  for (c=0;c<S->nmembers;c++)
    {
      found = 0;
      for (f = 0;f<S->nmembers;f++)
	if (S->members[f].cluster2 == c)
	  found = 1;

      if (found)
	{
	  for (f = 0;f<S->nmembers;f++)
	    if (S->members[f].cluster2 == c)
	      S->members[f].cluster = c2;
	  c2++;
	}
    }


  for (f=0;f<S->nmembers;f++)
    fprintf(fpreport,"%s %d\n",S->members[f].name,S->members[f].cluster);

  return(0);
}

