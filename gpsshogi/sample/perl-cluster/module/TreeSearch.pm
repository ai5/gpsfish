package TreeSearch;
use strict;
use IO::Handle;
use FileHandle;
use File::Path;
use module::TreeNode;
use module::UsiPosition;
use module::UsiEngine;
use module::CsaBoard;
use module::StopWatch;
use threads::shared;

my $probe_enabled_default = 1;
my $probe_enabled = 1;
my $temporal_full_root_search = undef;
my $search_msec_infinite = 3600*1000; # 1h
my $single_search_limit_msec = 5*1000*2.5;
my $root_split_limit_msec = 13*1000*2.5;
my $root_split_width_limit = 16;
my $kachi_enabled = 1;
my $ssh_interval = 0.2;
my ($probe_msec, $probe_msec_root);
my @slave_status;
my $quit_all;
#debug
my $debug_stopping = 0;
# verbosity in search_log.txt
my $log_all = 0;
my $log_pv = 0;
my $flush_allowed = 0;

sub status_busy { 2; }
sub status_wait_queue { 1; }
sub status_ready { -1; }


BEGIN {
  $SIG{PIPE} = 'IGNORE';
  share(@slave_status);
  share($quit_all);
  share($probe_msec);
  share($probe_msec_root);
  $probe_msec = $probe_msec_root = 1000;
}

# functions
sub read_config ($@) {
  my ($filename, $verbose) = @_;
  my @lines;
  open (IN, $filename) || die "open $!";
  while (<IN>) {
    chomp;
    next if /^#/;
    next unless /[0-9a-zA-Z]/;
    push(@lines, $_);
  }
  close IN;
  print STDERR "master ".$lines[0]."\n" if ($verbose);
  print STDERR "slave ".(@lines+0-1)."\n" if ($verbose);
  return @lines;
}

sub enableKachi ($) {
  my ($value) = @_;
  $kachi_enabled = $value;
}

sub setSSHInterval ($) {
  my ($value) = @_;
  die if $value < 0;
  $ssh_interval = $value;
}

sub setSingleSearchLimitSec ($) {
  my ($value) = @_;
  die if $value < 0;
  $single_search_limit_msec = $value*1000;
  print STDERR "single_search_limit_msec = $single_search_limit_msec\n";
}

sub enableLogging () {
  $log_pv = 1;
  $flush_allowed = 1;
}

# methods
sub new ($$$$) {
  my ($pkg, $logdir, $num_slaves, @programs) = @_;
  my $this = { objects => {}, programs => [@programs],
	       in => {}, out => new Thread::Queue,
	       log_queue => new Thread::Queue,
	       pid => 0, pid_done => 0,
	       book_depth => 30, last_output => 0, probing => {},
	       # slave management
	       living_slaves => [(1..$num_slaves)], removed_slaves => [],
	       last_received => { any => 0 },
	       nodes => {}, last_job => {}, # for each search
	       search_msec => $search_msec_infinite
	     };
  (File::Path::mkpath($logdir) || die $!) unless -d $logdir;
  my ($sec,$min,$hour,$mday,$mon,$year) = localtime(time);
  my $log_filename = sprintf("search_log-%d%02d%02d-%02d%02d%02d.txt",
			     1900+$year,$mon+1,$mday,$hour,$min,$sec);
  $this->{log_filename} = "$logdir/$log_filename";
  $this->{log_thread}
    = new threads(\&write_log_run, $this->{log_filename}, $this->{log_queue});
  my $master_config = {id=>0, command=>$programs[0], logdir=>$logdir };
  $this->{masterusi} = new UsiEngine($master_config);
  $this->{masterusi}->init;
  TreeNode::setMasterUsiEngine($this->{masterusi});
  TreeNode::setLogger($this->{log_queue});
  @slave_status = map {0} (0..$num_slaves);
  foreach my $i (1..$num_slaves) {
    Time::HiRes::sleep $ssh_interval
	if ($i > 1 && $programs[$i] =~ /(ssh|usi\.pl)\s+/
	    && $programs[$i-1] =~ /(ssh|usi\.pl)\s+/);
    my $in = new Thread::Queue;
    $this->{in}->{$i} = $in;
    $this->{objects}->{$i}
      = new threads(\&slave_run, $i, $programs[$i], $logdir, $in, $this->{out});
    $this->{last_received}->{$i} = 0;
    $this->{nodes}->{$i} = 0.0;
    $this->{last_job}->{$i} = undef;
  }
  bless $this;
  $this->{position} = "position startpos";
  my $position = new UsiPosition($this->{position});
  $this->{root} = new TreeNode($position, undef);
  $this->{masterusi}->set_position($position);
  $this->{masterusi}->genmove_probability(); # warm up up
  while (1) {
    print STDERR "waiting ".count_ready()." / ".$num_slaves."\n";
    if ($num_slaves - count_ready() < 3) {
      print STDERR join("", map { "  ".$_." ".$programs[$_]."\n" } enum_uninitialized());
    } elsif ($num_slaves - count_ready() < 10) {
      print STDERR "  ".join(',', enum_uninitialized())."\n"
    }
    sleep 1;
    last if count_ready() == $num_slaves;
  }
  return $this;
}

sub set_book_depth ($$) {
  my ($this, $new_depth) = @_;
  $this->{book_depth} = $new_depth;
}
sub set_probe_msec ($$) {
  my ($this, $new_msec) = @_;
  $probe_msec = $new_msec;
}
sub set_probe_root_msec ($$) {
  my ($this, $new_msec) = @_;
  $probe_msec_root = $new_msec;
}

