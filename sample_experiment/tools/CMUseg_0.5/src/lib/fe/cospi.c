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

#include	<math.h>
#include	"cospi.h"

double cospi(x)
double x;
{
   static int set=0;
   static double pi;
   
      if (!set) {
         pi = atan((double)1)*4;
         set = 1;
      }
   return( cos(x*pi) );
}
