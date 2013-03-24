#!/usr/bin/perl -w
use strict;
##
## adjusts the ramdisk size
##


use File::Copy;


###
### sub functions
###


## check ramdisk size
##
sub checkramdisksize
{
    my $ramdisksize = $_[0];
    my $minsize = $_[1];
    my $maxsize = $_[2];

    if($ramdisksize !~ /\d+/){ die "Parameter is not a number!\nFailed!\n"; }
    if($ramdisksize < $minsize){ die "Ramdisksize too small, try something like:\n$minsize < x < $maxsize!\nFailed!\n"; }
    if($ramdisksize > $maxsize){ die "Ramdisksize too large, try something like:\n$minsize < x < $maxsize!\nFailed!\n"; }
}


## check filenames
##
sub checkfilename
{
    my $filename = $_[0];
    if(!-e $filename){ die "Failed!\nThe specified file: \"$filename\" doesn't exist!\n"; }
    if(!-r $filename){ die "Failed!\nThe file: \"$filename\" isn't readable!\n"; }
    if(!-w $filename){ die "The file: \"$filename\" isn't writeable!\nIn case this file needs to be checked out!!!\n"; }
}


## last advice
##
sub advices
{
    my @filenames = @_;

    print "Finally check if the script worked for the content of the following files:\n\t";
    $" = "\n\t";
    print "@filenames\n";
    print "The files should be backuped and are named with a \".orig\" extension.\n\n";
}


###
### main function - programm start
###


## check ARGV
## 
if($#ARGV != 0){ die "Usage:\n$0 <ramdisksize>\nFailed!\n"; }


## check ramdisksize
##
my $minsize = 21000;
my $maxsize = 32768;
my $ramdisksize = $ARGV[0];
&checkramdisksize($ramdisksize, $minsize, $maxsize);
print "ramdisksize shall be set to be: $ramdisksize kB\n";


## ckeck filenames
##
## target implementation:
my %filelist = (
	       "/hix/gpon_cxu_f/cs/runconfig.sh", "export RAMDISK_SIZE"
	     , "/hix/gpon_cxu_f/os/linux-2.4/.config", "CONFIG_BLK_DEV_RAM_SIZE"
	     , "/hix/gpon_cxu_f/os/linux-2.4/linux.config", "CONFIG_BLK_DEV_RAM_SIZE"
	     , "/hix/gpon_cxu_f/make/Makefile", "SIZE"
	     );

my $filename;
foreach $filename (keys %filelist){
    print "check file $filename...";
    &checkfilename($filename);
    print "Ok!\n";
    
    ## backup
    print "Doing a backup of ${filename}, name it ${filename}.orig\n";
    copy(${filename}, ${filename} . ".orig") or die "Failed to do backups!\n";    
}
print "Filecheck succeeded!\n\n";


## match pattern and replace
##
foreach $filename (keys %filelist){    
    my $pattern = $filelist{$filename};    
    my $new_pattern = $pattern . "=" . $ramdisksize;
    print "Replacing \"$pattern\" by \"$new_pattern\" \nin $filename...";

    my $source = $filename . ".orig";
    my $dest = $filename;
    open(READ, "<$source"); 
    open(WRITE, ">$dest");
    while($_ = <READ>){
	s/${pattern} *= *\d+$/${new_pattern}/g;
	print WRITE $_;
    }
    close(READ);
    close(WRITE);
    print "Ok!\n\n";
}


## finals
##
&advices(keys %filelist);


print "READY.\n";
exit 0;
