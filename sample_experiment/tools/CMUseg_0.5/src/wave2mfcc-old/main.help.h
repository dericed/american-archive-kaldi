fprintf(stderr,
	"wave2mfcc - Convert audio stream into Mel-Frequency Cepstrum Coefficients\n"
	"Usage: %s\n"
	" FILE CONTROL)\n"
	"\t (single) -i <audio file> -o <cep file>\n"
	"\t (batch)  -c <control file> -di <input directory>   -ei <input extension>\n"
	"\t                            -do <output directory>  -eo <output extension>\n"
	"\n"
	" INPUT FORMAT\n"
	"\t -sphere (NIST format)\n"
	"\t -adc (old CSR format)\n"

	"\t -raw <B(ig endian) |  L(ittle endian) | M|U (mu law)> (raw binary)\n"
	"\n"
	" OUTPUT FORMAT\n"
	"\t -sphinx (SPHINX format)\n"
	"\t -logspec (not cepstra)\n"
	"\n"
	" FILTER PARAMETERS\n"
	"\t -alpha <premphasis parameter [def=%.3f]>\n"
	"\t -srate <sampling rate [def=%.3f Hz]>\n"
	"\t -frate <frame rate [def=%.3f frames/sec]>\n"
	"\t -wlen  <Hamming window length [def=%.4f sec]>\n"
	"\t -dft   <number of DFT points [def=%d samples]>\n"
	"\t -nfilt <number of fitler banks [def=%d]>\n"
	"\t -nlog  <number of log filter banks [def=%d]\n"
	"\t -logsp <log filter spacing ratio [def=%.5f]>\n"
	"\t -linsp <lin filter spacing [def=%.5f Hz]>\n"
	"\t -lowerf <lower edge of filters [def=%.5f Hz]>\n"
	"\t -upperf <upper edge of filters [def=%.5f Hz]>\n"
	"\t -ncep  <number of cep coefficients [def=%d]>\n"
	"\n"
	" INPUT PROCESSING\n"
	"\t -DC (offset removal)\n"
	"\t -dither (add 1-bit noise)\n"
	"\t -resample (DISABLED!) <up_factor> <down_factor> <FIR filter>\n"
	"\t -addnoise (DISABLED!) <db SNR power> (add noise to SNR)\n"
	"\t -babble (DISABLED!) <babble file> <babble SNR> (add external babble file to SNR)\n"
	"\n"
	" DIAGNOSTICS\n"
	"\t -showfilt (print out a list of all the filter parameters)\n"
	"\t -verbose (show lots of debug information)\n" 
	"\t -h[elp] (this screen)\n"
	"** compiled %s %s **\n",
	S->argv[0],
	(float)__PREEMPH_ALPHA,
	(float)__SAMPLE_RATE,
	(float)__FRAME_RATE,
	(float)__HAMMING_WINDOW_LENGTH,
	(int32)__DFT_POINTS,
	(int32)__NO_OF_FILTERS,
	(int32)__NUMBER_OF_LOG_TRIANGLES,
	(float)__LOG_SPACING,
	(float)__LINEAR_SPACING,
	(float)__LOWER_EDGE_OF_FILTERS,
	(float)__UPPER_EDGE_OF_FILTERS,
	(int32)__CEP_VECLEN,
	__DATE__,__TIME__);

