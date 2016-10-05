/* File: w_edit.c */

/************************************************/
/* This program is ueed to edit NIST SPHERE     */
/* headered files, creating a new file          */
/************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WEDIT_VERSION "2.0"

#include <sys/stat.h>
#include <sp/sphere.h>

char usage[] = "Usage:  %s [-v] -o [-[t|p]F:T]] [-w wes_script] [-c EXP] [-o OUT ] [-O Out_dir] { filein | - } \
{ fileout | - }\n\
        %s [-v] [-o OUT] [-O Out_dir] [-w wes_script] [-c EXP]\n\
w_edit Version: " WEDIT_VERSION ", SPHERE Library: " SPHERE_VERSION_STR"\n\
Where:  -tF:T	 Set the range for output from time F (seconds) to time T\n\
	-sF:T    Set the range for output from F samples to T samples\n\
\n\
		 - if F is missing, it defaults to the beginning of the \
file,\n\
		   if T is missing, go to the end of the file. \n\
\n\
	-cEXP    Extract only the samples from channels in EXP.  The \n\
		 expression will also add channels together if the '+'\n\
		 is usd.\n\
\n\
	-oOUT	 Set the output format to the following formats:\n\
                        alaw\n\
	         	ulaw\n\
	         	pculaw\n\
			pcm_01 | short_01\n\
			pcm_10 | short_10\n\
			pcm | short_natural\n\
        -O out_dir\n\
                 Define the output directory.  The directory is prepended to\n\
                 'fileout' with a '/' between 'out_dir' and 'fileout'.  Exclusive with\n\
                 'fileout' = '-'.\n\
        -S wes_script\n\
                 Define the W_Edit Script (WES) file\n\
	-v	 give verbose output\n\n";
char * prog;

#define BUF_SAMPLES  65536 

/* function prototypes */
int wedit_openfile(char *, char *, SP_FILE **, SP_INTEGER *, SP_INTEGER *, SP_INTEGER *);
int wedit_make_file(SP_FILE *, char *, char *, char *, char *, char *,SP_INTEGER , SP_INTEGER, SP_INTEGER, char *, SP_INTEGER, SP_INTEGER, int);
int wedit_closefile(char *, char *, SP_FILE **);
int process_wes_script(char *, char *);
int parse_info_line(char *read_buf, char **out_dir,
		    char **out_sdm, char **out_ext, char **units,
		    char **input_dir, SP_FILE **sp, int parse_only, char *pg);
int parse_segment_def(char *read_buf, char **infile, char **number1, 
                      char **number2, char **outfile, char *units);
int build_output_file(char *out_dir, char *out_sdm, char *out_ext,
                      char *units,char *input_dir, SP_FILE **sp, 
		      SP_INTEGER *channel_count, SP_INTEGER *sample_rate,
		      SP_INTEGER *sample_count, char *infile, char *number1,
                      char *number2, char *outfile);

