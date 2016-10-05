/*

   For each breakpoint defined (fake or real)
   Look for a silence nearby.

   Nearby is defined by silence_region and silence_region_f
   Silence is defined as a sequence of frames with a power level satisfying:
   1. Dynamic Range is small
   2. Mean power is less than Mean power outside the silence region - Threshold.



         
 */
#include "UTT_findsil.h"

#define MIN(x,y) (((x)<(y))?(x):(y))
#define MAX(x,y) (((x)>(y))?(x):(y))
#define RATIO_c0 4.57

int find_silences(suitcase_t *S)
{
  int sn,bn;
  int err,where;

  float c0_thresh,c0_dn;
  float bestDN,bestSNR;

  ERR_reset("find the silences");

  /* c0 based measures converted from dB */
  c0_thresh = S->silence_thresh / RATIO_c0;
  c0_dn = S->silence_dn / RATIO_c0;

  free(S->dn);
  free(S->snr);
  S->dn = (float *)calloc(MAX(S->silence_search_region_f,S->silence_search_region)*2,sizeof(float));
  S->snr = (float *)calloc(MAX(S->silence_search_region_f,S->silence_search_region)*2,sizeof(float));

  free(S->silences_point);
  free(S->silences_found);
  S->silences_point = (int *)calloc(S->num_boundaries+2,sizeof(int));
  S->silences_dn = (float *)calloc(S->num_boundaries+2,sizeof(float));
  S->silences_depth = (float *)calloc(S->num_boundaries+2,sizeof(float));
  S->silences_found = (int *)calloc(S->num_boundaries+2,sizeof(int));

  /* always find one at the beginning */
  sn = 0;
  S->silences_found[sn] = 1;
  S->silences_point[sn++] = 0;

  
  /* omit first and last boundaries, they are for sure */
  for (bn=1;bn<S->num_boundaries-1;bn++)
    {
      /* look for a silence at the boundary point */
      err = find_a_silence(S,S->boundaries_point[bn],S->boundaries_fake[bn],&bestDN,&bestSNR,&where);
      
      switch(err)
	{
	case 0: /* success */
	  if (where != S->silences_point[sn-1])
	    {
	      S->silences_found[sn] = 1;
	      S->silences_depth[sn] = bestSNR*RATIO_c0;
	      S->silences_dn[sn] = bestDN*RATIO_c0;
	      S->silences_point[sn++] = where;

	      if (S->verbose)
		fprintf(stderr,"%s: Found a silence near bp=%d at %d -- (DN=%4.2f,SNR=%4.2f)\n",
			S->argv[0],S->boundaries_point[bn]+S->frame_start,where+S->frame_start,bestDN*RATIO_c0,bestSNR*RATIO_c0);
	    }
	  else /* duplication, it happens! */
	    {
	      S->silences_found[sn] = 0;
	      S->silences_point[sn++] = where;
	      if (S->verbose)
		fprintf(stderr,"%s: Found a duplicate silence near bp=%d at %d\n",
			S->argv[0],S->boundaries_point[bn]+S->frame_start,where+S->frame_start);
	    }
	  break;
	  
	case 1: /* DN to high */
	  S->silences_found[sn++] = 0;
	  if (S->verbose)
	    fprintf(stderr,"%s: No silence near bp=%d -- Too much dn at %d (DN=%4.2f)\n",
		    S->argv[0],S->boundaries_point[bn]+S->frame_start,where+S->frame_start,bestDN*RATIO_c0);
	  break;
	  
	case 2: /* SNR to low */
	  S->silences_found[sn++] = 0;
	  if (S->verbose)
	    fprintf(stderr,"%s: No silence near bp=%d -- Not quiet enough at %d(SNR=%4.2f)\n",
		    S->argv[0],S->boundaries_point[bn]+S->frame_start,where+S->frame_start,bestSNR*RATIO_c0);
	  break;


	case 3: /* couldn't satisfy simultaneously */
	  S->silences_found[sn++] = 0;
	  if (S->verbose)
	    fprintf(stderr,"%s: No silence near bp=%d -- Best conditions aren't met at same time\n",
		    S->argv[0],S->boundaries_point[bn]+S->frame_start);
	  break;

	default:
	  ERR_return("Problem with silence detector");
	}
    }


  /* always find one at the end */
  S->silences_found[sn] = 1;
  S->silences_point[sn++] = S->data_size-1;
  S->num_silences = sn;
}


