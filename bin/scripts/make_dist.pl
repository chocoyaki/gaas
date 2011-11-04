#!/usr/bin/perl
#**
#*  @file  make_dist.pl
#*  @brief  Clean up CVS fields in DIET files, when building a distribution.
#*  @author Eddy Caron (Eddy.Caron@ens-lyon.fr)
#*  @section Licence 
#*    |LICENCE|
#****************************************************************************#
#* FIXME: Description of the file is to be updated...                       *#
#* Clean up CVS fields in DIET files, when building a distribution.         *#
#* This script takes at most one argument: "dev", set when building a       *#
#*  maintainer distribution. It reads Distribution_files.lst.               *#
#****************************************************************************#


use File::Path;
use File::Copy "cp";
use strict;
use Getopt::Long;
use Term::ANSIColor qw(:constants);
use List::Util qw(first max maxstr min minstr reduce shuffle sum);
$Term::ANSIColor::AUTORESET = 1;


&main();


#
# Main #
#
sub main(){
	# Distribution file list
	my $file_list = "bin/scripts/Distribution_files.lst";	
	# Rel. path to the script which processes templated files
	my $distrib_file_sh = "bin/scripts/distrib_file.sh";
	my ($major_version,$minor_version,$revision_version);
	my ($help,$version,$maintainer_mode,$check);
        &GetOptions("maintainer_mode"=>\$maintainer_mode,
		    "version=s"=>\$version,
		    "check"=>\$check,
                    "help|h"=>\$help);
	if ($help){
		&usage();
		exit(0);
	}
	if ($check){
		&checkDistFiles($file_list);
		exit(0);
	}
	if (!$version){
		&usage();
		exit(1);
	}
	if ($maintainer_mode){
		$version.="-dev";
	}
	($major_version,$minor_version,$revision_version)=split(/\./,$version);
	my $distdir="diet-".$version;
	print "DIET_VERSION $version\n";
	&checkAndCreateDistDir($distdir);
	&processDistFileList($distdir,$file_list,$distrib_file_sh,$maintainer_mode);
	&setVersion($distdir."/CMakeLists.txt",$major_version,$minor_version,$revision_version);
	&buildArchive($distdir);
	
}