int main(int argc, char **argv)
{
    char *filein=CNULL, *fileout=CNULL;
    SP_FILE *sp_in=SPNULL;
    char *format_conversion="SE-ORIG:SBF-ORIG";
    char sdmod[100];
    char *channel_conversions=CNULL, *tstr;
    SP_INTEGER channel_count, sample_rate, sample_count;   
    double begin_time=0.0, end_time=(-1.0);
    long begin_sample=0, end_sample=(-1);
    int use_sample=1, exit_val=0, c;
    char *wes_script = CNULL;
    char *out_dir = CNULL;

    prog = strrchr( argv[0], '/' );
    prog = ( prog == CNULL ) ? argv[0] : ( prog + 1 );


    while (( c=hs_getopt( argc, argv, "O:S:vo:c:s:t:" )) != -1 )
	switch ( c ) {
	  case 'v':
	    sp_verbose++;
	    break;
	  case 't':
	  case 's':
	    if (c == 't') use_sample=0;
	    tstr = strtok(hs_optarg,":");
	    if (*hs_optarg != ':'){
		if  (tstr != CNULL){
		    if (! strsame(tstr,"")){
			if (is_number(tstr)){
			    if (c == 't') {
				if ((begin_time = atof(tstr)) < 0){
				    fprintf(spfp,"Error: Beginning ");
				    fprintf(spfp,"time must be positive\n");
				    fprintf(spfp,usage,prog,prog);
				    goto FATAL_EXIT;
				}
			    } else {
				if ((begin_sample = atoi(tstr)) < 0){
				    fprintf(spfp,"Error: Beginning sample");
				    fprintf(spfp," must be positive\n");
				    fprintf(spfp,usage,prog,prog);
				    goto FATAL_EXIT;
				}
			    }
			} else {
			    fprintf(spfp,"Error: numeric value required ");
			    fprintf(spfp,"for '%c' option\n",c);
			    fprintf(spfp,usage,prog,prog);
			    goto FATAL_EXIT;
			}
		    }		    
		}
		tstr = strtok(CNULL,":");
	    }
	    if  (tstr != CNULL){
		if (! strsame(tstr,"")){
		    if (is_number(tstr)){
			if (c == 't'){
			    if ((end_time = atof(tstr)) < 0){
				fprintf(spfp,"Error: Ending time must ");
				fprintf(spfp,"be positive\n");
				fprintf(spfp,usage,prog,prog);
				goto FATAL_EXIT;
			    }
			} else {
			    if ((end_sample = atoi(tstr)) < 0){
				fprintf(spfp,"Error: Ending sample must ");
				fprintf(spfp,"be positive\n");
				fprintf(spfp,usage,prog,prog);
				goto FATAL_EXIT;
			    }
			}
		    } else {
			fprintf(spfp,"Error: numeric value required ");
			fprintf(spfp,"for '%c' option\n",c);
			fprintf(spfp,usage,prog,prog);
			goto FATAL_EXIT;
		    }
		}		    
	    }
	    if ((c == 't' && (begin_time > end_time) && (end_time != (-1.0)))||
		(c == 's' && (begin_sample > end_sample) && (end_sample != 
							     (-1)))){
		    fprintf(spfp,"Error: Beginning %s is after Ending %s.\n",
			    (c == 't') ? "time" : "sample",
			    (c == 't') ? "time" : "sample");
		    goto FATAL_EXIT;
		}
	    break;
	  case 'c':
	    channel_conversions = mtrf_strdup(hs_optarg);
	    break;
	  case 'S':
	    wes_script = mtrf_strdup(hs_optarg);
	    break;
	  case 'O':
	    out_dir = mtrf_strdup(hs_optarg);
	    break;
	  case 'o':
	    if (strsame(hs_optarg,"short_01") || strsame(hs_optarg,"pcm_01"))
		format_conversion = "SE-PCM:SBF-01";
	    else  if (strsame(hs_optarg,"short_10") ||
		      strsame(hs_optarg,"pcm_10"))
		format_conversion = "SE-PCM:SBF-10";
	    else  if (strsame(hs_optarg,"short_natural") || 
		      strsame(hs_optarg,"pcm"))
		format_conversion = "SE-PCM:SBF-N";
	    else  if (strsame(hs_optarg,"ulaw"))
		format_conversion = "SE-ULAW";
	    else  if (strsame(hs_optarg,"pculaw"))
		format_conversion = "SE-PCULAW";
	    else  if (strsame(hs_optarg,"alaw"))
		format_conversion = "SE-ALAW";
	    else{
		fprintf(spfp,"Error: unknown output type ");
		fprintf(spfp,"option '%s'\n",hs_optarg);
		fprintf(spfp,usage,prog,prog);
		goto FATAL_EXIT;
	    }
	    break;
	  default:
	    fprintf(spfp,"Error: unknown flag -%c\n",c);
	    fprintf(spfp,usage,prog,prog);
	    goto FATAL_EXIT;
	}

    if (sp_verbose > 0) fprintf(spfp,"%s Version: %s, %s\n",prog,WEDIT_VERSION,sp_get_version());

    if (wes_script != CNULL)
	if (process_wes_script(wes_script,prog) != 0)
	    goto FATAL_EXIT;

    if ((wes_script == CNULL) && (argc - hs_optind != 2)){
	fprintf(spfp,"Error: Requires 2 filename arguements\n");
	fprintf(spfp,usage,prog,prog);
	goto FATAL_EXIT;
    }

    if (argc - hs_optind == 2){	
	filein=argv[hs_optind];
	fileout=argv[hs_optind+1];
	
	if (out_dir != CNULL && strsame(fileout,"-")){
	    fprintf(spfp,"Error: Output directory can not be defined "
		    "when the output goes to stdout\n");
	    fprintf(spfp,usage,prog,prog);
	    goto FATAL_EXIT;
	}

	strcpy(sdmod,format_conversion);
	if (channel_conversions != CNULL){
	    strcat(sdmod,":CH-");
	    strcat(sdmod,channel_conversions);
	    mtrf_free(channel_conversions);
	}
	
	if (wedit_openfile("",filein,&sp_in,&channel_count,
			   &sample_rate,&sample_count) != 0)
	    goto FATAL_EXIT;

	/* if times were used on the command line, convert them to samples */
	if (! use_sample) {
	    if (sp_verbose > 1) printf("Converting times from %5.2f:%5.2f  to  ",begin_time,end_time);
	    if (begin_time > 0.0)   
		begin_sample = (double)sample_rate * (double)begin_time + 0.5;
	    if (end_time > 0.0)     
		end_sample = (double)sample_rate * (double)end_time + 0.5; 
	    else                    end_sample = (-1);
	    if (sp_verbose > 1) printf("samples from %ld:%ld\n",begin_sample,end_sample);
	}
	
	/* Set the end sample to sample_count IF it was not set */
	if (end_sample == (-1))
	    end_sample = sample_count;
	
	if (wedit_make_file(sp_in,"",filein,(out_dir==CNULL)?"":out_dir,
			    fileout,"",channel_count,
			    sample_rate,sample_count,
			    sdmod,begin_sample,end_sample,0) != 0)
	    goto FATAL_EXIT;
	goto CLEAN_UP;
    } else 
	goto CLEAN_UP;
	
  FATAL_EXIT:
    exit_val = 1;
    
  CLEAN_UP:
    if (wedit_closefile(prog,filein,&sp_in) != 0)
	exit_val = 1;
    if (wes_script != CNULL) mtrf_free(wes_script);
    if (out_dir != CNULL) mtrf_free(out_dir);
    
    exit(exit_val);
}

