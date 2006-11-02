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

#
# DIET version
#
#FIXME: to be set with CMake ? We should have a VERSION file in root directory...
open( CMakeLists, '<', "CMakeLists.txt");
while (<CMakeLists>) 
{
    $lines = $_;
    chomp $lines;
    if ($lines =~ /SET\(DIET_version[A-z]*/ )
    {
	( $null, $diet_ver) = split / /, $lines;
	( $diet_ver, $null) = split /\)/, $diet_ver;
	printf ("DIET Release $diet_ver\n");
	close(CMakeLists);
    };
};
close(CMakeLists);

#
# Distribution file list
#
$file_list = "bin/scripts/Distribution_files.lst";

#
# Rel. path to the script which processes templated files
#
$distrib_file_sh = "bin/scripts/distrib_file.sh";

#
# Test for "dev" argument.
#
if( $#ARGV == 0 ){
  if( $ARGV[0] =~ /^dev$/ ){
    $maintainer_mode = 1;
  }
} elsif( $#ARGV == -1 ){
  $maintainer_mode = 0;
} else {
  die( "Usage: make_dist.pl [dev]" );
}

#
# Prepare distribution directory
#
$distdir = "diet-" . $diet_ver;
if( $maintainer_mode ){
  $distdir .= "-dev";
}
if( -d $distdir ){
  print( "WARNING: $distdir exists, overwrite ? [y/N] " );
  $ans = <STDIN>;
  if( $ans =~ /[yY]/ ){
    rmtree $distdir;
  } else {
    print( "OK, I stop. Please rename $distdir and relaunch $0\n" );
    exit 0;
  }
}
mkdir $distdir;

#
# Read distribution file list
#

open( FILE, '<', $file_list )
or die( "Cannot open $file_list" );

# File type:
#  -1: unset
#   0: templated file (to be processed by distrib_file.sh)
#   1: untemplated file (to be put as is in distribution)
#   2: templated maintainer file
#   3: untemplated maintainer file
$file_type = -1;
$no_line = 0;
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

    $file = $_;
    die( "Cannot access $file ($file_list:$no_line)" ) unless -e $file;
    die( "No section defined before line $no_line" ) if( $file_type == -1 );

    #
    # OK, we got a file to process
    #
    if( $file_type >= 2 && $maintainer_mode == 0 ) {
      last SWITCH;
    }
    
    # Get path and base names
    ( $file_dir, $file_base ) = $file =~ /(.*)\/(.*)/;
    $dstfile = $distdir."/".$file;
    $dstfile_dir =  $distdir."/".$file_dir;

    # Create the path of the file in distribution tree
    if( ! -d $dstfile_dir ){ mkpath $dstfile_dir; }

    # If the source file does not exists, abort.
    #FIXME: Try to generate it with CMake (useful for docs, I think)
    die( "Cannot access nor generate $file" ) if( ! -e $file );

    # Copy or process file...
    if( $file_type == 1 || $file_type == 3 ){ # untemplated file
      cp $file, $dstfile;
    } else {                                  # templated file
      my @options;
      my @args;
      push @args, $file;
      push @args, $dstfile;
      $status = system( "$distrib_file_sh", @options, @args );
      die( "Formatting $file failed. Abort." ) if( $status != 0 );
    }
    
    # "last SWITCH" == DUMMY statement...
    $dummy = 0;
  }

}

#
# Build archive
#
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
