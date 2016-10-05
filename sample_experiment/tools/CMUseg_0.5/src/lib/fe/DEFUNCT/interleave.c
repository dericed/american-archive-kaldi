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

interleave(out, in_a, in_b, in_len)
float	*out, *in_a, *in_b;
int	in_len;
{
    int i, j;

    for (i = 0, j = 0; i < in_len; i++, j += 2) {
	out[j]   = in_a[i];
	out[j+1] = in_b[i];
    }
}

unpack(out_a, out_b, out_len, in)
float	*out_a, *out_b;
int	out_len;
float	*in;
{
    int i, j;

    for (i = 0, j = 0; i < out_len; i++) {
	out_a[i] = in[j++];
	out_b[i] = in[j++];
    }
}


