#!/usr/bin/perl
#
# This script changes "extern inline" to "static inline" in header
# files.  I did this so that I could use -finstrument-functions to
# trace Linux kernel code.  The script is pretty stupid if it finds
# extern and inline togther its likely to make a change.  It removes
# the inline from forward references and changes extern to static
# for definitions.

open(FIND, "find . -name \*.[ch] |") || die "couldn't run find on *.[ch]\n";
while ($f = <FIND>) {
       chop $f;
       if (!open(FILE, $f)) {
               print STDERR "Can't open $f\n";
               next;
       }
#      print STDERR "scanning $f\n";
       undef $file_content;
       $file_content = "";
       $modified = 0;
OUT:
       while ($line = <FILE>) {
               # check for comment, ignore lines that start with
               # a comment.  Ignore block comments
               if ($line =~ /^\s*\/\*.*\*\//) {
                       $file_content .= $line;
                       next;
               }
               if ($line =~ /^\s*\/\*/) {
                       $file_content .= $line;
                       while ($line = <FILE>) {
                               $file_content .= $line;
                               if ($line =~ /\*\//) {
                                       next OUT;
                               }
                       }
                       print STDERR "??? $f: end of file in comment?";

               }
               if ($line  =~ /extern\s+(.*)(inline|__inline|__inline__)\s/) {
                       $extra = 0;
                       if ($line =~ /^#define/) {
                               # Alpha & ARM have defines
                               # for extern inline which I'm
                               #ignoring for now.
                               $file_content .= $line;
                               next;
                       }
                       while (!($line =~ /;|{/)) {
                               if (!($nl = <FILE>)) {
                                       die "hit EOF... file=$f\n";
                               }
                               if (++$extra > 8) {
                                       print STDERR "??? $f: $line";
                                       last;
                               }
                               $line .= $nl;
                       }
                       if ($line =~ /{/) {
                               $line =~ s/extern/static/;
                               $modified = 1;
                       } elsif ($line =~ /;/) {
                               $line =~ s/[    ]*__inline__[   ]*/ /;
                               $line =~ s/[    ]*__inline[     ]*/ /;
                               $line =~ s/[    ]*inline[       ]*/ /;
                               $modified = 1;
                       }
               }
               $file_content .= $line;
       }
       close(FILE);
       $name = $f . ".orig";
       if ($modified && -e $name) {
               print STDERR "$name already exists - no changes made\n";
               next;
       }
       if ($modified) {
               if (link($f, $name)) {
                       unlink($f);
               } else {
                       print STDERR "Can't move $f to $name\n";
                       next;
               }
               if (!open(FILE, ">$f")) {
                       prinf STDERR "Can't open $f for output\n";
                       next;
               }
               print FILE $file_content;
               close(FILE);
       }
}