int wedit_closefile(char *prog, char *filein, SP_FILE **sp_in){
    if (*sp_in != SPNULL) {
	if (sp_close(*sp_in) != 0){
	    fprintf(spfp,"%s: sp_close() failed on input file '%s'.\n", prog,filein);
	    return 1;
	}
    }
    *sp_in = SPNULL;
    return 0;
}

int wedit_openfile(char *input_dir, char *filein, SP_FILE **sp, SP_INTEGER *channel_count, SP_INTEGER *sample_rate, SP_INTEGER *sample_count){
    SP_FILE *sp_in = SPNULL; 
    char inname[500];

    sprintf(inname,"%s%s%s",input_dir,(*input_dir=='\0')?"":"/",filein);

    if (*sp != SPNULL){
	/* if the filename is the same, don't close the file */
	if (! strsame((*sp)->read_spifr->status->external_filename,inname)){
	    if (wedit_closefile(prog,inname,sp) != 0)
                return(1);
	} else 
	    return(0);
    }

    if ((sp_in=sp_open(inname,"r")) == SPNULL){
	fprintf(spfp,"%s: Unable to open file '%s'\n",prog,
		(strsame(inname,"-") ? "stdin" : inname ));
	sp_print_return_status(spfp);
	goto FATAL_EXIT;
    }

    /* get the channel count from the input file */
    if (sp_h_get_field(sp_in,"channel_count",T_INTEGER,
		       (void *)channel_count) > 0){
	if (sp_get_return_status() != 104){
	    fprintf(spfp,"Error: Unable to get the '%s' ","channel_count");
	    fprintf(spfp,"field from file '%s'\n",
		    (strsame(inname,"-") ? "stdin" : inname ));
	    sp_print_return_status(spfp);
	    goto FATAL_EXIT;
	}
    }
    if (sp_h_get_field(sp_in,"sample_rate",T_INTEGER,
		       (void *)sample_rate) > 0){
	if (sp_get_return_status() != 104){
	    fprintf(spfp,"Error: Unable to get the '%s' ","sample_rate");
	    fprintf(spfp,"field from file '%s'\n",
		    (strsame(inname,"-") ? "stdin" : inname ));
	    sp_print_return_status(spfp);
	    goto FATAL_EXIT;
	}
    }
    if (sp_h_get_field(sp_in,"sample_count",T_INTEGER,
		       (void *)sample_count) > 0){
	if (sp_get_return_status() != 104){
	    fprintf(spfp,"Error: Unable to get the '%s'","sample_count");
	    fprintf(spfp," field from file '%s'\n",
		    (strsame(inname,"-") ? "stdin" : inname ));
	    sp_print_return_status(spfp);
	    goto FATAL_EXIT;
	}
    }
    
    *sp = sp_in;
    return 0;

  FATAL_EXIT:
    if (sp_in != SPNULL) sp_close(sp_in);
    *sp = SPNULL;
    return 100;
}

