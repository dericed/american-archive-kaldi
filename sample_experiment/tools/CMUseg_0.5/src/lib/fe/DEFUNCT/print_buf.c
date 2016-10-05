/* PRINT_BUF - buffer printing routines
 *------------------------------------------------------------*
 * Copyright 1988, Fil Alleva and Carnegie Mellon University
 *------------------------------------------------------------*
 */
/* HISTORY
 * 23-May-89  Jeff Rosenfeld (jdr) at Carnegie-Mellon University
 *	Created history. Fixed typo/bug in print_sbuf().
 */

#include <stdio.h> 

/* PRINT_SBUF - print buffer of short
 *------------------------------------------------------------*
 */
print_sbuf (n, buf, str, shift)
register int n;		/* Datum count */
register short *buf;	/* Data pointer */
register int shift;	/* Amount to shift before printing */
char *str;		/* Label to print */
{
    register int i;

    fprintf (stdout, "%s\n", str);
    for (i = 0; i < n; i++)
	fprintf (stdout, "%6d%c", *buf++ >> shift, ((i+1) % 10 ? ' ' : '\n'));
    fprintf (stdout, "\n");
}

/* PRINT_SBUF - print buffer of float
 *------------------------------------------------------------*
 */
print_fbuf (n, buf, str)
register int n;		/* Datum count */
register float *buf;	/* Data pointer */
char *str;		/* Label to print */
{
    register int i;

    fprintf (stdout, "%s\n", str);
    for (i = 0; i < n; i++)
	fprintf (stdout, "%.2f%c", *buf++, ((i+1) % 10 ? ' ' : '\n'));
    fprintf (stdout, "\n");
}
