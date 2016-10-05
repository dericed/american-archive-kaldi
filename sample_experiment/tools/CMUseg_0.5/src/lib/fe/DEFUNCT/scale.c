/* SCALE
 *------------------------------------------------------------*
 * Copyright 1988, Fil Alleva and Carnegie Mellon University
 *------------------------------------------------------------*
 * HISTORY
 * 23-May-89  Jeff Rosenfeld (jdr) at Carnegie-Mellon University
 *	Type-casts on args to bcopy() to pacify lint.
 *
 * 20-Jan-89  Fil Alleva (faa) at Carnegie-Mellon University
 *	Changed to return 0 when input has 0 magnitude rather than
 *	returning 'msb' since there is no correct scsling for a zero
 *	signal.
 *
 */

#include <stdlib.h>


scale (out, in, n, msb)
register short *out;
short *in;
int n;		/* Data count */
int msb;
{
    register            i;
    register short      mag = 0;
    register short      samp;
    register short     *p;
    register int        shift;

    i = n;
    p = in;
    do {
	samp = *p++;
	if (samp >= 0)
	    mag |= samp;
	else
	    mag |= (-samp);
    } while (--i);

    /*
     * If the signal is 0 then there is no shift.
     */
    if (mag == 0) {
	if (in != out)
	    memcpy ((char *)out, (char *)in, n * sizeof (short));
	return (0);
    }

    shift = msb - 14;
    while (!(mag & 0x4000)) {
	mag <<= 1;
	shift++;
    }

    if (shift) {
	i = n;
	p = in;
	if (shift > 0) {
	    do {
		*out++ = *p++ << shift;
	    } while (--i);
	}
	else {
	    shift = -shift;
	    do {
		*out++ = *p++ >> shift;
	    } while (--i);
	    shift = -shift;
	}
    }
    else {
	if (in != out)
	    memcpy ((char *)out, (char *)in, n * sizeof (short));
    }
    return shift;
}