/* Find a silence near a breakpoint.  Fake is whether to use narrow or wide search.
   Returns 0 for success, and where is where it found the best silence.  bestDN and bestSNR
   are returned, where points the the location.
*/

int find_a_silence(suitcase_t *S,int point,int fake,float *bestDN,float *bestSNR,int *where)
{
  int whereDN,whereSNR;

  int notfound;
  float outer_mean,inner_mean,inner_min,inner_max;
  int search_region; /* where to look (+/- the center point) */
  int fr,fr2;
  int beg_fr,end_fr;
  int beg_inner_fr,end_inner_fr;
  int beg_outer_fr,end_outer_fr;

  search_region = (fake)?S->silence_search_region_f:S->silence_search_region;

  /* constrain to limits of data */
  beg_fr = MAX(0,point - search_region);
  end_fr = MIN(S->data_size,point + search_region);

  
  /* collect the DN and SNR stats */
  for (fr=beg_fr;fr<end_fr;fr++)
    {
      beg_outer_fr = MAX(0,fr - S->silence_window_outer);
      end_outer_fr = MIN(S->data_size,fr + S->silence_window_outer);

      /* get outer stats */
      outer_mean = 0;
      for (fr2 = beg_outer_fr;fr2 < end_outer_fr;fr2++)
	outer_mean += S->data[fr2*S->data_dim];
      outer_mean /= (end_outer_fr - beg_outer_fr);


      /* get inner stats */

      beg_inner_fr = MAX(0,fr - S->silence_window);
      end_inner_fr = MIN(S->data_size,fr + S->silence_window);

      inner_mean = 0;
      for (fr2 = beg_inner_fr;fr2 < end_inner_fr;fr2++)
	{
	  if (fr2 == beg_inner_fr || inner_max < S->data[fr2*S->data_dim])
	    inner_max = S->data[fr2*S->data_dim];

	  if (fr2 == beg_inner_fr || inner_min > S->data[fr2*S->data_dim])
	    inner_min = S->data[fr2*S->data_dim];

	  inner_mean += S->data[fr2*S->data_dim];
	}
      inner_mean /= (end_inner_fr - beg_inner_fr);


      S->snr[fr-beg_fr] = outer_mean - inner_mean;
      S->dn[fr-beg_fr] = inner_max - inner_min;
    }

  for (fr=beg_fr;fr<end_fr;fr++)
    {
      if (fr == beg_fr || S->dn[fr-beg_fr] < *bestDN)
	{
	  *bestDN = S->dn[fr-beg_fr];
	  whereDN = fr;
	}

      if (fr == beg_fr || S->snr[fr-beg_fr] > *bestSNR)
	{
	  *bestSNR = S->snr[fr-beg_fr];
	  whereSNR = fr;
	}
    }

  /* find out best case */
  if (*bestDN > S->silence_dn / RATIO_c0)
    {
      *where = whereSNR;
      return(1);
    }

  if (*bestSNR < S->silence_thresh / RATIO_c0)
    {
      *where = whereSNR;
      return(2);
    }

  
  notfound = 1;
  /* best silence has lowest SNR at any DN that suffices */
  for (fr=beg_fr;fr<end_fr;fr++)
    {
      /* when conditions are met,  and SNR is optimal (or first time) */
      if ((S->dn[fr-beg_fr] < S->silence_dn/RATIO_c0 && S->snr[fr-beg_fr] > S->silence_thresh / RATIO_c0)
	  && (notfound || *bestSNR < S->snr[fr-beg_fr]))
	{
	  *bestDN = S->dn[fr-beg_fr];
	  *bestSNR = S->snr[fr-beg_fr];
	  *where = fr;
	  notfound = 0;
	}
    }

  /* can't meet both specs */
  if (notfound)
    return(3);
  
  return(0);
}






