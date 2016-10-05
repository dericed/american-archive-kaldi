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

#include "cospi.h"

extern char **alloc2d();

/* -------------------------------------------------------------------
   This function creates a set of N half cosines for cepstral vectors.
   They are stored by row so we can use the rotate_vector function.
   We use the c[k] = sum x[n] cos (pi/N n-1/2 k) formula.
   We return the float matrix.
   ------------------------------------------------------------------- */
float **create_rotation_vectors (vector_size, new_vector_size)
int vector_size, new_vector_size;
{
    int i, j;
    float **matrix, scale, *row;

    matrix = (float **) alloc2d (new_vector_size, vector_size, sizeof(float));
    scale = 1.0 / (float) vector_size;

    for (i=0; i<new_vector_size; i++) {
	row = matrix[i];
	for (j=0; j<vector_size; j++) {
	    /*    row[j] = cospi (i * (j + 0.5) * scale);*/ /* j is 0 based, so add 1/2 */
	    /*
	     * modified by Aki
	     */
	    row[j] = cospi ( ((double) scale)*((double) i)*( ((double) j)+0.5) );
	}
    }
    return matrix;
}

/* -----------------------------------------------------------
   This function rotates a vector by a set of rotation vectors.
   The rotation vectors are assumed to be stored by row.
   We do num_vectors rotations and store results in new_vector.
   ----------------------------------------------------------- */
rotate_vector (vector, vector_size, rotation_vectors, new_vector, num_vectors)
int vector_size, num_vectors;
float *vector, **rotation_vectors, *new_vector;
{
    register int n;
    register float *vp, *end, *rp, sum;

    end = vector + vector_size;		/* set up the end pointer	*/
    rp = rotation_vectors[0];		/* initialize rotation pointer	*/

    for (n=0; n<num_vectors; n++) {
	sum = 0.0;			/* (num_vectors <= vector_size) */
	for (vp=vector; vp<end;)	/* take the dot product of the 	*/
	    sum += *(vp++) * *(rp++);	/* vector and rotation vector	*/
	new_vector[n] = sum;		/* store in the new_vector	*/
    }
}