sub num_slaves ($) {
  my ($this) = @_;
  return @{$this->{living_slaves}}+0;
}
sub slave_list ($) {
  my ($this) = @_;
  return @{$this->{living_slaves}};
}
sub num_removed_slaves ($) {
  my ($this) = @_;
  return @{$this->{removed_slaves}}+0;
}

sub input ($$) {
  my ($this, $line) = @_;
  my $log_queue = $this->{log_queue};
  my $time = sprintf("time=%d", StopWatch::current_time);
  $log_queue->enqueue("IN $line ".$time."\n");
  $log_queue->enqueue(sprintf "counts pid %d done %d ready %d busy %d\n",
		      $this->{pid}, $this->{pid_done}, count_ready(), count_busy());
  if ($line =~ /^(position|sfen)/) {
    $log_queue->enqueue("\n\nnew position $line\n\n");
    $this->{position} = $line;
    $this->{root} = new TreeNode(new UsiPosition($line), undef);
    $log_queue->enqueue($this->{root}->csashow);
    print STDERR $this->{root}->csashow;
  }
  elsif ($line =~ /^stop/) {
    $this->stop_all;
  }
  elsif ($line =~ /^echo\s+(.*)/) {
    print STDERR "$1\n";
    $this->health_test
      if ($this->{pid} && $this->{pid} != $this->{pid_done});
  }
  elsif ($line =~ /^go\s+mate(\s?(\d+))/) {
    my $msec = $2 || $probe_msec;
    my $position = new UsiPosition($this->{position});
    $this->{masterusi}->set_position($position);
    my ($ret, $moves) = $this->{masterusi}->gomate(msec=>$msec);
    if ($ret ne "checkmate") {
      print $this->output_msg("checkmate $ret")."\n";
    } else {
      print $this->output_msg(sprintf "checkmate %s", join(' ', @$moves))."\n";
    }
  }
  elsif ($line =~ /^go\s+benchmark\s*(id=([^\s]*))?\s*(msec=(\d+))?/) {
    die if $this->{benchmark};
    $this->{benchmark} = 1;
    $this->{pid}++;
    $this->go_all;
    my $prefix = $2 || "1";
    my $current_time = StopWatch::current_time;
    print "info string $prefix expanding\n";
    my $msec = $4 || 20*1000;
    my $job = {pid=>$this->{pid}, command=>"go_benchmark",
	       msec=>$msec, ts_enqueue=>$current_time};
    foreach my $id ($this->slave_list) {
      my $rank = $prefix.".".$id;
      $this->{in}->{$id}->enqueue({%$job, prefix=>$rank});
    }
    my ($results, $info, $bestmoves) = ([], [], []);
    while ((grep {$_} @$bestmoves) < ($this->slave_list+0)) {
      my $ret = $this->{out}->dequeue;
      # printf STDERR "rcv %d %d %s\n", (@$bestmoves+0), ($this->slave_list+0), $ret->{line};
      if ($ret->{line} =~ /^info\s+string\s+(.*)/) {
	if ($ret->{line} =~ /expanding/) {
	  print $ret->{line}."\n";
	} else {
	  $info->[$ret->{id}] .= $ret->{line}."\n";
	}
      } elsif ($ret->{line} =~ /^bestmove\s+(.*)/) {
	$bestmoves->[$ret->{id}] = $1;
      } else {
	warn unless ($ret->{line} =~ /^info.*nodes\s+(\d+)/);
	$results->[$ret->{id}] = $1;
      }
    }
    my $sum = 0.0;
    foreach my $id ($this->slave_list) {
      my $rank = $prefix.".".$id;
      my $nps = $results->[$id]*1.0/$msec;
      printf "info string %5s nps = %5.1fk %s\n",
	$rank, $nps, $this->{programs}->[$id];
      print $info->[$id] if $info->[$id];
      $sum += $nps;
    }
    $this->add_job_separator;
    while (count_ready() < $this->num_slaves) {
      sleep 0.1;
    }
    printf "info string %s sum nps = %.1fk\n", $prefix, $sum;
    $this->{benchmark} = undef;
    printf "info nodes %d\nbestmove pass\n", $sum*$msec
      if ($prefix ne "1");
    $this->{pid_done} = $this->{pid};
  }
  elsif ($line =~ /^go/) {
    die if $this->{benchmark};
    my $position = new UsiPosition($this->{position});
    $this->{masterusi}->set_position($position);
    $probe_enabled = $probe_enabled_default;
    $temporal_full_root_search = undef;
    if ($line =~ /^go byoyomi (\d+)/) {
      $this->{search_msec} = $1;
      if ($this->{search_msec} < $single_search_limit_msec) {
	$log_queue->enqueue("probe disabled and search by 1 client since byoyomi $this->{search_msec} < $single_search_limit_msec\n");
	$temporal_full_root_search = 1;
      } elsif ($this->{search_msec} < $root_split_limit_msec) {
	$log_queue->enqueue("probe disabled and split at root since byoyomi $this->{search_msec} < $root_split_limit_msec\n");
	$probe_enabled = undef;
      }
    } else {
      $this->{search_msec} = $search_msec_infinite;
    }
    $this->{pid}++;
    $log_queue->enqueue(sprintf "GO pid %d with $line\n", $this->{pid});
    my $gobook = $line =~ /^go book/;
    my $godeclarewin = $line =~ /^go declare_win/;
    my $bookmove = $position->move_count < $this->{book_depth}
      ? $this->{masterusi}->gobook($this->{book_depth}) : "pass";
    my $declaremove =
      $kachi_enabled ? $this->{masterusi}->godeclarewin() : "pass";
    my $bestmove = undef;
    if ($gobook) {
      $bestmove = $bookmove;
    } elsif ($godeclarewin) {
      $bestmove = $declaremove;
    } else {
      $bestmove = $bookmove if $bookmove !~ /(resign|pass)/;
      $bestmove = $declaremove if $declaremove !~ /(resign|pass)/;
    }
    if ($bestmove) {
      $this->{root}->{full_root_search} = 1;
      $this->{out}->enqueue({pid=>$this->{pid}, id=>0, line => "bestmove $bestmove", position=>$this->{position}});
      $this->{nosearch}->{$this->{pid}} = 1;
      $this->{probing}->{$this->{pid}} = {};
    }
    else {
      $this->start_search;
    }
  }
  elsif ($line =~ /^usinewgame/) {
    $this->{nosearch} = {};
    $this->{pid} = 0;
    $this->{pid_done} = 0;
    $this->{probing} = {};
    TreeNode::setStopPID(-1);
    foreach my $i ($this->slave_list) {
      $this->{nodes}->{$i} = 0.0;
      $this->{last_received}->{$i} = 0;
      $this->{last_job}->{$i} = undef;
    }
    $this->stop_all if $this->{root}; # clear stop_flag in UsiEngine
    TreeNode::clearAlmostFinal();
    $log_queue->enqueue("\n\n\nNEW GAME\n\n");
  }
  elsif ($line =~ /^status/) {
    print STDERR "\nROOT\n".$this->{root}->child_summary(300);
    $this->show_status;
  }
  elsif ($line =~ /^ignore_moves\s+(.*)/) {
    $this->{root}->{ignore_moves} = [split(/\s+/,$1)];
  }
  elsif ($line =~ /^genmove_probability/) {
    my $position = new UsiPosition($this->{position});
    $this->{masterusi}->set_position($position);
    my $moves = $this->{masterusi}->genmove_probability();
    $this->output_msg(sprintf "genmove_probability %s\n",
      join(' ', map { "$_->[0] $_->[1]" } sort { $a->[1] <=> $b->[1] } map { [$_, $moves->{$_}] } keys %$moves));
  }
  elsif ($line =~ /^setoption.*/) {
    $log_queue->enqueue("ignored $line\n");
  }
  else {
    $log_queue->enqueue("\nerror $line\n");
    die $line;
  }
  $log_queue->enqueue("flush");
}