int wedit_make_file(SP_FILE *sp_in, char *indir, char *filein, char *out_dir, char *fileout, char *out_ext, SP_INTEGER channel_count, SP_INTEGER sample_rate, SP_INTEGER sample_count,char *sdmod,SP_INTEGER begin_sample, SP_INTEGER end_sample, int show_status){
    SP_FILE *sp_out = SPNULL;
    SP_REAL spreal;
    char *read_buf = CNULL;
    int n1, exit_val, samples_read=0;
    char data_origins[200];
    SP_STRING str;
    char *fname;
    char *inname;
    
    fname = mtrf_strdup(rsprintf("%s%s%s%s%s",out_dir,
				 (*out_dir != '\0')?"/":"",fileout,
				 (*out_ext != '\0')?".":"",out_ext));
    inname = mtrf_strdup(rsprintf("%s%s%s",indir,
				 (*indir != '\0')?"/":"",filein));

    if (show_status > 0) 
	fprintf(spfp,"Building %s from input file %s from samples %ld to %ld\n",
		fname,inname,begin_sample,end_sample);


    /* create the output file */
    if ((sp_out=sp_open(fname,"w")) == SPNULL){
	fprintf(spfp,"%s: Unable to open output file '%s'\n",prog,
		(strsame(fname,"-") ? "stdout" : fname ));
	sp_print_return_status(spfp);
	goto FATAL_EXIT;
    }
    /* duplicate the input sphere file's header */
    if (sp_copy_header(sp_in,sp_out) != 0){
	fprintf(spfp,"%s: Unable to duplicate the input file header\n",prog);
	sp_print_return_status(spfp);
	goto FATAL_EXIT;
    }

    if (begin_sample != 0 || end_sample != sample_count){
	/* Add start_time and end_time to the header fields */
	spreal = (SP_REAL)begin_sample / (SP_REAL)sample_rate;
	if (sp_h_set_field(sp_out,"start_time",T_REAL,(void *)&spreal) != 0){
	    fprintf(spfp,"Warning: sp_h_set_field failed on 'start_time'\n");
	    sp_print_return_status(spfp);
	}    
	spreal = (SP_REAL)end_sample / (SP_REAL)sample_rate;
	if (sp_h_set_field(sp_out,"end_time",T_REAL,(void *)&spreal) != 0){
	    fprintf(spfp,"Warning: sp_h_set_field failed on 'end_time'\n");
	    sp_print_return_status(spfp);
	}    
	
	/* set the 'data_origins' field */
	*data_origins = '\0';
	if (sp_h_get_field(sp_out,"database_id",T_STRING,(void *)&str) == 0){
	    strcat(data_origins,str);
	    free(str);
	}
	if (sp_h_get_field(sp_out,"database_version", T_STRING,(void *)&str) == 0){
	    if (*data_origins != '\0')	strcat(data_origins,",");
	    strcat(data_origins,str);
	    free(str);
	}
	if (sp_h_get_field(sp_out,"utterance_id",T_STRING,(void *)&str) == 0)
	    ;
	else if (sp_h_get_field(sp_out,
				"conversation_id",T_STRING,(void *)&str)==0)
	    ;
	else  /* default to the filename */
	    str = mtrf_strdup(filein);
	if (*data_origins != '\0') strcat(data_origins,",");
	strcat(data_origins,str);
	free(str);
	
	/* Set the new 'data_origins' field */
	if (sp_h_set_field(sp_out,"data_origins",
			   T_STRING,(void *)&data_origins) != 0){
	    fprintf(spfp,"Warning: sp_h_set_field failed on 'data_origins'\n");
	    sp_print_return_status(spfp);
	}    
    }

    /* convert the file using the format_conversion field */
    if (sp_set_data_mode(sp_out,sdmod) != 0){
	fprintf(spfp,"%s: Unable to set data mode to '%s' on file '%s'\n",
		prog,sdmod,(strsame(fname,"-") ? "stdout" : fname ));
	sp_print_return_status(spfp);
	goto FATAL_EXIT;
    }

    /* delete the sample_checksum of the times have changed */
    if ((begin_sample != 0) || (end_sample != sample_count))
	if (sp_h_delete_field(sp_out,SAMPLE_CHECKSUM_FIELD) > 100){
	    fprintf(spfp,"%s: Unable to delete sample checksum field",prog);
	    sp_print_return_status(spfp);
	    goto FATAL_EXIT;
	}

    /* allocate memory for the reading buffer */
    if ((read_buf=(char *)sp_data_alloc(sp_in,BUF_SAMPLES)) == CNULL){
	fprintf(spfp,"%s: Unable to allocate read buffer\n",prog);
	sp_print_return_status(spfp);
	goto FATAL_EXIT;
    }

    if (sp_seek(sp_in,begin_sample,0) != 0){
	fprintf(spfp,"Error: sp_seek() failed\n");
	sp_print_return_status(spfp);
	goto FATAL_EXIT;
    }
    samples_read = begin_sample;

    /* begin processing the file */
    do {
	int samp_to_get;
	if (samples_read + BUF_SAMPLES < end_sample)
	    samp_to_get = BUF_SAMPLES;
	else
	    samp_to_get = end_sample - samples_read + 1;
	if (samp_to_get == 0){
	    break;
	}
	
	n1 = sp_read_data((char *)read_buf,samp_to_get,sp_in);
	if (n1 == 0 && sp_error(sp_in) != 0) {
	    fprintf(spfp,"%s: sp_error() returned Non-Zero code\n",prog);
	    sp_print_return_status(spfp);
	    goto FATAL_EXIT;
	}
	samples_read += n1;
	if (sp_write_data((char *)read_buf,n1,sp_out) != n1){
	    fprintf(spfp,"%s: Unable to write block to output file\n",prog);
	    sp_print_return_status(spfp);
	    goto FATAL_EXIT;
	}
    } while (n1 > 0);
    /* everything went OK */
    exit_val = 0;
    goto CLEAN_UP;

  FATAL_EXIT:
    exit_val = 1;

  CLEAN_UP:

    if (sp_in != SPNULL) {
	if (read_buf != CNULL) sp_data_free(sp_in,read_buf);
    }
    if (sp_out != SPNULL)
	if (sp_close(sp_out) != 0){
	    fprintf(spfp,"%s: sp_close() failed on output file '%s'.\n",
		    prog,fname);
	    exit_val = 1;
	}
    
    if (exit_val == 1)
	if ((fname != CNULL) && ! strsame(fname,"-")) unlink(fname);

    mtrf_free(fname);
    mtrf_free(inname);

    return(exit_val);
}

