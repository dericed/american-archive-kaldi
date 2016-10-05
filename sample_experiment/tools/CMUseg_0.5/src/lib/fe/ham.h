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
 */

#include "const.h"

typedef struct HAMMING_WINDOW {
    float  *window_pts;
    int window_size;
} HAMMING_WINDOW;
