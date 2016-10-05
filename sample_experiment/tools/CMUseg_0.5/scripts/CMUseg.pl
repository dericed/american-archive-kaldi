#!/usr/local/bin/perl -w

# File:  CMUseg.pl
$Version="1.1";

# History:  
#    version 1.0  Released Aug 26, 1997
#    version 1.1  Released Oct 22, 1997
#        - Fixed an error in the documented frame rate.
#        - Fixed a problem with underscores in the UEM file name
#        - Converted the output of UTT_Kseg to limit the number of break
#          points to 20 per line.  This avoids buffer overflow problems 
#          later in the pipeline.

$usage ="Usage: CMUseg.pl <OPTIONS> -t tmpdir -r root_dir in.uem out.pem\n".
"Version: $Version\n".
"Desc:  CMUseg.pl performs an acoustic segmentation on a NIST SPHERE headered\n".
"       waveform file.  The script will read a NIST UEM (Unpartitioned Evaluation\n".
"       Map) file 'in.uem' and perform the following functions:	\n".	
"          1: Parameterizes the file with 'wave2mfcc'	\n".
"          2: Locates changes in acoustic conditions with 'UTT_Kseg'\n".
"          3: Finds places to legally draw a break with 'UTT_findsil'\n".
"          4: Detects telephone BW speech with 'UTT_gauss_class'\n".
"          5: Clusters the utterances for MLLR adaptation 'UTT_cluster'\n".
"\n".
"       The scripts uses the temporary directory to hold the parameterize\n".
"       waveform file and other intermediate files.  The program requires 20 Mb.\n".
"       of disk space per hour of segmented speech.\n".
"\n".
"       The output of the program will be a NIST format PEM (Partitioned \n".
"       Evaluation Map) file in 'out.pem'\n".
"\n".
"File formats:\n".
"       A NIST UEM file contains a series of waveform segments.  There is one\n".
"       segment record per line line and contains 4 fields:\n".
"       <filename> <channel ('1' only)> <begin time> <end time>\n".
"\n".
"          <filename> is the 'basename' of the sphere audio file\n".
"           Comment lines begin with a double semi-colon ';;'\n".
"\n".
"       A NIST PEM file contains a series of waveform segments.  Each segment\n".
"       record is contained on a signle line and contains 6 fields:\n".
"       <filename> <speaker> <channel> <begin time> <end time> <F_condition>\n".
"\n".
"          <filename> is the 'basename' of the sphere audio file\n".
"          <speaker> will identify group into which the segment was clustered.\n".
"          <F_condition> will be either F2 (for telephone bandwadth speech) or F0\n".
"                        for all other speech.\n".
"          Comment lines begin with a double semi-colon ';;'\n".
"\n".
"Required arguments:\n".
"    -r root_dir Pathname to the source code directory of the CMUseg package\n".
"    -t tmpdir   Use the specified temporary directory for intermediate files.\n".
"                !!!!!   THIS DIRECTORY MUST BE CLEANED UP BY THE USER   !!!!!\n".
"\n".
"OPTIONS\n".
"    -a arch     Use the compiled executables for the architecture 'arch'.\n".
"                Either 'hpux, alpha_osf1, sun or linux'.  Default the\n".
"                value is suppled by the  envorinment variable '\$arch'\n".
"    -e wavext   Use the extension 'wavext' on the files specified in the UEM\n".
"                file.  The default extension is 'sph'.\n".
"    -w wavdir   Look for the waveforms specified in the directory 'wavdir'.  The\n".
"                default directory is '.'\n".
"    -m max      Set the Maximum length for a segment to 'max' frames.  Each\n".
"                frame is 10 ms in duration.  Default size is unlimited\n".
"    -v          Write verbose status messages.  Otherwise run silently\n".
"    -c          Show the executed commands before they are executed\n".
"Caveats:\n".
"    - This version of the script only works with single channel PCM audio data\n";