### Function definition ###
#
# Usage
#
sub usage(){
	print "\n";
	print GREEN "Usage : $0 --version=<major>.<minor>.<revision>\n";
	print "\n";
	print GREEN "Required :\n";
	print "\t --version=<major>.<minor>.<revision> \n";
 	
 	print GREEN "Optional :\n";
 	print "\t --check check files listed in the distrib_file\n";
	print "\t and list file in the DIET source which doesn't appear\n";
 	print "\t --maintainer generate the devel archive of diet\n";
 	print "\t --h|help display this help message\n";
 	print GREEN "example :\n";
 	print "\t$0 --version=2.4.1\n";
	print "\t it will generate archive : ";
	print RED "diet-2.4.1.tar.gz\n";
 	print "\t$0 --version=2.4.1 -m\n";
	print "\t it will generate archive : ";
	print RED "diet-2.4.1-dev.tar.gz\n";
	print "\n";
}
#
# set version in the CMakeLists.txt of the distrib.
#
sub setVersion(){
	my ($dest_file,$major_version,$minor_version,$revision_version)=(@_);
	open( CMakeLists, '<', "CMakeLists.txt");
	open( DESTFILE, '>', "$dest_file");
	while (<CMakeLists>) {
	    my $line = $_;
	    chomp $line;
	    if (/_MAJOR_VERSION/ && /SET\(/ && !/\$/){
		my ($set,$var,$version) = split(/\s+/, $line);
		#printf ("$line\n");
		$line="$set $var $major_version )";
		#printf ("$line\n");
	    }
	    if (/_MINOR_VERSION/ && /SET\(/ && !/\$/){
		my ($set,$var,$version) = split(/\s+/, $line);
		#printf ("$line\n");
		$line="$set $var $minor_version )";
		#printf ("$line\n");
	    }
	    if (/_REVISION_VERSION/ && /SET\(/ && !/\$/){
		my ($set,$var,$version) = split(/\s+/, $line);
		#printf ("$line\n");
		$line="$set $var $revision_version )";
		#printf ("$line\n");
	    }
	    print DESTFILE $line."\n";
	}
	close(CMakeLists);
	close(DESTFILE);
}

sub checkAndCreateDistDir(){
	my ($distdir)=(@_);
	if( -d $distdir ){
	  print( "WARNING: $distdir exists, overwrite ? [y/N] " );
	  my $ans = <STDIN>;
	  if( $ans =~ /[yY]/ ){
	    rmtree $distdir;
	  } else {
	    print( "OK, I stop. Please rename $distdir and relaunch $0\n" );
	    exit 0;
	  }
	}
	mkdir $distdir;
}
#
# Process distribution file list
#
sub processDistFileList(){
	my ($distdir,$file_list,$distrib_file_sh,$maintainer_mode)=(@_);
	open( FILE, '<', $file_list ) or die( "Cannot open $file_list" );
	# File type:
	#  -1: unset
	#   0: templated file (to be processed by distrib_file.sh)
	#   1: untemplated file (to be put as is in distribution)
	#   2: templated maintainer file
	#   3: untemplated maintainer file
	my $file_type = -1;
	my $no_line = 0;
	while( <FILE> ){
	  $no_line++;
	  chomp;
	  SWITCH: {
	    if( /^#/ ){ # Line commented out
	      #print( "DEBUG: Line $no_line is commented out\n" );
	      last SWITCH;
	    }
	    if( /^ *$/ ){
	      #print( "DEBUG: Line $no_line is empty\n" );
	      last SWITCH;
	    }
	    if( /^\[templated\] *$/i ){
	      $file_type = 0;
	      #print( "DEBUG: Line $no_line activates templated files section\n" );
	      last SWITCH;
	    }
	    if( /^\[untemplated\] *$/i ){
	      $file_type = 1;
	      #print( "DEBUG: Line $no_line activates untemplated files section\n" );
	      last SWITCH;
	    }
	    if( /^\[devel_templated\] *$/i ){
	      $file_type = 2;
	      #print( "DEBUG: Line $no_line activates templated dev files section\n" );
	      last SWITCH;
	    }
	    if( /^\[devel_untemplated\] *$/i ){
	      $file_type = 3;
	      #print( "DEBUG: Line $no_line activates untemplated dev files section\n" );
	      last SWITCH;
	    }
	
	    #
	    # OK, we got a file to process
	    #
	    if( $file_type >= 2 && $maintainer_mode == 0 ) {
	      last SWITCH;
	    }
	
	    my $file = $_;
	    die( "Cannot access $file ($file_list:$no_line)" ) unless -e $file;
	    die( "No section defined before line $no_line" ) if( $file_type == -1 );
	    
	    # Get path and base names
	    my ( $file_dir, $file_base ) = $file =~ /(.*)\/(.*)/;
	    my $dstfile = $distdir."/".$file;
	    my $dstfile_dir =  $distdir."/".$file_dir;
	
	    # Create the path of the file in distribution tree
	    if( ! -d $dstfile_dir ){ &mkpath($dstfile_dir); }
	
	    # If the source file does not exists, abort.
	    #FIXME: Try to generate it with CMake (useful for docs, I think)
	    die( "Cannot access nor generate $file" ) if( ! -e $file );
	    # Copy or process file...
	    if( $file_type == 1 || $file_type == 3 ){ # untemplated file
	      cp($file, $dstfile);
	    } else {                                  # templated file
	      my @options;
	      my @args;
	      push @args, $file;
	      push @args, $dstfile;
	      my $status = system( "$distrib_file_sh", @options, @args );
	      die( "Formatting $file failed. Abort." ) if( $status != 0 );
	    }
	    
	    # "last SWITCH" == DUMMY statement...
	    my $dummy = 0;
	  }
	
	}
}
#
# Build archive
#
sub buildArchive(){
	my ($distdir)=(@_);
	my @options;
	my @args;
	push @options, "cf";
	push @args, $distdir.".tar";
	push @args, $distdir;
	system( "tar", @options, @args );
	@options = ();
	@args = ();
	push @options, "-9";
	push @args, $distdir.".tar";
	system( "gzip", @options, @args );
	rmtree( $distdir );
}
#
# get file from $file_list
#
sub getDistFileList(){
	my ($file_list)=(@_);
	# tab which contains all files which are defined in the $file_list file
	my (@files_list);
	# hash which contains a tab of all files which are defined in the $file_list file
	# keys : file_type name;
	my (%files_list_hash);
	my ($exit,@messages); # if $exit is set the program exit and print @messages;
	# tabs which of all file_type
	my (@files_templated,@files_untemplated,@file_templated_maintainer,@file_untemplated_maintainer);
	open( FILE, '<', $file_list ) or die( "Cannot open $file_list" );
	# File type:
	#  -1: unset
	#   0: templated file (to be processed by distrib_file.sh)
	#   1: untemplated file (to be put as is in distribution)
	#   2: templated maintainer file
	#   3: untemplated maintainer file
	my $file_type = -1;
	my $no_line = 0;
	while( <FILE> ){
	  $no_line++;	  
	  chomp;
	  SWITCH: {
	    if( /^#/ ){ # Line commented out
	      #print( "DEBUG: Line $no_line is commented out\n" );
	      last SWITCH;
	    }
	    if( /^ *$/ ){
	      #print( "DEBUG: Line $no_line is empty\n" );
	      last SWITCH;
	     }
	    if( /^\[templated\] *$/i ){
	      $file_type = 0;
	      #print( "DEBUG: Line $no_line activates templated files section\n" );
	      last SWITCH;
	    }
	    if( /^\[untemplated\] *$/i ){
	      $file_type = 1;
	      #print( "DEBUG: Line $no_line activates untemplated files section\n" );
	      last SWITCH;
	    }
	    if( /^\[devel_templated\] *$/i ){
	      $file_type = 2;
	      #print( "DEBUG: Line $no_line activates templated dev files section\n" );
	      last SWITCH;
	    }
	    if( /^\[devel_untemplated\] *$/i ){
	      $file_type = 3;
	      #print( "DEBUG: Line $no_line activates untemplated dev files section\n" );
	      last SWITCH;
	    }
	
	    my $file = $_;
	    unless( -e $file){
		push(@messages,"$no_line:$file_type $file cannot access ");
	    }
	    #die( "Cannot access $file ($file_list:$no_line)" ) unless -e $file;
	    if ( $file_type == -1){
		push(@messages,"No section defined before line $no_line)");
	    }
	    
	    # Get path and base names
	    my ( $file_dir, $file_base ) = $file =~ /(.*)\/(.*)/;
	    $file="./".$file;
	    if ($file_type >= 0){
		if (&tab_contain($file,\@files_list)==1){
			push (@messages,"$no_line:$file_type $file already defined in the file");
		}else{
			push(@files_list,$file);
		}
	    }	
	   SWITCH: {
		if($file_type == 0){
			push(@files_templated,$file);
		}
		if($file_type == 1){
			push(@files_untemplated,$file);
		}
		if($file_type == 2){
			push(@file_templated_maintainer,$file);
		}
		if($file_type == 3){
			push(@file_untemplated_maintainer,$file);
		}
	   }
	    # "last SWITCH" == DUMMY statement...
	    my $dummy = 0;
	  }	
	}
	if (scalar(@messages)){
		for my $m (@messages){print "$m\n";}
		exit 1;
	}
	$files_list_hash{"templated"}=\@files_templated;
	$files_list_hash{"untemplated"}=\@files_untemplated;
	$files_list_hash{"templated_maintainer"}=\@file_templated_maintainer;
	$files_list_hash{"untemplated_maintainer"}=\@file_untemplated_maintainer;
	return \@files_list,\%files_list_hash;
}
#
# get all dir which names is not contains in the unmatch tab
# go recursively into the initial $path.
# the var $dir contains the name of all visited dir
#
sub getAllDir(){
	my($path,$dirs,$unmatch) = @_;
	$path .= '/' if($path !~ /\/$/);
  	my @file_list=glob($path.'*');
	## loop through the files contained in the directory
	for my $eachFile (@file_list) {
		my ( $file_dir, $file_base ) = $eachFile =~ /(.*)\/(.*)/;
    		## if the file is a directory
	    	if(-d $eachFile && !&tab_contain($file_base,$unmatch)) {
			push(@$dirs,$eachFile);
		      	&getAllDir($eachFile,$dirs,$unmatch);
    		}
    	}	
}
#
# get all files which are located in all path contains in the $dir tab
# return the hash tab index by dir and tab of all file in this path
# return the tab of all files
#
sub getAllFiles(){
	my ($dirs)=(@_);
	my @all_files;
	my %hash_files;
	for my $dir (@$dirs){
		#print GREEN "$dir\n";
		my @dir_files;
		$dir .= '/' if($dir !~ /\/$/);
		my @file_list=glob($dir.'*');
		for my $f (@file_list){
			if (-f $f){				
				push(@all_files,$f);
				push(@dir_files,$f);
			}
		}
		$hash_files{$dir}=\@dir_files;
	}
	return (\%hash_files,\@all_files);
}

#
# TO DO in order to remplace the shell script
#
sub checkFile(){
		
}
sub checkDistFiles(){
	my ($file_list)=(@_);
	print "check\n";
	print "file_list=$file_list\n";
	my ($dist_files,$dist_hash_dir_files)=&getDistFileList($file_list);		

	my @unmatch = ("CVS");	
	my @src_dirs=("./");
	&getAllDir("./",\@src_dirs,\@unmatch);	
	my ($src_hash_dir_files,$src_files)=&getAllFiles(\@src_dirs);	
	my $missed=&check_missing($src_files,$dist_files);
	print "##########################################################\n";
	print "# These files are not in the distrib                     #\n";
	print "# please add them in $file_list  #\n";
	print "# if needed                                              #\n";
	print "##########################################################\n";
	for my $i (@$missed){
		#if ($i=~/\.c$/ || $i=~/\.cc$/ || $i=~/\.h$/ || $i=~/\.hh$/){
		if ($i=~/^\.\/src./){
			if ($i=~/\.c$/ ||
			 	$i=~/\.cc$/ ||
				$i=~/\.java$/  ||
				$i=~/\.h$/  ||
				$i=~/\.hh$/ ){
				print BOLD RED "\t$i\n";
			}else{
				print RED "\t$i\n";
			}
		}else{
			print WHITE "\t$i\n";
		}
	}
	print "TOTAL = ".scalar(@$missed)."\n";
	print "##########################################################\n";
	print "# NB dist_files = ".scalar(@$dist_files)."\n";
	print "# NB src_dirs   = ".scalar(keys(%$src_hash_dir_files))."\n";
	print "# NB src_files  = ".scalar(@$src_files)."\n";
	print "##########################################################\n";
	print "# Distfile list:                                         #\n";
	print "# $file_list                     #\n";
	print "##########################################################\n";
 	for my $file_type (keys(%$dist_hash_dir_files)){
 		my $list=$dist_hash_dir_files->{$file_type};
		print "# ";
 		print GREEN "$file_type ";
 		print scalar(@$list)."\n";
 	}
	print "##########################################################\n";
# 	&printTable($src_files);
# 	#print all no-empty dir files	
# 	for my $d (keys(%$src_hash_dir_files)){
# 		my $dir_files=$src_hash_dir_files->{$d};
# 		my $nb=scalar(@$dir_files);		
# 		if($nb){			
# 			print BLUE "$d ";
# 			print "$nb\n";	
# 		}
# 	}
}

sub check_missing(){
	my ($l1,$l2)=(@_);
	my (@missed);	
	# in l1 but not in l2
	foreach my $k (@$l1){
		if (!&tab_contain($k,$l2)){
# 			print RED "$k\n";
			push(@missed,$k);
		}
	}
# 	print MAGENTA "@1@".scalar(@missed)."\n";
	# in l2 but not in l1
	foreach my $k (@$l2){ 
		if (!&tab_contain($k,$l1)){ 
# 			print GREEN "$k\n";
			push(@missed,$k);
		}
	}
# 	print MAGENTA "@1@".scalar(@missed)."\n";
	return \@missed;
}

sub tab_contain(){
	my ($val,$tab)=(@_);
	foreach my $value (@$tab){
		#print "$val =? $value ,";
		#if ($val=~/^$value$/){
		if ($val eq $value){
			#print GREEN "OK\n";
			return 1;
		}
	}
	#print RED "KO\n";
	return 0;
}

sub printTable(){
	my ($tab)=(@_);
	for my $i (@$tab){
		print "$i\n";
	}
}
