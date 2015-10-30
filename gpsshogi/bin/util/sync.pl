#!/usr/bin/perl -w
use strict;
use Getopt::Std;
use File::Path;
use Time::Piece;

# sync.pl -l localdir -h host -r remotedir
# -- localdir/in -> remotedir@host/in
# -- remotedir@host/out -> remotedir@host/out

my %options;
getopts("l:h:r:N",\%options);

my ($localdir, $host, $remotedir, $lpath, $rpath, $newgame)
  = ($options{l} || "local-csa",
     $options{h} || "komaba",
     $options{r} || "/home/wcsc23/remote-csa",
     $options{L} || "..",
     $options{R} || "/home/wcsc23",
     $options{N},
    );

my $now = Time::Piece::localtime;
if ($newgame) {
    rename $localdir, $localdir."-".$now->datetime();
    system "ssh", $host, "mv $remotedir $remotedir-".$now->datetime();
}
mkpath("$localdir/in", 1, 0755);
mkpath("$localdir/out", 1, 0755);

my $initialize = "mkdir -p $remotedir/in; mkdir -p $remotedir/out; "
    ."chmod 755 $remotedir/in; chmod 755 $remotedir/out";
system "ssh", $host, $initialize;

{
  my $pid=fork;
  if (!$pid){
    my $cmd = "perl $lpath/bin/util/filetostdout.pl -k -d $localdir/in | ssh $host perl -I $rpath/bin $rpath/bin/util/stdintofile.pl -d $remotedir/in";
    print STDERR "$cmd\n";
    system($cmd);
    exit(0);
  }
}

{
  my $pid=fork;
  if (!$pid){
    my $cmd = "ssh $host perl -I $rpath/bin $rpath/bin/util/filetostdout.pl -k -d $remotedir/out | perl $lpath/bin/util/stdintofile.pl -d $localdir/out";
    print STDERR "$cmd\n";
    system($cmd);
    exit(0);
  }
}
sleep(3);
print STDERR "hopefully ready\n";

wait;
wait;
