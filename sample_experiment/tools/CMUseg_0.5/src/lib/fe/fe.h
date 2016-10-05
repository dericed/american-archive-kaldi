/*
 * fe.h -- Front-end for computing raw audio samples into cepstra
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
 * 16-August-96 M A Siegler (msiegler@cs.cmu.edu) Uses default values now
 *
 * 21-May-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Substantially modified to be driven with externally provided data, rather
 * 		than explicitly reading an A/D source.
 * 		Removed logging functions (now done at a higher level, eg, by the caller).
 * 
 * 29-Nov-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created.
 */


#ifndef _FE_H_
#define _FE_H_

int32 fe_defaults ( void );
/* call to set the default parameters for the filters
   


/*
 * One-time initialization for entire run.
 * Return value: 0 if successful, <0 if error.
 */

int32 fe_init ( void );

/*
 * Called once at start of each utterance.
 * Return value: 0 if successful, <0 if error.
 */
int32 fe_start ( void );


/*
 * Called once at end of each utterance.
 * Return value: 0 if successful, <0 if error.
 */
int32 fe_stop ( void );


/*
 * Compute cepstra for the given input speech samples.
 * Each call can provide arbitrary #samples to be processed.  Also, samples for successive
 * frames are overlapped.  Therefore, an internal buffer is used to manage partial frame
 * samples and overlap between successive calls.
 * Return value: #cep frames computed, possibly 0; <0 if error.
 */
int32 fe_raw2cep (int16 *raw,		/* In: Buffer containing input samples */
		  int32 rawlen,		/* In: #samples in raw buf */
		  float **cep);		/* Out: cep[0]..cep[N-1] contain cepstrum output
					   vectors (N is #frames computed this call) */

int32 fe_raw2spec (int16 *raw,		/* In: Buffer containing input samples */
		  int32 rawlen,		/* In: #samples in raw buf */
		  float **spec);	/* Out: spec[0]..spec[N-1] contain log spectrum output
					   vectors (N is #frames computed this call) */

#endif
