#include "UTT_cluster.h"

/* get mean & var of a set of data */
int get_member_stat(suitcase_t *S,int n)
{
  int i,j,off;
  member_t *m;
  
  m = S->members + n;
  m->count = S->data_size;
  m->cluster = -1; /* not yet clustered */
  m->cluster2 = -1;

  for (i=0;i<S->data_dim;i++)
    m->var[i] = m->mean[i] = 0.0;

  for (i=0;i<S->data_size;i++)
    {
      off = i*S->data_dim;
      for (j=0;j<S->data_dim;j++)
	{
	  m->mean[j] += S->data[off+j];
	  m->var[j] += S->data[off+j] * S->data[off+j];
	}
    }
  for (j=0;j<S->data_dim;j++)
    {
      m->mean[j] /= m->count;
      m->var[j] /= m->count;
      m->var[j] -= m->mean[j] * m->mean[j];
    }

  return(0);
}

int init_clusters(suitcase_t *S)
/* initialize the clusters: one member each */
{
  int n,i,j;

  ERR_reset("Initializing clusters");

  S->nclusters = S->nmembers;
  if ( (S->clusters = (cluster_t *)calloc(S->nclusters,sizeof(cluster_t))) == NULL)
    ERR_return("Memory alloc error");

  for (n=0;n<S->nclusters;n++)
    {
      S->clusters[n].count = S->members[n].count;  
      for (j=0;j<S->data_dim;j++)
	{
	  S->clusters[n].mean[j] = S->members[n].mean[j];
	  S->clusters[n].var[j] = S->members[n].var[j];
	}
    }
  
  return(0);
}



/* compute distance of a member from a cluster (not containing the member) 
   Uses cross-entropy measure between two sets of statistics.
   returns -1 if the distance couldn't be computed in either case */

float dist_member2cluster(suitcase_t *S,int mem,cluster_t *c)
{
  int j;
  member_t *m;
  float dist;

  m = S->members + mem;  
  dist = 0.0;

  /* cross entropy */
  
  for (j=0;j<S->data_dim;j++)
    {
      if (m->var[j] == 0 || c->var[j] == 0)
	return (-1);
      
      dist += 0.25 *
	( m->var[j]/c->var[j] 
	  + c->var[j]/m->var[j]
	  + (m->mean[j] - c->mean[j])*(m->mean[j] - c->mean[j])*(1/m->var[j] + 1/c->var[j])
	  - 2.0 );
    }
  dist /= S->data_dim;

  return (dist);
}