sub elapsed_msec ($) {
  my ($this) = @_;
  return StopWatch::current_time - $this->{start_time};
}
sub elapsed ($) {
  my ($this) = @_;
  return $this->elapsed_msec / 1000.0;
}

sub start_search ($) {
  my ($this) = @_;
  $this->{root}->genmove_probability;
  if ($this->{root}->move_count == 0) {
    $this->{nosearch}->{$this->{pid}} = 1;
    $this->{out}->enqueue({pid=>$this->{pid}, id=>0, line => "bestmove resign", position=>$this->{position}});
    return;
  }
  elsif ($this->{root}->move_count == 1) {
    $this->{nosearch}->{$this->{pid}} = 1;
    my ($move) = keys %{$this->{root}->{moves}};
    print $this->output_msg("info string forced move at the root: $move")."\n"; # better to enqueue
    $this->{out}->enqueue({pid=>$this->{pid}, id=>0, line => "bestmove $move", position=>$this->{position}});
    return;
  }
  $this->go_all;
  $this->{last_output} = $this->{start_time} = $this->{root}->{start_time}
    = StopWatch::current_time;
  if ($this->num_slaves == 1 || $temporal_full_root_search) {
    $this->{root}->{full_root_search} = 1;
    my $command = ($this->{search_msec} > $probe_msec) ? "go" : "go_probe";
    my $job = { pid=>$this->{pid}, command=>$command, position=>$this->{position},
		msec=>$this->{search_msec}, ts_enqueue=>$this->{start_time} };
    $job->{ignores} = [ @{$this->{root}->{ignore_moves}} ]
      if ($this->{root}->{ignore_moves});
    $this->{last_job}->{1} = $job;
    $this->{in}->{1}->enqueue($job);
  }
  else {
    my $root = $this->{root};
    $this->start_probe_node($root, [$this->slave_list]);
  }
}

sub select_living ($$) {
  my ($this, $slaves) = @_;
  my $result = [ grep { my $a = $_; grep { $_ == $a } $this->slave_list } @$slaves ];
  $this->{log_queue}->enqueue("select_living: ".join(',',@$slaves)
			      ." => ".join(',', @$result)."\n")
      if (@$slaves+0 > @$result+0);
  return $result;
}

