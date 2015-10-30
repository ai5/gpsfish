#!/usr/bin/perl -w
use strict;
use threads;
use threads::shared;
use Thread::Queue;
use Getopt::Std;
use FileHandle;
use module::UsiEngine;
use module::TreeSearch;
use module::CsaBoard;
use IO::Select;

$| = 1;

my $options = {};
getopts('a:c:f:l:N:S:s:v:n:DB:P:Q:R:L:KW:w:Z', $options);
my $num_slaves = $options->{N} || -1;
my $verbose = $options->{v} || 0;
my $config_file = $options->{c} || "config";
my $benchmark_file = $options->{f};
my $benchmark_seconds = $options->{S};
my $name = $options->{n} || "gpsshogi_expt";
my $author = $options->{a} || "teamgps";
my $book_depth = (defined $options->{B}) ? $options->{B} : 35;
my $root_width_option = $options->{R} || 2;
my $leaf_width_option = $options->{L} || 3;
my $probe_msec_option = $options->{P} || 1000;
my $sleep_before_exit = $options->{W} || 1;
TreeSearch::enableKachi(0) if $options->{K};
TreeSearch::setSSHInterval($options->{s} || 2);
TreeSearch::setSingleSearchLimitSec($options->{Q}) if $options->{Q};
TreeNode::setSplitNearWidth($options->{w})
    if defined $options->{w};
TreeSearch::enableLogging() if defined $options->{Z};
my $logdir = $options->{l} || "log";
{
    my ($sec,$min,$hour,$mday,$mon,$year) = localtime(time);
    $logdir = sprintf("$logdir/%d%02d%02d-%02d%02d%02d",
		      1900+$year,$mon+1,$mday,$hour,$min,$sec)
}
sub byoyomi_margin () { return 500; }
sub try_read ($);
sub go_benchmark ($);
sub important_message ($) { my ($m) = @_; return $m =~ /(stop|bestmove|go)/; }
sub log_msg ($);

die "configuration not found: \"$config_file\"" unless -r $config_file;
my @master_and_slave = TreeSearch::read_config($config_file,1);
$num_slaves = @master_and_slave-1 if ($num_slaves < 0);
if ($benchmark_file && ! -r $benchmark_file) {
  warn "$benchmark_file not found";
  $benchmark_file = undef;
}
TreeNode::setShowMovesInCSA(1) if ($options->{D});
die unless $leaf_width_option >= 1;

### main
my $search;
my $line = ($benchmark_file || <>);
print "id name $name\n";
print "id author $author\n";
print "usiok\n";
while ($benchmark_file || ($line = <>)) {
  chomp $line;
  if ($benchmark_file || $line =~ /^isready/) {
    $search = new TreeSearch($logdir, $num_slaves, @master_and_slave);
    $search->set_book_depth($book_depth);
    if ($probe_msec_option =~ /(\d+):(\d+)/) {
      die unless ($1 >= $2 && $2 >= 1000);
      $search->set_probe_root_msec($1);
      $search->set_probe_msec($2);
    } else {
      die unless $probe_msec_option >= 1000;
      $search->set_probe_msec($probe_msec_option);
    }
    TreeNode::setLeafWidth($leaf_width_option);
    TreeNode::setRootWidth($root_width_option);
    print STDERR "readyok\n";
    print "readyok\n";
    last;
  }
  print STDERR "waiting isready $line\n";
}
$search->input("position startpos");
log_msg("we are ready.");

my ($stdin_selector, $stdin_buf) = (new IO::Select(*STDIN), "");
my ($go_count, $bestmove_count, $wait_bestmove) = (0,0,undef,"");

INPUT: while (1) {
  my $record_move = undef;
  $record_move = go_benchmark($benchmark_file) if $benchmark_file;
  my $start_time = StopWatch::current_time;
  my $scheduled_stop = 0;
  $scheduled_stop = $start_time + $benchmark_seconds*1000
    if ($benchmark_file && $benchmark_seconds);
 PROBLEM: while (1) {
    if (! $wait_bestmove) {
      if (my $line = try_read(0.1)) {
	last INPUT if ($line =~ /quit/);
	$scheduled_stop = 0
	  if ($line =~/stop/);
#	if ($line =~ /echo\s+(.*)/) {
#	    print $1."\n";
#	    next;
#	}
	log_msg ("> ".$line) if (important_message($line));
	$search->input($line);
	if ($line =~ /^go/) {
	  if ($line =~ /^go\s+byoyomi\s+(\d+)/) {
	    $scheduled_stop = StopWatch::current_time + $1 + byoyomi_margin;
	  }
	  $go_count++ unless $line =~ /^go\s+(mate|benchmark)/;
	}
	$wait_bestmove = ($bestmove_count < $go_count) if ($line =~ /^stop/);
      }
    }
    while ($search->has_output || $wait_bestmove) {
      my $ret = $search->process_output($wait_bestmove);
      last unless $ret;
      print $ret."\n";
      log_msg ("< ".$ret) if (important_message($ret));
      if ($ret =~ /^bestmove\s+(.*)/) {
	$scheduled_stop = 0;
	$bestmove_count++;
	$wait_bestmove = undef;
	if ($record_move) {
	  printf STDERR "%s: %s search %s record %s\n", $benchmark_file,
	    ($record_move eq $1 ? "OK" : "NG"), $1, $record_move;
	  sleep 1;
	  if (@ARGV) {
	    $benchmark_file = shift @ARGV;
	    last PROBLEM;
	  }
	  last INPUT;
	}
      }
      next;
    }
    my $now = StopWatch::current_time;
    $search->idle_test
      if ($now > $search->{last_output}+1200);
    if ($scheduled_stop > 0 && $now > $scheduled_stop
	&& !$wait_bestmove) {
      $wait_bestmove = 1;
      log_msg("scheduled stop by usi.pl");
      $search->input("stop");
      $scheduled_stop = 0;
    }
  }
}
print STDERR "finish\n";
$search->finish;
sleep $sleep_before_exit;

sub try_read ($) {
  my ($timeout) = @_;

  while (1) {
    my $len = index($stdin_buf,"\n");
    if ($len >= 0) {
      my $line = substr($stdin_buf, 0, $len+1);
      $stdin_buf = substr($stdin_buf, $len+1);
      $line =~ s/\r?\n$//;
      $line =~ s/^\s+//;
      print STDERR "U<<< $line time=".time."\n";
      return $line;
    }
    return undef if (defined $timeout && ! $stdin_selector->can_read($timeout));
    my $buf; sysread(*STDIN, $buf, 65536);
    die "stdin closed" unless (defined $buf);
    $stdin_buf .= $buf;
    $timeout = 0 if defined $timeout;
  }
  die;
}

sub go_benchmark ($) {
  my ($filename) = @_;
  my $position = $search->{masterusi}->file_to_usiposition($filename);
  my $record_move = undef;
  if ($filename =~ /.csa$/) {
    $record_move = $position->{moves}->[0] if $position->move_count;
    $position = $position->initial_position;
  }
  $search->input("usinewgame");
  $search->input($position->to_s);
  $search->input("go infinite");
  return $record_move;
}

sub log_msg ($) {
  my ($msg) = @_;
  my ($sec,$min,$hour,$mday,$mon,$year) = localtime(time);
  printf STDERR "' %d%02d%02d-%02d%02d%02d (%d): %s\n",
    1900+$year,$mon+1,$mday,$hour,$min,$sec,
      StopWatch::current_time,
	  $msg;
}