#define BUF_WIDTH 300

int process_wes_script(char *wes_script, char *prog){
    char *out_dir = CNULL, *def_out_dir = "";
    char *out_sdm = CNULL, *def_out_sdm = "SBF-ORIG";
    char *out_ext = CNULL, *def_out_ext = "";
    char *units =   CNULL, *def_units   = "sample";
    char *input_dir = CNULL, *def_input_dir = "";
    char *number1 = CNULL, *number2 = CNULL, *outfile = CNULL, *infile = CNULL;
    SP_INTEGER channel_count, sample_count, sample_rate;
    FILE *fp;
    char read_buf[BUF_WIDTH];
    SP_FILE *sp = SPNULL;
    int i, errors = 0;

    if (sp_verbose > 1)
	fprintf(spfp,"Executing w_edit Script '%s'\n",wes_script);

    for (i=0; i<2; i++){
	/* i == 0 --> pre-parse the file */
	/* i == 1 --> execute the file */
	if (sp_verbose > 1)
	    if (i == 0)
		fprintf(spfp,"Pre-parsing w_edit script\n");
	    else
		fprintf(spfp,"Executing commands\n");

	if ((fp = fopen(wes_script,"r")) == NULL){
	    fprintf(spfp,"Error: Unable to open script file '%s'\n",
		    wes_script);
	    return(1);
	}

	while (safe_fgets(read_buf,BUF_WIDTH,fp) != NULL){
	    strip_newline(read_buf);
	    if (*read_buf == '#')
		; /* ignore the comment line */
	    else if (is_empty(read_buf)) 
		; /* ignore the empty line containing only whitespace */
	    else if (*read_buf == '*'){
		if (parse_info_line(read_buf,&out_dir, &out_sdm, &out_ext, 
				    &units, &input_dir, &sp, i==0,prog) != 0)
		    errors ++;
	    } else {
		/* This must be a segment definition */
		if (parse_segment_def(read_buf,&infile,&number1,&number2,
				      &outfile,units) != 0){
		    errors ++;
		} else if (i == 1)
		    if (build_output_file((out_dir==CNULL)?def_out_dir:out_dir,
					  (out_sdm==CNULL)?def_out_sdm:out_sdm,
					  (out_ext==CNULL)?def_out_ext:out_ext,
					  (units==CNULL)?def_units:units,
					  (input_dir==CNULL)?def_input_dir:
					                     input_dir,
					  &sp, &channel_count, &sample_rate,
					  &sample_count,infile,
					  number1,number2,outfile) != 0)
			errors++;
	    }
	    if (i == 1 && errors > 0){
		fprintf(spfp,"**** Aborting execution ****\n");
		return(1);
	    }
	}
	fclose(fp);
	if (sp != SPNULL && wedit_closefile(prog,"Final File",&sp) != 0)
	    errors++;
	if (out_dir != CNULL)    { mtrf_free(out_dir); out_dir = CNULL;}
	if (out_sdm != CNULL)    { mtrf_free(out_sdm); out_sdm = CNULL;}
	if (out_ext != CNULL)    { mtrf_free(out_ext); out_ext = CNULL;}
	if (units   != CNULL)    { mtrf_free(units);   units   = CNULL;}
	if (infile  != CNULL)    { mtrf_free(infile);  infile = CNULL;}
	if (input_dir != CNULL)  { mtrf_free(input_dir); input_dir = CNULL;}
	if (number1 != CNULL)    { mtrf_free(number1); number1 = CNULL; }
	if (number2 != CNULL)    { mtrf_free(number2); number2 = CNULL; }
	if (outfile != CNULL)    { mtrf_free(outfile); outfile = CNULL; }
	if (i == 0 && errors > 0){
	    fprintf(spfp,"**** %d Errors were located in the script"
		    " file, please correct and re-run ***\n",errors);
	    return(1);
	}	
    }

    return(0);
}

	
#define FIELD_WIDTH 50
int build_output_file(char *out_dir, char *out_sdm, char *out_ext, char *units,
		      char *input_dir, SP_FILE **sp, 
		      SP_INTEGER *channel_count, SP_INTEGER *sample_rate,
		      SP_INTEGER *sample_count, char *infile, char *number1, 
		      char *number2, char *outfile){
    static int def_num = 0;
    int begin_sample, end_sample;
    char *outname;

    if (sp_verbose >= 5){
	fprintf(spfp,"out_dir: '%s' ",out_dir);
	fprintf(spfp,"out_sdm: '%s' ",out_sdm);
	fprintf(spfp,"out_ext: '%s' ",out_ext);
	fprintf(spfp,"units: '%s' ",units);
	fprintf(spfp,"input_dir: '%s'\n",input_dir);
	fprintf(spfp,"infile: '%s' number1: '%s' number2: '%s' outfile: '%s'\n",
		infile,number1, number2, (outfile==CNULL)?"DEFAULT":outfile);
    }

    if (wedit_openfile(input_dir,infile,sp,channel_count,
		       sample_rate,sample_count) != 0)
	return(1);
    def_num ++;

    /* figure out the sample measurments */
    if (strsame(units,"sample")){
	begin_sample = atoi(number1);
	end_sample   = atoi(number2);
    } else {
	begin_sample = (int)((double)*sample_rate * atof(number1) + 0.5);
	end_sample   = (int)((double)*sample_rate * atof(number2) + 0.5);
    }
    
    /* create the output file name */
    if (outfile != CNULL){
	alloc_singarr(outname,strlen(outfile) + 1,char);
	strcpy(outname,outfile);
    } else {
	alloc_singarr(outname,strlen(infile)+5,char);
	sprintf(outname,"%s.%04d",infile,def_num);
    }
    if (wedit_make_file(*sp,input_dir,infile,out_dir,
			outname,out_ext,*channel_count,
			*sample_rate,*sample_count,
			out_sdm,begin_sample,end_sample,sp_verbose) != 0)
	return(1);
    mtrf_free(outname);
    return(0);
}