sub split_node ($$) {
  my ($this, $node) = @_;
  my $log_queue = $this->{log_queue};
  $node->{probing} = $this->select_living($node->{probing});
  my $assignments = $node->assign_by_probe;
  my $current_time = StopWatch::current_time;
  my $elapsed_msec = $current_time - $this->{start_time};
  my $elapsed = $elapsed_msec/1000.0;
  my $estimated_msec = $this->{search_msec} - $elapsed_msec;
  $estimated_msec = $probe_msec if ($estimated_msec < $probe_msec);
  my $pid = $this->{pid};
  $log_queue->enqueue(sprintf "SPLIT at %s %.2fs. %s\n", $node->csahistory_from_root||"root", $elapsed,
		      join('-', map {$node->csamove($_->[0]).":".join(',',@{$_->[1]})} @$assignments));
  my @done;
  foreach my $a (@$assignments) {
    my ($move, $slaves) = @$a;
    my $slave_count = @$slaves+0;
    die unless $slave_count > 0;
    if ($move eq "other") {
      die unless $slave_count == 1;
      my $id = $slaves->[0];
      delete $this->{probing}->{$pid}->{$id};
      my $job = {pid=>$this->{pid}, command=>"go", position=>$node->{position}->to_s,
		 ignores=>[@done], msec=>$estimated_msec, ts_enqueue=>$current_time };
      push (@{$job->{ignores}}, @{$node->{ignore_moves}})
	if $node->{ignore_moves};
      $this->{last_job}->{$id} = $job;
      $this->{in}->{$id}->enqueue($job);
      last;
    }
    my $child = $node->child($move);
    die unless $child;
    if ($slave_count == 1) {
      my $id = $slaves->[0];
      delete $this->{probing}->{$pid}->{$id};
      my $job = {pid=>$this->{pid}, command=>"go", position=>$child->{position}->to_s,
		 msec=>$estimated_msec, ts_enqueue=>$current_time};
      $this->{last_job}->{$id} = $job;
      $this->{in}->{$id}->enqueue($job);
    } else {
      $this->start_probe_node($child, $slaves);
    }
    push (@done, $move);
  }
  $node->hide_children(@done);
}

