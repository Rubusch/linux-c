#!/usr/bin/perl -w
use strict;
##
## remove leading characters - if present (using match)
## 

use File::Copy;


## check filename
##
sub checkfilename
{
    my $filename = shift;

    if(!-e $filename){ die "File \"$filename\" doesn't exist!\nFailed!\n"; }
    if(!-r $filename){ die "File \"$filename\" isn't readable!\nFailed!\n"; }
    if(!-w $filename){ die "File \"$filename\" isn't writeable!\nFailed!\n"; }
}



###
### main 
###

## checks
if($#ARGV != 0){ die "Usage:\n  $0 <filename>\nFailed!\n"; }
my $filename = $ARGV[0];
&checkfilename($filename);

## backup
print "backup $filename -> ${filename}.orig\n";
copy(${filename}, ${filename}. ".orig") or die "Failed to backup!\n";

## 3, 2, 1, action
print "\nReplacing leading leading crap...\n";
open(READ, "<${filename}.orig");
open(WRITE, ">$filename");
my $tmp;
while($_ = <READ>){
    chomp;
    s/^\+//g;
    print WRITE "$_\n";
}
close(READ);
close(WRITE);

## done
print "check $filename, in case do a:\n  mv ${filename}.orig $filename\n";
print "READY.\n";
exit 0;

