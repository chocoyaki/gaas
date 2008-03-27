#!/usr/bin/perl
#****************************************************************************#
#* FIXME: Description of the file is to be updated...                       *#
#* Clean up CVS fields in DIET files, when building a distribution.         *#
#* This script takes at most one argument: "dev", set when building a       *#
#*  maintainer distribution. It reads Distribution_files.lst.               *#
#*                                                                          *#
#*  Author(s):                                                              *#
#*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       *#
#*                                                                          *#
#* $LICENSE$                                                                *#
#****************************************************************************#
#* $Id$
#* $Log$
#* Revision 1.5  2008/03/27 18:47:03  rbolze
#* update scripts and list of distrib. files
#* exec "./bin/scripts/make_dist.pl -help" for usage
#*
#* Revision 1.4  2006/11/27 11:41:35  pcombes
#* Do not test file presence if not needed in distribution
#*
#* Revision 1.3  2006/11/02 01:57:32  ecaron
#* Get version number for release from CMakeLists.txt file (perl version)
#*
#* Revision 1.2  2006/11/02 00:24:43  ecaron
#* Get version number for release from CMakeLists.txt file
#*
#* Revision 1.1  2006/11/01 23:45:39  ecaron
#* Script to generate the DIET distribution
#* Author: Philippe Combes
#*
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
	my ($help,$version,$maintainer_mode);
        &GetOptions("maintainer_mode"=>\$maintainer_mode,
		    "version=s"=>\$version,
                "help|h"=>\$help);
	if ($help){
		&usage();
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
	    if( ! -d $dstfile_dir ){ mkpath $dstfile_dir; }
	
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