sub start_probe_node ($$) {
  my ($this, $node, $slaves) = @_;
  my $log_queue = $this->{log_queue};
  $slaves = $this->select_living($slaves);
  my @moves = $node->sort_moves;
  if (! $probe_enabled || $moves[0] eq "resign") {
    shift @moves if $moves[0] eq "resign";
    $this->start_search_infinite($node, [@moves], $slaves);
  } else {
    die if (@moves+0 == 0);
    if (@moves+0 == 1) {
      my $msg = "probe one reply $moves[0] at ".$node->csahistory_from_root;
      $log_queue->enqueue($msg."\n");
      my $child = $node->create_child($moves[0]);
      return $this->start_probe_node($child, $slaves);
    }
    my $msec = $node->{parent} ? $probe_msec : $probe_msec_root;
    my $current_time = StopWatch::current_time;
    $node->{start_probe} = $current_time;
    $node->{probing} = [@$slaves];
    $node->{probing_done} = [];
    my $pid = $this->{pid};
    my $num_slaves = @$slaves + 0;
    my $try_checkmate = ($num_slaves > 3) ? 1 : 0;
    if ($try_checkmate) {
      --$num_slaves;
      $node->{checkmate} = "trying";
    } else {
      delete $node->{checkmate};
    }
    $#moves = $num_slaves-2 # 1 for other
      if($#moves > $num_slaves-1);
    my $msg = sprintf ("PROBE at %s %.2f --", $node->csahistory_from_root || "root",
		       $this->elapsed);
    my $i = 0;
    for ($i=0; $i<=$#moves; ++$i) {
      my $move = $moves[$i];
      my $id = $slaves->[$i];
      my $new_position = $node->create_child($move)->{position};
      $msg .= sprintf " %s:%d", $node->csamove($move), $id;
      my $job = {pid=>$this->{pid}, command=>"go_probe", position=>$new_position->to_s,
		 msec=>$msec, ts_enqueue=>$current_time};
      $this->{last_job}->{$id} = $job;
      $this->{in}->{$id}->enqueue($job);
      $this->{probing}->{$pid}->{$id} = $node;
    }
    if ($node->move_count > $num_slaves) {
      my $id = $slaves->[$i++];
      $msg .= " other:$id";
      my $job = {pid=>$this->{pid}, command=>"go_probe", position=>$node->{position}->to_s,
		 msec=>$msec, ignores=>[@moves], ts_enqueue=>$current_time};
      push (@{$job->{ignores}}, @{$node->{ignore_moves}})
	if $node->{ignore_moves};
      $this->{last_job}->{$id} = $job;
      $this->{in}->{$id}->enqueue($job);
      $this->{probing}->{$pid}->{$id} = $node;
    }
    die unless ($i == $num_slaves) || ($i == $#moves+1);
    if ($try_checkmate) {
      my $id = $slaves->[$i++];
      $msg .= " mate:$id";
      my $job = {pid=>$this->{pid}, command=>"go_mate", position=>$node->{position}->to_s,
		 msec=>$msec, ts_enqueue=>$current_time};
      $this->{last_job}->{$id} = $job;
      $this->{in}->{$id}->enqueue($job);
      $this->{probing}->{$pid}->{$id} = $node;
    }
    die unless ($i == @{$node->{probing}}+0) || ($i == $#moves+1+$try_checkmate);
    $log_queue->enqueue($msg."\n");
    # print STDERR $msg."\n";
  }
}

sub start_search_infinite ($$$$) {
  my ($this, $node, $moves_ref, $slaves) = @_;
  $slaves = $this->select_living($slaves);
  splice (@$slaves, $root_split_width_limit)
    if ((@$slaves + 0) > $root_split_width_limit);
  my $log_queue = $this->{log_queue};
  my $num_slaves = @$slaves + 0;
  my @moves = @$moves_ref;
  $#moves = $num_slaves-2 # 1 for other
    if($#moves > $num_slaves-1);
  $log_queue->enqueue("SPLIT infinite ".join(',',@moves)."--".join(',',@$slaves)."\n");
  my $current_time = StopWatch::current_time;
  my $rank = 1;
  $node->{rank} = {};
  foreach my $i (0..$#moves) {
    my $move = $moves[$i];
    my $id = $slaves->[$i];
    my $child = $node->create_child($move);
    $node->{rank}->{$move} = $rank++;
    my $new_position = $child->{position};
    $log_queue->enqueue("split $id ".$child->csahistory_from_root."\n");
    delete $this->{probing}->{$this->{pid}}->{$id};
    my $job = {pid=>$this->{pid}, command=>"go", position=>$new_position->to_s,
	       ts_enqueue=>$current_time, msec=>$this->{search_msec}};
    $this->{last_job}->{$id} = $job;
    $this->{in}->{$id}->enqueue($job);
  }
  if ($node->move_count > $num_slaves) {
    my $id = $slaves->[$num_slaves-1];
    $log_queue->enqueue("split $id ".$node->csahistory_from_root." (other)\n");
    delete $this->{probing}->{$this->{pid}}->{$id};
    my $job = {pid=>$this->{pid}, command=>"go", position=>$node->{position}->to_s, ignores=>[@moves],
	       ts_enqueue=>$current_time, msec=>$this->{search_msec}};
    push(@{$job->{ignores}}, @{$this->{root}->{ignore_moves}})
      if (! $node->{parent} && $this->{root}->{ignore_moves});
    $this->{last_job}->{$id} = $job;
    $this->{in}->{$id}->enqueue($job);
  }
}

sub stop_all ($) {
  my ($this) = @_;
  print STDERR "stop_all pid = $this->{pid}\n";
  $this->{stopping_pid} = $this->{pid};
  TreeNode::setStopPID($this->{pid});
  foreach my $i ($this->slave_list) {
    $this->stop_search_of_id($i, $this->{pid});
  }
}

sub stopping ($) {
  my ($this) = @_;
  return ($this->{stopping_pid}||-1) == $this->{pid};
}

sub node_sum ($) {
  my ($this) = @_;
  my $sum = 0.0;
  foreach my $i ($this->slave_list) {
    $sum += $this->{nodes}->{$i};
  }
  return $sum;
}

sub has_output ($) {
  my ($this) = @_;
  return $this->{out}->peek;
}

sub output_msg ($$) {
  my ($this, $msg) = @_;
  my $log_queue = $this->{log_queue};
  $this->{last_output} = StopWatch::current_time;
  $log_queue->enqueue("OUT $msg time=".int($this->{last_output}/1000)."\n");
  return $msg;
}

sub output_root_bestmove ($$) {
  my ($this, $bestmove_msg) = @_;
  $this->{pid_done} = $this->{pid};
  warn "$bestmove_msg" unless $bestmove_msg =~ /^bestmove\s+(.*)/;
  my $bestmove = $1;
  unless ($this->{nosearch}->{$this->{pid_done}}) {
    # ensure info score is shown for the selected move
    print STDERR "bestmove: $bestmove, (last bestmove: $this->{root}->{last_bestmove})\n"
      if ($this->{root}->{last_bestmove} && $this->{root}->{last_bestmove} ne $bestmove);
    print $this->output_msg($this->{root}->usi_info_pv($bestmove))."\n" # better to enqueue
      if ($bestmove !~ /(resign|win)/ && !$this->{root}->{full_root_search});
    # make all slaves sleep (stop all if needed)
    $this->add_job_separator;
    my $finish_count = count_ready();
    if ($finish_count < $this->num_slaves) {
      print STDERR "\nwait for all slaves $finish_count < ".$this->num_slaves."\n";
      $this->stop_all
	unless defined $this->{stopping_pid}
	  && $this->{stopping_pid} == $this->{pid};
      while (($finish_count = count_ready()) < $this->num_slaves) {
	sleep 0.1;
      }
    }
  }
  my $log_queue = $this->{log_queue};
  $log_queue->enqueue("OUT $bestmove_msg time=".time."\n");
  $log_queue->enqueue("flush");
  return $bestmove_msg;
}

sub idle_test ($) {
  my ($this) = @_;
  return unless $this->{pid} > $this->{pid_done};
  my $now = StopWatch::current_time;
  $this->{log_queue}->enqueue("ping $now\n");
  $this->{out}->enqueue({ping=>1, ts_ping=>$now});
  $this->health_test
    if ($this->{pid} && $this->{pid} != $this->{pid_done});
}

sub test_win_or_split_node ($$) {
  my ($this, $probe_root) = @_;
  my $msg = sprintf "probe finish %s => %s (%s)\n",
    $probe_root->csahistory_from_root || "root",
      $probe_root->csamove($probe_root->is_final),
	$probe_root->{checkmate} || "-";
  $this->{log_queue}->enqueue($msg);
  if ($probe_root->has_checkmate_win) {
    my $msg = $probe_root->update_checkmate_win;
    printf $this->output_msg("info score cp %d nodes %d pv %s")."\n", # better to enque but...
      abs($this->{root}->win_value), # relative to player
	$this->node_sum, $1
	  if ($msg && $msg =~ /^bestmove\s+(.*)/);
    return $msg;
  }
  $this->split_node($probe_root);
  return undef;
}

sub process_search_output ($$$) {
  my ($this, $now, $ret) = @_;
  my $log_queue = $this->{log_queue};
  if ($ret->{pid} != $this->{pid} || $ret->{pid} <= $this->{pid_done}) {
    warn "delay: pid $this->{pid}/$this->{pid_done} returned $ret->{pid}, id = $ret->{id}, $ret->{line}";
    return;
  }
  die unless $ret->{line};
  return if ($ret->{line} =~ /^info\s+depth\s+[0-9]+\s*$/);
  my $position = new UsiPosition($ret->{position});
  my $node = $this->{root}->find($position)
    || die "tree broken ".$ret->{line}." id ".$ret->{id}.' at '.$position->to_s;

  my $received = sprintf "%06.2f slave %3d: %s",
    ($now-($this->{root}->{start_time}||0))/1000.0, $ret->{id}, $ret->{line};
  my $probe_root = $this->{probing}->{$ret->{pid}}->{$ret->{id}};

  my $timestamp_test = ($log_pv || $log_all)
    && !$this->stopping && ! $this->{nosearch}->{$this->{pid}}
    && $this->{root}->{start_time} && $ret->{ts_received};
  my $need_warn = "";
  if ($timestamp_test) {
    $received .= sprintf " rcv %.2f",
      ($ret->{ts_received}-$this->{root}->{start_time})/1000.0;
    $received .= sprintf " timestamp go (%.2f, %.2f, %.2f)",
      ($ret->{ts_enqueue}-$this->{root}->{start_time})/1000.0,
	($ret->{ts_dequeue}-$this->{root}->{start_time})/1000.0,
	  ($ret->{ts_go}-$this->{root}->{start_time})/1000.0
	    if ($ret->{line} =~ /^bestmove/ && $ret->{ts_go} && $ret->{ts_enqueue});
    if ($ret->{ts_go} && $ret->{ts_enqueue}
	&& $ret->{ts_go}-$ret->{ts_enqueue} > 200) {
      $need_warn = "delayed go after enqueue";
    } elsif ($now - $ret->{ts_received} > 300) {
      $need_warn = "delayed after received";
    } elsif (($this->{last_received}->{any}||0) - $ret->{ts_received} > 200) {
      $need_warn = "delayed message by queue congestion";
    } elsif ($probe_root &&
	     ($now - $ret->{ts_enqueue}
	      > ($probe_root->{parent} ? $probe_msec : $probe_msec_root) + 1000)) {
      $need_warn = "delayed probe result";
    }
  }
  warn $received.' '.$need_warn if $need_warn;
  $log_queue->enqueue($received.' '.$need_warn."\n")
    if ($need_warn || $log_all || ($log_pv && $ret->{line} =~ /pv/)
	|| $ret->{line} =~ /(string|checkmate)/);
  $this->{last_received}->{any} = $ret->{ts_received}
    if ($ret->{ts_received});
  $this->{last_received}->{$ret->{id}} = $now;
  $this->{nodes}->{$ret->{id}} = $1
    if ($ret->{line} =~ /^info.*nodes\s+([0-9]+)/);
  my $msg = $node->update_leaf($ret, $this->{in});
  if ($probe_root && ! $probe_root->{parent} && $this->stopping
      && $probe_root->probe_finished) {
    unless ($msg && $msg =~ /^bestmove/) { # "go mate" responded the last
      my $bestmove = $probe_root->{last_bestmove};
      unless ($bestmove) {
	warn "use resign for stopped probe";
	$bestmove = "resign";
      }
      $msg = "bestmove $bestmove";
    }
    return $msg;
  }
  if ($probe_root && ! $this->stopping) { # ignore "bestmove" in $msg if probing
    if ($probe_root->probe_finished) {
      return $this->test_win_or_split_node($probe_root);
    }
    return;
  }
  if ($this->stopping && !$msg && $debug_stopping) {
    $log_queue->enqueue("\nROOT\n".$this->{root}->child_summary(300));
    while ($node->{parent}) {
      $node = $node->{parent};
      $log_queue->enqueue("\nnode ".$node->csahistory_from_root."\n".$node->child_summary(300));
    }
  }
  return $msg;
}

sub health_test ($) {
  my ($this) = @_;
  # print STDERR "health test\n";
  my $now = StopWatch::current_time;
  my $log_queue = $this->{log_queue};
  foreach my $id ($this->slave_list) {
    my $job = $this->{last_job}->{$id};
    next if $slave_status[$id] != status_busy;
    my ($received, $ping, $error) = UsiEngine::receive_status_of_id($id);
    my $last_activity = ($received > $ping) ? $received : $ping;
    unless ($job->{ts_enqueue}) {
      warn;
      next;
    }
    $last_activity = $job->{ts_enqueue} if $job->{ts_enqueue} > $last_activity;
    my $idle = $now - $last_activity;
    if ($idle > 1500) {
      my $node = $this->{root}->find(new UsiPosition($job->{position}))
	|| die "tree broken ".$id.' at '.$job->{position}."\n";
      my $history = $node->csahistory_from_root || "root";
      my $msg = sprintf "health %d %s %d idle %d position %s %s\n",
	$id, $job->{command}, $slave_status[$id], $idle,
	  $node->csahistory_from_root || "root", $this->{programs}->[$id];
      print STDERR $msg;
      $log_queue->enqueue($msg);
    }
    if ($idle > 3500 + ($this->{search_msec}||0)/16 || $error) {
      my $node = $this->{root}->find(new UsiPosition($job->{position}))
	|| die "tree broken ".$id.' at '.$job->{position}."\n";
      my $history = $node->csahistory_from_root || "root";
      my $msg = sprintf "SLAVE %2d was REMOVED by %s (%s) %s\a\n",
	$id, ($error || ($now-$last_activity)/1000.0."s"),
	  $this->{programs}->[$id], $job->{command};
      $msg .= sprintf "  working on %s", $history;
      warn $msg;
      $log_queue->enqueue($msg."\n");
      $this->{living_slaves} = [ grep { $_ != $id } $this->slave_list ];
      push(@{$this->{removed_slaves}}, $id);
      if ($job) {
	my $probe_root = $this->{probing}->{$this->{pid}}->{$id};
	push(@{$probe_root->{probing_done}}, $id) if ($probe_root);
	if ($job->{ignores}) {
	  $node->{other}->{final} = 1;
	} else {
	  warn unless $node->{parent};
	  if ($node->{parent}) {
	    $node->{final} = $node->{leaf}->{final} = $node->{leaf}->{pv}
	      ? $node->{leaf}->{pv}->[0] : "resign";
	    warn unless $node->{leaf};
	    if ($node->{leaf}) {
	      my $msg = sprintf "REMOVED last update: value %s nodes %s update %s pv %s\n",
		$node->{leaf}->{value} || "-", $node->{leaf}->{nodes} || "-",
		  $node->{leaf}->{update} || "-",
		    $node->{leaf}->{pv} ? join(":",@{$node->{leaf}->{pv}}) : "";
	      print STDERR $msg;
	      $log_queue->enqueue($msg);
	    }
	  }
	}
	my $win_by_checkmate = $this->test_win_or_split_node($probe_root)
	  if $probe_root && $probe_root->probe_finished;
	return $win_by_checkmate if $win_by_checkmate;
      }
      $this->show_status;
      print STDERR "\nROOT\n".$this->{root}->child_summary(300);
      $log_queue->enqueue("flush");
    }
  }				# foreach slave
}

sub process_output ($@) {
  my ($this,$blocking_wait) = @_;
  my $log_queue = $this->{log_queue};
  while ($this->has_output || $blocking_wait) {
    my $ret = $this->{out}->dequeue;
    my $now = StopWatch::current_time;
    my $msg = undef;
    $msg = $this->process_search_output($now, $ret) unless $ret->{ping};
    if ($msg) {
      return $this->output_root_bestmove($msg)
	if ($this->{root}->is_final);
      if (TreeNode::isAlmostFinal($this->{pid})) {
	print STDERR "stop all\n";
	$this->stop_all;
	TreeNode::clearAlmostFinal();
      }
      return $this->output_msg($msg);
    }
    # misc info
    next unless ($this->{last_output}+1000 < $now);
    if ($this->stopping) {
      my $busy = count_busy();
      print STDERR "wait stop #".($busy)."\n";
      if ($busy < 8) {
	foreach my $id ($this->slave_list) {
	  next unless $slave_status[$id] == status_busy;
	  print STDERR "  $id ".$this->{programs}->[$id]."\n";
	}
      }
      $this->idle_test;
      $this->{last_output} = $now;
    }
    # look around health of slaves
    my $may_win = $this->health_test;
    return $may_win if $may_win;
    # info nodes
    unless ($this->{root}->{full_root_search}) {
      my @node_count = grep { $_->[1] >= 1.0 } map { [$_, $this->{nodes}->{$_}/1000.0] } sort {$this->{nodes}->{$b} <=> $this->{nodes}->{$a}} keys %{$this->{nodes}};
      if ($this->num_slaves >= 3 && ! $this->stopping && (@node_count+0 >= 3)) {
	my $msg = sprintf "explored max %.1fk (%d) med %.1fk (%d) min %.1fk (%d) busy %d / %d (%d) nps %.1fk",
	  $node_count[0]->[1], $node_count[0]->[0],
	    $node_count[@node_count/2]->[1], $node_count[@node_count/2]->[0],
	      $node_count[$#node_count]->[1], $node_count[$#node_count]->[0],
		count_busy(), $this->num_slaves, $this->num_removed_slaves,
		  1.0 * $this->node_sum/($now - $this->{root}->{start_time});
	$log_queue->enqueue($msg."\n");
	if (($this->{last_statistics_output}||0)+1000*10 < $now) {
	  print STDERR $msg."\n";
	  $this->{last_output}=$this->{last_statistics_output}=$now;
	}
      }
      if (($this->{last_pv_output}||0)+1000*5 < $now
	  && $this->{root}->{last_bestmove}) {
	$this->{last_output} = $this->{last_pv_output} = $now;
	return $this->{root}->usi_info_pv;
      } else {
	$this->{last_output} = $now;
	return "info nodes ".$this->node_sum." time ".($now - $this->{root}->{start_time})
	  ." nps ".int($this->node_sum/(($now - $this->{root}->{start_time})/1000));
      }
    }
  }
  return undef;
}

sub stop_search_of_id ($$$) {
  my ($this, $id, $pid) = @_;
  UsiEngine::stop_by_pid($id, $pid);
}

sub slave_run ($$$$) {
  my ($id, $program, $logdir, $in, $out) = @_;
  die $id unless $program;
  local $SIG{PIPE} = 'IGNORE';
  my $config = {id=>$id, command=>$program, logdir=>$logdir };
  my $slaveusi = new UsiEngine($config);
  $slaveusi->init;
  $slaveusi->set_depth(10);
  $slave_status[$id] = status_ready;
  while (! $quit_all) {
    {
      lock(@slave_status);
      while ((!$slave_status[$id]||$slave_status[$id]!=status_wait_queue) && !$quit_all) {
	cond_wait(@slave_status);
      }
    }
    last if ($quit_all);
    while (my $data = $in->dequeue) {
      # print STDERR "status $id => 2\n";
      $slave_status[$id] = status_busy;
      ### command
      $data->{out} = $out;
      $data->{id} = $id;
      $data->{ts_dequeue} = StopWatch::current_time;
      if ($data->{command} eq "go") {
	$slaveusi->go_infinite($data);
      }
      elsif ($data->{command} eq "go_probe") {
	$data->{msec} = $probe_msec unless $data->{msec};
	$slaveusi->go_probe($data);
      }
      elsif ($data->{command} eq "go_mate") {
	$data->{msec} = $probe_msec unless $data->{msec};
	$slaveusi->go_mate($data);
      }
      elsif ($data->{command} eq "go_benchmark") {
	$data->{position}
	  = "position sfen ln5nk/3+Bls1sl/5p3/2ppp3p/pN1P5/5PP1P/PP2G1NP1/4P1SK1/+rr3G2L b B2G2Ps2p 1"
	    unless $data->{position};
	if ($program =~ /usi\.pl/) {
	  $slaveusi->send("go benchmark id=$data->{prefix} msec=$data->{msec}\n");
	  while (1) {
	    my $line = $slaveusi->read();
	    chomp $line;
	    $out->enqueue({id=>$id, line=>$line});
	    last if ($line =~ /^bestmove/);
	  }
	} else {
	  $slaveusi->go_probe($data);
	}
      }
      else {
	# todo probe
	die "$data->{command}";
      }
      # print STDERR "status $id => 1\n";
      $slave_status[$id] = status_wait_queue;
    }
    # print STDERR "status $id => -1\n";
    $slave_status[$id] = status_ready;
  }
  $slaveusi->finish;
  sleep 1;
}

sub write_log_run ($$) {
  my ($log_filename, $log_queue) = @_;
  my $log = new FileHandle "> $log_filename";
  $log->autoflush(1) if $log_all;
  while (my $data = $log_queue->dequeue) {
    if ($data =~ /^flush$/) {
      $log->flush if $flush_allowed;
      next;
    }
    print $log $data;
  }
}

sub count_ready () {
  return (grep { defined $_ && $_ == status_ready } @slave_status)+0;
}
sub count_busy () {
  return (grep { defined $_ && $_ == status_busy } @slave_status)+0;
}
sub enum_slaves_of_status ($) {
  my ($status) = @_;
  my @ret = ();
  foreach my $i (1..$#slave_status) {
    push(@ret, $i) if ($slave_status[$i] == $status);
  }
  return @ret;
}
sub enum_uninitialized () {
  return enum_slaves_of_status(0);
}
sub go_all ($) {
  my ($this) = @_;
  lock(@slave_status);
  foreach my $id ($this->slave_list) {
    $slave_status[$id] = 1;
  }
  cond_broadcast(@slave_status);
}

sub add_job_separator ($) {
  my ($this) = @_;
  foreach my $id ($this->slave_list) {
    $this->{in}->{$id}->enqueue(undef);
  }
}

sub finish ($) {
  my ($this) = @_;
  $quit_all = 1;
  {
    lock(@slave_status);
    cond_broadcast(@slave_status);
  }
  $this->{masterusi}->finish;

  foreach my $id (keys %{$this->{objects}}) {
    if (grep { $_ == $id } $this->slave_list) {
      $this->{objects}->{$id}->join;
    } else {
      $this->{objects}->{$id}->detach;
    }
  }
  $this->{log_queue}->enqueue(undef);
  $this->{log_thread}->join;
  sleep 1;
}

sub show_status ($) {
  my ($this) = @_;
  print STDERR "working slaves: total ".$this->num_slaves."\n";
  my $now = StopWatch::current_time;
  foreach my $id ($this->slave_list) {
    my ($received, $ping, $error) = UsiEngine::receive_status_of_id($id);
    my $last_activity = ($received > $ping) ? $received : $ping;
    my $msg = "  id = $id  status = $slave_status[$id]";
    if ($slave_status[$id] == status_busy) {
      my $job = $this->{last_job}->{$id};
      warn unless $job;
      if ($job) {
	$last_activity = $job->{ts_enqueue} if $job->{ts_enqueue} > $last_activity;
	my $node = $this->{root}->find(new UsiPosition($job->{position}))
	  || warn "tree broken ".$id.' at '.$job->{position}."\n";
	$msg .= " ".$job->{command};
	$msg .= " at ".($node->csahistory_from_root || "root")
	  if $node;
	$msg .= " enq ".$job->{ts_enqueue};
      }
    }
    $msg .= " now ".$now." active ".$last_activity;
    $msg .= " err ".$error if $error;
    print STDERR $msg."\n";
    $this->{log_queue}->enqueue($msg."\n");
  }
}

# END
1;
