/* sunOS doesn't have a f_memmove function (!)
   so I wrote a simple one.

  The f_memmove() function copies n bytes from the string at the location
  pointed to by the s2 parameter to the string at the location pointed to by
  the s1 parameter. Copying proceeds from left to right, which means overlapping
  strings are alright id the source overlaps the right half. This operates on characters
  directly.

        |------------|  from
  |---------|           to

  */


#include <string.h>

void *f_memmove(void *s1,const void *s2,size_t n);

void *f_memmove(void *s1,const void *s2,size_t n)
{
  char *sp1,*sp2;
  size_t i=0;
  sp1 = (char *)s1;sp2 = (char *)s2;

  while (i<n)
    {
      *sp1 = *sp2;
      sp1++; sp2++; i++;
    }
  return((void *)s1);
}

