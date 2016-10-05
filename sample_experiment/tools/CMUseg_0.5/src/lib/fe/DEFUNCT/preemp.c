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


void preemp(out, in, prior, icnt)
float *out;
short *in;
short prior;
int icnt;
{
    int i;
    short t1, t2;
    float factor = 0.97;

    t1 = prior;
    for (i = 0; i < icnt; i++) {
	t2 = in[i];
	out[i] = (float) t2 - factor * (float) t1;
	t1 = t2;	/* hopefully a register to register xfer */
    }
}

