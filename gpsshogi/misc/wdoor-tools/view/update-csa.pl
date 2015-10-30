#!/usr/bin/perl -w
use strict;
use FileHandle;

my $prepare_rating = "/home/shogi-server/bin/mk_game_results";

my %finished;
my %playing;

my $ire = '[A-Za-z0-9_@.-]+';

my $finished_fh;
$finished_fh = new FileHandle "> finished.txt" || die "open $!";
$finished_fh->autoflush(1);

my $floodgate_rating_fh;
$floodgate_rating_fh = new FileHandle "|$prepare_rating > floodgate-results.txt" || die "open $!";
$floodgate_rating_fh->autoflush(1);

my $others_rating_fh;
$others_rating_fh = new FileHandle "|$prepare_rating > others-results.txt" || die "open $!";
$others_rating_fh->autoflush(1);

my $finished_store = "../tmp";
mkdir $finished_store."/floodgate";
mkdir $finished_store."/others";

sub store_finished ($$) {
}

sub get_record_info ($$)
{
    my ($dir, $record) = @_;
    $record->{moves} = 0;
    $record->{result} = 'unknown';
    $record->{time} = [0,0];
    $record->{summary} = $record->{reason} = $record->{win} = $record->{lose} = "N/A";
    my $turn = 0;
    open (CSA, $dir . "/" . $record->{file})
	|| die "$!";
    while (<CSA>) {
	chomp;
	if (/^[+-][0-9][0-9][0-9][0-9]/) {
	    ++$record->{moves};
	    next;
	}
	if (/^T([0-9]+)/) {
	    $record->{time}[$turn] += $1;
	    $turn = ($turn+1)%2;
	    next;
	}
	if (/^\'\$END_TIME:(.*)/) {
	    $record->{endtime} = $1;
	    next;
	}
	if (/^\'summary:([^:]+):([^:]+):([^:]+)/) {
	    $record->{summary} = $_;
	    $record->{reason} = $1;
	    $record->{win} = $3;
	    $record->{lose} = $2;
	    if ($record->{lose} =~ /win$/) {
		my $tmp = $record->{win};
		$record->{win} = $record->{lose};
		$record->{lose} = $tmp;
	    }
	    next
		if ($record->{reason} =~ /abnormal/
		    #|| $record->{reason} =~ /time up/
		   );
	    if ($record->{win} =~ /win$/) {
		if ($record->{win} =~ /^$record->{sente} win$/) {
		    $record->{result} = 'sente';
		} elsif ($record->{win} =~ /^$record->{gote} win$/) {
		    $record->{result} = 'gote';
		}
	    } elsif ($record->{win} =~ /draw/) {
		$record->{result} = 'draw';
	    }
	    next;
	}
    }
    return $record;
}

sub add_record_finished ($$$) {
    my ($fh, $dir, $record) = @_;
    printf $fh "%s %s %s %d %d %d %s %s\n",
	$record->{file}, $record->{sente}, $record->{gote},
	    $record->{moves}, $record->{time}[0], $record->{time}[1],
		$record->{result},
		    $record->{reason};
    my $dst = $finished_store;
    if ($record->{game} =~ /floodgate-/) {
	$dst .= "/floodgate";
    } else {
	$dst .= "/others";
    }
    system "cp", "$dir/".$record->{file}, $dst;
    if ($record->{game} =~ /floodgate-/) {
      print $floodgate_rating_fh $dst."/".$record->{file}."\n";
    } else {
      print $others_rating_fh $dst."/".$record->{file}."\n";
    }
}
sub add_record ($$$) {
    my ($fh, $dir, $record) = @_;
    printf $fh "%s %s %s %d %d %d\n",
	$record->{file}, $record->{sente}, $record->{gote},
	    $record->{moves}, $record->{time}[0], $record->{time}[1];
}

sub update ($$$) {
    my @list = ();

    %playing = ();
    my ($dir, $verbose, $refresh_playing) = @_;
    opendir(DIR, $dir) || warn "opendir $dir !";
    @list = readdir(DIR);
    closedir(DIR)
      || warn "closedir $dir: $!";
    foreach my $file (sort @list) {
	next
	    if $file =~ /^\./;
	if ( -d "$dir/$file" ) {
	    print STDERR "$dir/$file\n";
            update("$dir/$file", $verbose, $refresh_playing);
        }
	next
	    if -z $file;
	next
	    unless $file =~ /^($ire)\+($ire\-[0-9]+-[0-9]+)[\+:]($ire)[\+:]($ire)[\+:]([0-9]{4})([0-9]{2})([0-9]{2})([0-9]{2})([0-9]{2})([0-9]{2})\.csa$/;
	my $record = { event => $1, game => $2, sente => $3, gote => $4,
		       year => $5, month => $6, date => $7, hour => $8,
		       minute => $9, second => $10, file => "$file"
		     };
	next
	    if ($finished{$file});
	get_record_info($dir, $record);
	if ($record->{endtime}) {
	    print STDERR "$file\n"
		if ($verbose);
	    $finished{$file} = 1;
	    add_record_finished($finished_fh, $dir, $record);
	} else {
	    $playing{$file} = $record;
	}
    }
    my $playing_fh;
    if ($refresh_playing) {
      $playing_fh = (new FileHandle "> playing.txt") || die "open $!";
    } else {
      $playing_fh = (new FileHandle ">> playing.txt") || die "open $!";
    }
    foreach my $file (keys %playing) {
	my $record = $playing{$file};
	add_record($playing_fh, $dir, $record);
    }
    close $playing_fh;
}

foreach my $d (@ARGV) {
    print STDERR "process $d\n";
    update($d, 0, 1);
}

sub make_directory ($) {
    my ($now) = @_;
    my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst)
	= localtime($now);
    return sprintf("%04d/%02d/%02d", $year+1900, $mon+1, $mday);
}
my $now = time;
my $dir = make_directory($now);
my $dir_yesterday = make_directory($now-86400);

while (1) {
    update($dir_yesterday, 0, 1);
    update($dir, 0, 0);

    $now = time;
    my $new_dir = make_directory($now);
    print STDERR $new_dir."\n"
	if ($new_dir ne $dir);
    $dir = $new_dir;
    $dir_yesterday = make_directory($now-86400);
    sleep 5;
}
