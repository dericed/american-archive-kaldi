/* sunOS doesn't have a memmove function (!)
#ifdef sun

/*
 from the man page

  The memmove() function copies n bytes from the string at the location
  pointed to by the s2 parameter to the string at the location pointed to by
  the s1 parameter.  Copying takes place as though the n number of bytes from
  string s2 were first copied into a temporary location having n bytes that
  do not overlap either of the strings pointed to by s1 and s2. Then, n
  number of bytes from the temporary location is copied to the string pointed
  to by s1. Consequently, this operation is nondestructive and proceeds from
  left to right.
  */


#include <string.h>

void *memmove(void *s1,const void *s2,size_t n);

void *memmove(void *s1,const void *s2,size_t n)
{

}

#endif