####################  GLOBAL VARIABLES   #######################
$Arch=$ENV{"arch"};
$SphExt="sph";
$WavDir=".";
$TmpDir=".";
$CMUsegDir="";
$UEM = "";
$PEM = "";
$Vb = 0;
@UEM_Segs = ();
$MaxLen="";
$comline="$0 ".join(" ",@ARGV);
$ShowCom=0;
####################  End of Global Variables ##################


###############################################################
################            MAIN               ################

&process_commandline();
&load_UEM();
&parameterize_speech();
&run_UTT_Kseg();
&run_UTT_findsil();
&run_UTT_gauss_class();
&run_UTT_cluster();
&build_pem();

exit 0;

################         END OF MAIN           ################
###############################################################

################        Subroutines            ################


### Get the command line arguments
###
sub process_commandline{
    require "getopts.pl";
    &Getopts('vt:w:e:r:m:ca:');

    if (defined($opt_a)) {  $Arch=$opt_a; }

    if (!defined($opt_r)) {  die("$usage\n\nError: Path to CMUseg installation directory is required"); }
    $CMUsegDir=$opt_r;
    
    if (!defined($opt_t)) {  die("$usage\n\nError: The temporary directory is required"); }
    $TmpDir=$opt_t;
    if (! -d $TmpDir) { die("Error: Temporary directory '$TmpDir' does not exit"); }
    if (! -w $TmpDir) { die("Error: Temporary directory '$TmpDir' is not writeable"); }
    
    if (defined($opt_v))  {  $Vb = 1; $opt_v = 1; }
    if (defined($opt_w))  {  $WavDir = $opt_w; }
    if (defined($opt_e))  {  $SphExt = $opt_e; }  
    if (defined($opt_m))  {  $MaxLen = "-m $opt_m"; }
    if (defined($opt_c))  {  $ShowCom = 1; $opt_c = 1; }
  
    if ($#ARGV >  1){        die("$usage\n\nError: too many command line arguments"); }
    if ($#ARGV != 1){        die("$usage\n\nError: in.uem and out.pem not specified"); }
    
    $UEM=$ARGV[0];
    $PEM=$ARGV[1];
}

### Read the UEM file, creating the @Segments array.  Check each
### segment to make sure that:
###       the waveform is available
###       the times are legal
###       
sub load_UEM{
    local(@line);
    local($file, $chan, $bt, $et); 
    local($fp);

    if ($Vb) { print("Loading and Checking UEM file $UEM\n"); }
    open(INF,"<$UEM") || die("Error: Open of UEM file '$UEM' Failed.\n"); 
    while (<INF>){
	if ($_ !~ /^;;/) {
	    s/^\s+//;
	    chop;
	    @line = split;
	    ($file, $chan, $bt, $et) = split;
	    
	    if (@line != 4)  { die("Error: UEM segment '$_' has more than 4 fields"); }
	    if ($bt > $et)   { die("Error: UEM segment '$_' has a begin time larger than the end time"); }
	    if ($bt < 0.0)   { die("Error: UEM segment '$_' has a begin time less than 0.0"); }
	    if ($chan != 1)  { die("Error: UEM segment '$_' has a channel id != 1"); }

	    ### Check the existence of the waveform
	    $fp = "${WavDir}/${file}.${SphExt}";
	    if (! -r $fp) { die("Error: UEM Segment '$_', can not locate waveform $fp"); }
	    
	    ### make the table of Segments
	    push(@UEM_Segs, "$file $chan $bt $et");
	}
    }
    close(INF);
}

###  Compute the MFCC files for each segment's waveform file
sub parameterize_speech{
    local($seg, $file, $chan, $bt, $et);
    local($wavfp, $mfcfp, $mfclog);

    if ($Vb) { print "Computing MFCC files with wave2mfcc\n"; }
    if ($Arch eq "alpha_osf1") {
	print STDERR "Warning: OSF1 users may see an \"unaligned process\" message\n";
	print STDERR "         while computing the MFCC files.  Please ignore this warning."
    }
    foreach $seg(@UEM_Segs){
	($file, $chan, $bt, $et) = split(/\s+/,$seg);
	$wavfp = "${WavDir}/${file}.${SphExt}";
	$mfcfp = "${TmpDir}/${file}.mfc";
	$mfclog = "${TmpDir}/${file}.mfc.log";
	if (! -e $mfcfp) {
	    if ($Vb) { print "   Computing MFCC File for $file\n"; }
	    local ($com) = "${CMUsegDir}/bin/${Arch}/wave2mfcc -i $wavfp -sphere ".
		"-sphinx -o $mfcfp 1> $mfclog 2>&1";
	    if ($ShowCom) {print "Exec: $com\n";}
	    $exit = system $com;
	    if ($exit != 0) { 
		unlink ($mfcfp);
		die("Error: wave2mfcc Failed. See $mfclog");
	    }
	}

    }

}

### 
sub run_UTT_Kseg{
    local($seg, $file, $chan, $bt, $et);
    local($mfcfp);
    local($ctlfile) = "$TmpDir/UTT_Kseg.ctl";
    local($UTT_Kseg_out) = "$TmpDir/UTT_Kseg.out";
    local($tmpfile) = "$TmpDir/UTT_Kseg.out.tmp";
    local($logfile) = "$TmpDir/UTT_Kseg.log";
    local($exit);
    local($com);

    if ($Vb) { print "Running UTT_Kseg, Output Going to $UTT_Kseg_out\n"; }

    if (-f $UTT_Kseg_out) { unlink($UTT_Kseg_out); }

    foreach $seg(@UEM_Segs){
	open(CTL,">$ctlfile") || die ("Unable to open control file $ctlfile");
	($file, $chan, $bt, $et) = split(/\s+/,$seg);
	$mfcfp = "${TmpDir}/${file}.mfc";
	printf CTL "%s %6.0f %6.0f %s\n", $mfcfp, $bt * 100.0, $et * 100.0, $file;
	close CTL;

	if ($Vb) { printf("   Executing UTT_Kseg on Segment $seg\n"); }
	$com = "${CMUsegDir}/bin/${Arch}/UTT_Kseg -c $ctlfile ".
	    "-h 100 -w 250 -s 500 -v -m 0.05 -f 0.1 ".
		"-r $tmpfile 1>> $logfile 2>&1";
	if ($ShowCom) {print "Exec: $com\n";}
	$exit = system $com;
	if ($exit != 0) { 
	    unlink ($UTT_Kseg_out);
	    die("Error: UTT_Kseg Failed. See $logfile");
	}

	# capture the output files, reprocessing it to limit the number of 
	# break points per line
	open(TMP,$tmpfile) || die("Error: Failed to open the temporary output file"
				  ." generated by UTT_Kseg");
	open(OUT,">>$UTT_Kseg_out") || 
	    die("Error: Failed to open UTT_Kseg output file");
	while (<TMP>){
	    local(@a) = split;
	    while ($#a > 30){
		# Take the first set
		local(@x) = splice(@a,0,25);
		# fill back in the original array		
		splice(@a, 0, 0, ($x[0], $x[$#x], $x[2], $x[3])); 

		# Set the end frame to the last value in x
		$x[2] = $x[$#x];
		# remove the last item from @x
		splice(@x,$#x,1);
		# write out the chunk
		print OUT join(" ",@x)."\n";
	    }
	    if ($#a <= 30 && $#a > 0){
		print OUT join(" ",@a)."\n";
	    }
	}
	close(OUT);
	close(TMP);
	# system "cat $tmpfile >> $UTT_Kseg_out";
    }
}

### 
sub run_UTT_findsil{
    local($exit);
    local($com);
    local($UTT_Kseg_out) = "$TmpDir/UTT_Kseg.out";
    local($UTT_findsil_out) = "$TmpDir/UTT_findsil.out";
    local($logfile) = "$TmpDir/UTT_findsil.log";

    if ($Vb) { print "Running UTT_findsil, Output Going to $UTT_findsil_out\n"; }

    if (-f $UTT_findsil_out) { unlink($UTT_findsil_out); }

    $com = "${CMUsegDir}/bin/${Arch}/UTT_findsil -c $UTT_Kseg_out ".
	"-r $UTT_findsil_out -sw 7 -sW 200 -sn 200 -st 8 -sd 10 -v $MaxLen ".
	    " 1> $logfile 2>&1";
    if ($ShowCom) {print "Exec: $com\n";}
    $exit = system $com;
    if ($exit != 0) { 
	unlink ($UTT_findsil_out);
	die("Error: UTT_finsil Failed. See $logfile");
    }
}

### 
sub run_UTT_gauss_class{
    local($exit);
    local($com);
    local($UTT_findsil_out) = "$TmpDir/UTT_findsil.out";
    local($UTT_gauss_out) = "$TmpDir/UTT_gauss.out";
    local($logfile) = "$TmpDir/UTT_gauss.log";
    local($ctlfile) = "$TmpDir/UTT_gauss.ctl";

    if ($Vb) { print "Running UTT_gauss, Output Going to $UTT_gauss_out\n"; }

    if (-f $UTT_gauss_out) { unlink($UTT_gauss_out); }

    ## make the input ctl file
    open(CTL,">$ctlfile") || die("Unable to open UTT_gauss control file $ctlfile");
    open(FIS,"<$UTT_findsil_out") || die("Unable to open out file '$UTT_findsil_out' from UTT_findsil");
    while(<FIS>){
	local(@l, $i);
        @l=split;
	foreach $i(1 .. $#l-1) {
	    printf CTL "%s %s %s %s_%07d_%07d_%05d\n",$l[0],$l[$i],$l[$i+1],$l[0],$l[$i],$l[$i+1],$i;
	}
    }
    close CTL;
    close FIS;
    
    $com = "${CMUsegDir}/bin/${Arch}/UTT_gauss_class ".
	" -i $ctlfile -id $TmpDir -c ${CMUsegDir}/classes/h496clas ".
	    " -ie mfc -r $UTT_gauss_out -d ${CMUsegDir}/classes/h496dist ".
		" -v 1> $logfile 2>&1";
    if ($ShowCom) {print "Exec: $com\n";}
    $exit = system $com;
    if ($exit != 0) { 
	unlink ($UTT_gauss_out);
	die("Error: UTT_gauss_class Failed. See $logfile");
    }
}

### 
sub run_UTT_cluster{
    local($exit);
    local($com);
    local(%classes) = ();
    local(%segments) = ();
    local($UTT_gauss_ctl) = "$TmpDir/UTT_gauss.ctl";
    local($UTT_gauss_out) = "$TmpDir/UTT_gauss.out";
    local($UTT_cluster_8_out) = "$TmpDir/UTT_cluster_8.out";
    local($UTT_cluster_16_out) = "$TmpDir/UTT_cluster_16.out";
    local($UTT_cluster_8_ctl) = "$TmpDir/UTT_cluster_8.ctl";
    local($UTT_cluster_16_ctl) = "$TmpDir/UTT_cluster_16.ctl";
    local($UTT_cluster_8_log) = "$TmpDir/UTT_cluster_8.log";
    local($UTT_cluster_16_log) = "$TmpDir/UTT_cluster_16.log";
    

    if ($Vb) { print "Running UTT_cluster, Output Going to $UTT_cluster_8_out and $UTT_cluster_16_out\n"; }

    if (-f $UTT_cluster_8_out) { unlink($UTT_cluster_8_out); }
    if (-f $UTT_cluster_16_out) { unlink($UTT_cluster_16_out); }

    ### read in the output of gauss_cluster
    open(IN,"$UTT_gauss_out") || die("Unable to open classification file '$UTT_gauss_out'"); 
    while (<IN>){
	local($id,$class) = split;
	$classes{$id} = $class;
    }
    close(IN);

    ### read in the control file for gauss classification
    open(IN,"$UTT_gauss_ctl") || die("Unable to open classification file '$UTT_gauss_ctl'"); 
    while (<IN>){
	local($file, $bf, $ef, $id) = split;
	if ($classes{$id} =~ /^$/){ die("Error looking up id $id in '$UTT_gauss_ctl'"); }
	$segments{$id} = $_;
    }
    close(IN);
    
    ### build the control files for classification 
    open(C_8K,">$UTT_cluster_8_ctl") ||
	die("Unable to open 8Khz cluster control file '$UTT_cluster_8_ctl'");
    open(C_16K,">$UTT_cluster_16_ctl") ||
	die("Unable to open 16Khz cluster control file '$UTT_cluster_16_ctl'");

    foreach $id (sort (keys %segments)){
	if ($classes{$id} eq "P"){
	    print C_8K $segments{$id}; 
	} else {
	    print C_16K $segments{$id}; 	    
	}
    }
    close(C_8K);
    close(C_16K);

    #######  Finally, run cluster on the two control files

    $com = "${CMUsegDir}/bin/${Arch}/UTT_cluster -m 200 -t 0.03 -i $UTT_cluster_8_ctl ".
        " -d ${TmpDir} -e mfc -r $UTT_cluster_8_out ".
	    " -v 1> $UTT_cluster_8_log 2>&1";

    if ($ShowCom) {print "Exec: $com\n";}
    $exit = system $com;
    if ($exit != 0) { 
	unlink ($UTT_cluster_8_out);
	die("Error: UTT_cluster on 8Khz data Failed. See $UTT_cluster_8_log");
    }


    $com = "${CMUsegDir}/bin/${Arch}/UTT_cluster -m 1000 -t 0.055 -i $UTT_cluster_16_ctl ".
        " -d $TmpDir -e mfc -r $UTT_cluster_16_out ".
	    " -v 1> $UTT_cluster_16_log 2>&1";

    if ($ShowCom) {print "Exec: $com\n";}
    $exit = system $com;
    if ($exit != 0) { 
	unlink ($UTT_cluster_16_out);
	die("Error: UTT_cluster on 16Khz data Failed. See $UTT_cluster_16_log");
    }
}

###
sub build_pem{
    local($UTT_cluster_8_out) = "$TmpDir/UTT_cluster_8.out";
    local($UTT_cluster_16_out) = "$TmpDir/UTT_cluster_16.out";
    local(@segments) = ();
    local($seg);

    if ($Vb) { print "Building the final PEM file  $PEM\n"; } 

    # load in the cluster information
    open(IN,"$UTT_cluster_8_out") || 
	die("Unable to load the 8Khz cluster file '$UTT_cluster_8_out'\n");
    while(<IN>){
	local($id,$group) = split;
	push(@segments,sprintf("%s F2-%04d F2",$id,$group));
    }
    close(IN);

    open(IN,"$UTT_cluster_16_out") || 
	die("Unable to load the 16Khz cluster file '$UTT_cluster_16_out'\n");
    while(<IN>){
	local($id,$group) = split;
	push(@segments,sprintf("%s F0-%04d F0", $id,$group));
    }
    close(IN);
    
    open(OUT,">$PEM") || die("Unable to open PEM file '$PEM'");
    $date = `date`;
    print OUT ";; Automatically generated PEM file on $date";
    print OUT ";; Commandline: $comline\n";
    foreach $seg (sort(@segments)){
	local($id, $grp, $cond) = split(/\s+/,$seg);
	local($rid) = join("",reverse(split(//,$id)));
	local($ord, $ef, $bf, $file) = split(/_+/,$rid,4);
	$ef = join("",reverse(split(//,$ef)));
	$bf = join("",reverse(split(//,$bf)));
	$file = join("",reverse(split(//,$file)));
	printf OUT "%s 1 %s %8.2f %8.2f %s\n",$file,$grp,$bf/100.0,$ef/100.0,$cond;
    }
    close (OUT);
}