int parse_segment_def(char *read_buf, char **infile, char **number1,
		      char **number2, char **outfile, char *units){
    char *p = read_buf;
    int l;

    if (*number1 != CNULL) { mtrf_free(*number1); *number1 = CNULL; }
    if (*number2 != CNULL) { mtrf_free(*number2); *number2 = CNULL; }
    if (*outfile != CNULL) { mtrf_free(*outfile); *outfile = CNULL; }
    if (*infile != CNULL)  { mtrf_free(*infile);  *infile = CNULL; }

    /* THE FIRST TOKEN */
    if (*(p += strspn(p," \t\n")) == '\0'){
	fprintf(spfp,"Error: no data in segment line '%s'\n",read_buf);
	return(1);
    }
    if ((l = strcspn(p," \n\t")) == 0){
	fprintf(spfp,"Error: no first number in segment line '%s'\n",
		read_buf);
	return(1);
    }
    alloc_singarr(*infile,l+1,char);
    strncpy(*infile,p,l);    (*infile)[l] = '\0';
    p += l;

    /* THE FIRST NUMBER TOKEN */
    if (*(p += strspn(p," \t\n")) == '\0'){
	fprintf(spfp,"Error: no data in segment line '%s'\n",read_buf);
	return(1);
    }
    if ((l = strcspn(p," \n\t")) == 0){
	fprintf(spfp,"Error: no second number in segment line '%s'\n",
		read_buf);
	return(1);
    }
    alloc_singarr(*number1,l+1,char);
    strncpy(*number1,p,l);    (*number1)[l] = '\0';
    p += l;

    /* THE SECOND NUMBER TOKEN */
    if (*(p += strspn(p," \t\n")) == '\0'){
	fprintf(spfp,"Error: no data in segment line '%s'\n",read_buf);
	return(1);
    }
    if ((l = strcspn(p," \n\t")) == 0){
	fprintf(spfp,"Error: no second number in segment line '%s'\n",
		read_buf);
	return(1);
    }
    alloc_singarr(*number2,l+1,char);
    strncpy(*number2,p,l);    (*number2)[l] = '\0';
    p += l;

    /* THE OPTIONAL FILE NAME */
    if (*(p += strspn(p," \t\n")) != '\0'){
	if ((l = strcspn(p," \n\t")) == 0){
	    fprintf(spfp,"Error: missing file name in segment line '%s'\n",
		    read_buf);
	    return(1);
	}
	alloc_singarr(*outfile,l+1,char);
	strncpy(*outfile,p,l);    (*outfile)[l] = '\0';
	p += l;
    }

    if (units == CNULL || strsame(units,"sample")){
	if (is_integer(*number1) == 0){
	    fprintf(spfp,"Error: first number '%s' is NOT an integer '%s'\n",
		    *number1,read_buf);
	    return(1);
	}
	if (is_integer(*number2) == 0){
	    fprintf(spfp,"Error: second number '%s' is NOT an integer '%s'\n",
		    *number2,read_buf);
	    return(1);
	}
    } else {
	if (is_number(*number1) == 0){
	    fprintf(spfp,"Error: first number '%s' is NOT a float '%s'\n",
		    *number1,read_buf);
	    return(1);
	}
	if (is_number(*number2) == 0){
	    fprintf(spfp,"Error: second number '%s' is NOT a float '%s'\n",
		    *number2,read_buf);
	    return(1);
	}
    }
    return(0);
}	    

