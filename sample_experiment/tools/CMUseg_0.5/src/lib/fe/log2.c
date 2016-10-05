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

#include <math.h>

double log2(x)
double x;
{
   static int set=0;
   static double lg2;

      if (!set) {
         lg2 = log((double)2);
         set = 1;
     }
   return( log(x)/lg2 );
}
