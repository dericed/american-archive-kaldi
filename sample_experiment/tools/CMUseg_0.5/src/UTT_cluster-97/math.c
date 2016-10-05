#include "UTT_cluster.h"

/* get mean & var of a set of data */
int get_member_stat(suitcase_t *S,int n)
{
  int i,j,off;
  member_t *m;
  
  m = S->members + n;
  m->count = S->data_size;
  m->cluster = n;

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




int init_clusters(suitcase_t *S,float **cdist)
/* initialize the clusters: one member each */
/* initialize the cdist matrix */
{
  int n,i,j;

  ERR_reset("Initializing clusters");

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
  
  /* cluster distance initialization, decides which cluster distances DO get recomputed */
  /* -1 == recompute, -2 == skip this one */

  for (i=0;i<S->nclusters;i++)
    for (j=0;j<S->nclusters;j++)
      cdist[i][j] = -1;
  
  return(0);
}



/* compute distance of a member from a cluster (not containing the member) 
   Uses cross-entropy measure between two sets of statistics.
   if member has not enough data points, the mahalanobis distance is used.
   returns -1 if the distance couldn't be computed in either case */

float dist_member2cluster(suitcase_t *S,int mem,cluster_t *c)
{
  int j;
  member_t *m;
  float dist;



  if (c->count < S->cluster_mincount)
    return(-1);

  m = S->members + mem;  
  dist = 0.0;

  if (m->count >= S->cluster_mincount)
    /* cross entropy */
    {
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
    }
  else
    /* mahalanobis distance */
    {
      for (j=0;j<S->data_dim;j++)
	{
	  if (c->var[j] == 0)
	    return (-1);
	  
	  dist += 0.5 * (m->mean[j] - c->mean[j])*(m->mean[j] - c->mean[j]) / c->var[j];
	}
      dist /= S->data_dim;
    }

  return (dist);
}


int stat_combine_clusters(suitcase_t *S,int clus1,int clus2,cluster_t *m)
/* compute statistics if two clusters are combined return -1 if one of the clusters is empty*/
{
  int j;
  cluster_t *c1,*c2;


  
  c1 = S->clusters+clus1;
  c2 = S->clusters+clus2;

  if (c1->count == 0 || c2->count == 0)
    return(-1);

  m->count = c1->count + c2->count;
  
  for (j=0;j<S->data_dim;j++)
    {

      m->mean[j] = (c1->count * c1->mean[j] + c2->count * c2->mean[j])/m->count;
      m->var[j] =  (c1->count * c1->var[j] + c2->count * c2->var[j] + 
		    c1->count * c1->mean[j] * c1->mean[j] + 
		    c2->count * c2->mean[j] * c2->mean[j] ) / m->count - m->mean[j] * m->mean[j];
    }

  return(0);
}



float dist_combine_clusters(suitcase_t *S,int c1,int c2)
/* Compute maximum distance if two clusters are combined.
   Returns -1 if one cluster was empty, or too many variances were 0
   if c1==c2, just returns the maximum distance of that cluster  */   

/* Smart about small counts in members. if member count < S->cluster_mincount,
   the variance is not trusted, and mahalanobis distance is used instead.
   If size of combined cluster is too small, a -1 is returned */

/* 22 Oct 1997 msiegler
   Maximum cluster criteria: keeps clusters from getting too big.

   If defined (0 means undefined)
   Two clusters may not be joined if
    Both are larger than cluster_mincount and the sum of their counts is larger than cluster_maxcount and 
   This allows small items to be added to bigger ones, but does not permit combining large ones
   */
    
   

{
  int i,ct,first=1;
  float dist,max_dist;
  cluster_t c;
  
  /* not enough data in the combined cluster */
  if (S->clusters[c1].count + S->clusters[c2].count < S->cluster_mincount)
    return(-1);

  /* clusters are too big (and we care) */
  /*
  if (S->cluster_maxcount &&
      (S->clusters[c1].count > S->cluster_mincount && S->clusters[c2].count > S->cluster_mincount) &&
      (S->clusters[c1].count + S->clusters[c2].count > S->cluster_maxcount) )
    return(-1);
    */


  if(stat_combine_clusters(S,c1,c2,&c))
    return(-1);

  ct = 0;
  for (i=0;i<S->nmembers;i++)
    if (S->members[i].cluster == c1 || S->members[i].cluster == c2)
      {

	/* built in detection of member size */
	dist = dist_member2cluster(S,i,&c);
	
	if (dist != -1)
	  {
	    if (first || max_dist < dist)
	      {
		max_dist = dist;
		first = 0;
	      }
	    ct++;
	  }
      }
  
  if (ct)
    return(max_dist);
  else
    return(-1);
}


int combine_clusters(suitcase_t *S,int c1,int c2)
/* combine two clusters and renumber them two the lower of the two
   zero out the other cluster. Returns -1 if one of the clusters was empty 
   returns new number */
{
  int j,renum,erase;
  cluster_t c;

  /* no action */
  if (c1 == c2)
    return(0);

  renum = MIN(c1,c2);
  erase = MAX(c1,c2);

  if (stat_combine_clusters(S,c1,c2,&c))
    return(-1);

  S->clusters[erase].count = 0;
  S->clusters[renum].count = c.count;

  for (j=0;j<S->data_dim;j++)
    { 
      S->clusters[erase].mean[j] = 0.0;
      S->clusters[erase].var[j] = 0.0;
      S->clusters[renum].mean[j] = c.mean[j];
      S->clusters[renum].var[j] = c.var[j];
    }

  /* move cluster pointer */
  for (j=0;j<S->nmembers;j++)
    if (S->members[j].cluster == erase)
      S->members[j].cluster = renum;

  return(renum);
}

int update_cluster_distances(suitcase_t *S,float **cdist)
{
  int i,j;

  /* get computations we haven't done yet, was reset to -1 if clusters changed somehow */
  for (i=0;i<S->nclusters;i++)
    for (j=0;j<S->nclusters;j++)
      if (cdist[i][j] == -1)
	cdist[i][j] = dist_combine_clusters(S,i,j);

  return(0);
}


int reset_cluster_distances(suitcase_t *S,float **cdist,int best_i,int best_j)
{
  int i,j;

  for (i=0;i<S->nclusters;i++)
    {
      /* seeds the recomputation for later: rows & columns! 
	 if the cdist value is < -1 i.e. -2, that means we aren't ever
	 supposed to recompute, so make sure we don't force it to -1 later! 
	 However, cdist for actual distances is on the interval [0,1] */

      cdist[best_i][i] = MIN(cdist[best_i][i],-1);
      cdist[i][best_i] = MIN(cdist[i][best_i],-1);
      cdist[best_j][i] = MIN(cdist[best_j][i],-1);
      cdist[i][best_j] = MIN(cdist[i][best_j],-1);
    }

  return(0);
}