int parse_info_line(char *read_buf, char **out_dir, char **out_sdm,
		    char **out_ext, char **units, char **input_dir,
		    SP_FILE **sp, int parse_only, char *prog){
    char field_name[FIELD_WIDTH], field_value[FIELD_WIDTH];
    char *p = read_buf;
    int l;

    /* first parse the input line */
    if (*(p += strspn(p," *\t\n")) == '\0'){
	fprintf(spfp,"Error: no field name on info line '%s'\n",
		read_buf);
	return(1);
    }
    if ((l = strcspn(p," \n\t=")) == 0){
	fprintf(spfp,"Error: no equal sign on info line '%s'\n",
		read_buf);
	return(1);
    }
    strncpy(field_name,p,MIN(FIELD_WIDTH,l));
    field_name[MIN(FIELD_WIDTH,l)] = '\0';
    p += l;
    if (*(p += strspn(p," =\t\n")) == '\0'){
	field_value[0] = '\0';
	if (! strsame(field_name,"no_extension")){
	    fprintf(spfp,"Error: missing field value on info line '%s'\n",
		    read_buf);
	    return(1);
	}
    } else {
	if ((l = strcspn(p," \n\t=")) == 0){
	    fprintf(spfp,"Error: no field value on info line '%s'\n",
		    read_buf);
	    return(1);
	}
	strncpy(field_value,p,MIN(FIELD_WIDTH,l));
	field_value[MIN(FIELD_WIDTH,l)] = '\0';
    }

    if (sp_verbose >= 6) fprintf(spfp,"INFO: name '%s' val '%s'\n",
					 field_name,field_value);
    if (strsame(field_name,"output_directory")){
	if (*out_dir != CNULL) mtrf_free(*out_dir);
	*out_dir = mtrf_strdup(field_value);
    } else if (strsame(field_name,"output_data_mode")){
	if (*out_sdm != CNULL) mtrf_free(*out_sdm);
	*out_sdm = mtrf_strdup(field_value);
    } else if (strsame(field_name,"extension")){
	if (*out_ext != CNULL) mtrf_free(*out_ext);
	*out_ext = mtrf_strdup(field_value);
    } else if (strsame(field_name,"no_extension")){
	if (*out_ext != CNULL) mtrf_free(*out_ext);
	*out_ext = CNULL;
    } else if (strsame(field_name,"units")){
	if (strsame(field_value,"sample") ||
	    strsame(field_value,"second")){
	    if (*units != CNULL) mtrf_free(*units);
	    *units = mtrf_strdup(field_value);
	} else {
	    fprintf(spfp,"Error: Unrecognized unit type on info line '%s'\n",
		    read_buf);
	    return(1);
	}
    } else if (strsame(field_name,"input_directory")){
	if (*input_dir != CNULL) mtrf_free(*input_dir);
	*input_dir = mtrf_strdup(field_value);
    } else {
	fprintf(spfp,"Error: Unrecognized information line '%s'\n",
		read_buf);
	return(1);
    }
    return(0);
}
