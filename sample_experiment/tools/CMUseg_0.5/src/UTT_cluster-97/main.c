/* 

Tue Nov 12 14:53:35 EST 1996 msiegler

Cluster utterances together based on second-order statistics,
using the Cross-Entropy measure.

Technique for clustering:
1. Every member has its own cluster
2. Compute the resultant distortion of combining any two clusters (remember others!)
3. Pick the pair with the lowest distortion
4. If it is less than the threshold, combine, go to step 2.
5. If not, quit

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
  S->cluster_mincount = 0;
  S->cluster_maxcount = 0; /* no restriction */

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
	      S->cluster_mincount = atoi(S->argv[++argp]);
	    else
	      argerr=1;
	    break;

	  case 'M':
	    if (argp+1 < S->argc)
	      S->cluster_maxcount = atoi(S->argv[++argp]);
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
	      "\t[-m (cluster minimum count for VAR trust def=none)]\n" 
	      "\t[-M (cluster maximum def=none)]\n" 
	      "\t[-v (verbose)]\n"
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
  int i,j,n,ct,found,first,best_i,best_j,Step=1;
  int lastcount=0;
   
  float dist,best_dist,lastavg=0;
  float **cdist;

  S->nclusters = S->nmembers;
  cdist = (float **)alloc2d(S->nclusters,S->nclusters,sizeof(float));

  if(S->verbose)
    fprintf(stderr,"Clustering\n");
  init_clusters(S,cdist);

  do
    {
      found = 0;

      update_cluster_distances(S,cdist);
      

      /* get the best distance between two clusters */
      first = 1;
      for (i=0;i<S->nclusters;i++)
	for (j=0;j<S->nclusters;j++)
	  if (i!=j)
	    {
	      dist = cdist[i][j];
	      if (dist >=0 && (first || best_dist > dist))
		{
		  best_dist = dist; best_i = i; best_j = j; first = 0;
		}
	    }

      /* output average cluster distance */
      if (S->verbose)
	{
	  j=0; dist=0;
	  for (i=0;i<S->nclusters;i++)
	    if (S->clusters[i].count && cdist[i][i]>=0)
	      {
		/* cdist[i][i] is maximum distance between member and cluster centroid */
		dist += cdist[i][i];
		j++;
	      }
	  if (j > 0)
	    {
	      lastcount = j;
	      lastavg = dist/j;
	      fprintf (stderr,"step %d: Average cluster distance = %.3f (%d clusters)\n",Step++,lastavg,lastcount);
	    }
	}      


      /* if we actually found a pair of clusters we can work with */
      if (!first)
	{
	  if (S->verbose)
	    fprintf(stderr,"Clusters %d and %d have the best distance %.3f\n",best_i,best_j,best_dist);

	  if (best_dist <= S->dist_thr)
	    {
	      found = 1;

	      n = combine_clusters(S,best_i,best_j);

	      if (n >=0)
		{
		  if (S->verbose)
		    fprintf(stderr,"Combined clusters %d and %d into %d\n",best_i,best_j,n);

		  reset_cluster_distances(S,cdist,best_i,best_j);
		}
	      /* failed because of something bogus in the clusters RARE */
	      else
		if (S->verbose)
		  fprintf(stderr,"Couldn't combine clusters %d and %d\n",best_i,best_j);
	    }
	}
      else
	if (S->verbose)
	  fprintf(stderr,"Couldn't find two clusters to check\n");
    }
  while (found);


  if (S->verbose && lastcount)
    fprintf (stderr,"TOTAL: %d clusters, %.4f avg distance\n",lastcount,lastavg);


  free2d(cdist);
  return(0);
}


int report(suitcase_t *S)
{
  FILE *fpreport;
  int f,f2,c;

  ERR_reset("Generating a report");

  if ( (fpreport = fopen(S->report_fn,"w")) == NULL)
    ERR_return2("could not open the report file",S->report_fn);


  /* renumber the clusters in consecutive integers */

  c = 0;
  for (f=0;f<S->nclusters;f++)
    {
      if (S->clusters[f].count)
	{
	  for (f2 = 0;f2<S->nmembers;f2++)
	    if (S->members[f2].cluster == f)
	      S->members[f2].cluster2 = c;
	  c++;
	}
    }


  for (f=0;f<S->nmembers;f++)
    fprintf(fpreport,"%s %d\n",S->members[f].name,S->members[f].cluster2);

  return(0);
}

