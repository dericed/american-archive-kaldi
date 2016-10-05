/*
 * 
 * 
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1996 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * 
 Modified
 Mon Aug 12 1996
 Matthew A. Siegler
 Added some parameters here that were constants before, and cleaned up the structure
*/
#include "const.h"




float SAMPLE_RATE;
int32 DFT_POINTS;
int32 LOG2_DFT_POINTS;
int32 FRAME_RATE;
int32 FRAME_SPACING;
int32 LEN_WINDOW;
int32 NO_OF_FILTERS;
int32 NUMBER_OF_LOG_TRIANGLES;
int32 CEP_VECLEN;
float LOG_SPACING;
float LINEAR_SPACING;
float LOWER_EDGE_OF_FILTERS;
float UPPER_EDGE_OF_FILTERS;
float HAMMING_WINDOW_LENGTH;
float PREEMPH_ALPHA;

/* this value means use the internal default */
#define FILTER_DEFAULT -1

/* filter parameters */
typedef struct MEL_FILTER {
  float left_edge, center_freq, right_edge;
  float *filter_pts;
  int    filter_size;
} MEL_FILTER;

MEL_FILTER *mel_filters;

#ifndef _H_defaults
#define _H_defaults
/* defaults for mel_filters */

#define __SAMPLE_RATE  16000.0
#define __DFT_POINTS  256
#define __LOG2_DFT_POINTS  8
#define __FRAME_RATE  100
#define __FRAME_SPACING  160

#define __HAMMING_WINDOW_LENGTH  0.025625
#define __LEN_WINDOW  ((int32)((float)__HAMMING_WINDOW_LENGTH * (float)__SAMPLE_RATE + 0.5))

#define __NO_OF_FILTERS  40
#define __NUMBER_OF_LOG_TRIANGLES  27
#define __CEP_VECLEN  13
#define __LOG_SPACING  1.0711703
#define __LINEAR_SPACING  ((float)(66.666664 / 16000.0 * __SAMPLE_RATE ))
#define __LOWER_EDGE_OF_FILTERS  ((float)(133.33334 / 16000.0 * __SAMPLE_RATE ))
#define __UPPER_EDGE_OF_FILTERS  ((float)(6855.4976 / 16000.0 * __SAMPLE_RATE ))

#define __PREEMPH_ALPHA  0.97


#endif
