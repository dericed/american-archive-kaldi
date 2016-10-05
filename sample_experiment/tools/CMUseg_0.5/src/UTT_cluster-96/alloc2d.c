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

#include <stdio.h>
#include <stdlib.h>


/*CopyRight 1991 Massachusetts Institute of Technology*/
char          **alloc2d(dim1, dim2, size)
int dim1;		/* "x" dimension */
int dim2;		/* "y" dimension */
int size;		/* number of bytes each entry takes */
{
    int             i;		/* loop control variable */
    unsigned        nelem;	/* total number of elements */
    char	    *p,		/* pointer to matrix memory */
    **pp;			/* pointer to matrix mem table */

    /*
     * Compute total number of elements needed for the two-dimensional
     * matrix
     */
    nelem = (unsigned) dim1 * dim2;

    /*
     * Allocate the memory needed for the matrix
     */
    p = calloc(nelem, (unsigned) size);

    /*
     * If the allocation were not successful, return a NULL pointer
     */
    if (p == NULL) return (NULL);

    /*
     * Now allocate a table for referencing the matrix memory
     */
    pp = (char **) calloc((unsigned) dim1, (unsigned) sizeof(char *));

    /*
     * If the allocation were not successful, return a NULL pointer
     * and free the previously allocated memory
     */
    if (pp == NULL) {
	free(p);
	return (NULL);
    }

    /*
     * Fill the table with locations to where each row begins
     */
    for (i = 0; i < dim1; i++)
	pp[i] = p + (i * dim2 * size);

    return (pp);
}

void free2d(p)
void **p;
{
   int i;
   if (p!=NULL) {
      free(p[0]);
      free(p);
   }
   return;
}

