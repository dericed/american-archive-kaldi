/*

Thu Aug 8  1996 msiegler
Matthew A. Siegler

 Data structures for the mfc conversion
*/

#ifndef _h_mfc
#define _h_mfc

/* filter definitions */
typedef struct mfc_parm_s
{
  int32   rate_sample;        /* samples/second */
  float rate_frame;         /* frames/second  */
  
  int32   len_window;
  int32   len_dft;
  
  int32   num_filters;
  int32   num_logfilters;
  int32   num_linfilters;

  int32   num_vector;

  float ratio_logspacing;
  float width_linspacing;
  
  float freq_firstfilter;
  float freq_lastfilter;
} mfc_parm_t;
