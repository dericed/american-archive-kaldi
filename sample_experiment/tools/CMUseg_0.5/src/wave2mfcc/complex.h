
/* Complex number manipulation routine headers				*/
/*
 * BUGS
 *	If you pass a double to one of these routines when a complex
 *	is expected, the calculations performed will be incorrect in
 *	most cases.  However, the code will compile without generating
 *	error or warning messages.  Linting your source will catch
 *	these bugs (highly recommended).
 *
 * HISTORY
 * 15-Feb-87  Andy Gruss (gruss) at Carnegie-Mellon University
 *	Created.
 *
 */
#ifndef	M_PI
#define M_PI	(3.14159265358979323846)
#endif	M_PI
typedef struct { double r, i; } complex;

/* Conversions to complex form						*/
extern complex c_cartc();
#define	c_realc(r)	(c_cartc((r), 0.0))
#define	c_imagc(i)	(c_cartc(0.0, (i)))
#define	c_polarc(m, w)	(c_cartc((m)*cos(w), (m)*sin(w)))
#define c_magc(m)	(c_realc(m))		/* Angle is 0		*/
#define c_argc(w)	(c_polarc(1.0, (w)))	/* Magnitude is 1	*/

/* Conversions from complex form (return type double, NOT complex!!)	*/
#define	c_real(x)	((x).r)			/* Valid lvalue!	*/
#define	c_imag(x)	((x).i)			/* Valid lvalue!	*/
#define c_mag(x)	(sqrt((x).r*(x).r + (x).i*(x).i))
#define c_mag2(x)	((x).r*(x).r + (x).i*(x).i)
#define	c_dB(x)		(10.0*log10(c_mag2(x)))
#define c_arg(x)	(atan2((x).i, (x).r))

/* Operations on complex numbers					*/
extern complex c_neg(), c_conj(), c_inv();
extern complex c_add(), c_sub(), c_mul(), c_div();
extern complex c_exp(), c_log(), c_pow(), c_root();
extern complex c_wgnoise();

