package UsiEngine;
use strict;
use IPC::Open2;
use IO::Handle;
use IO::Select;
use module::UsiPosition;
use module::StopWatch;
use threads::shared;

my $default_msec = 5000;
my $default_depth = 10;
my $flush_allowed_usi = 0;
my @stop_flag;
my @go_count;
my @last_ping;
my @last_received;
my @errors;

BEGIN {
  $| = 1;
  share(@stop_flag);
  share(@go_count);
  share(@last_ping);
  share(@last_received);
  share(@errors);
}
sub important_message ($) { my ($m) = @_; return $flush_allowed_usi && $m =~ /(stop|bestmove|go|quit)/; }

sub new ($$) {
  my ($pkg, $config) = @_;
  my $this =
    {
     config => $config,
     id => $config->{id},
     logdir => $config->{logdir} || ".",
     options => {}, buf => ""
    };
  $stop_flag[$this->{id}] = -1;
  $last_received[$this->{id}] = $last_ping[$this->{id}] = StopWatch::current_time;
  (mkdir($this->{logdir}) || die $!) unless -d $this->{logdir};
  $this->{io} = $this->{logdir}."/"."io-".$this->{id}.".txt";
  $this->{stderr} = $this->{logdir}."/"."stderr-".$this->{id}.".txt";
  die "command not specified" unless $config->{command};
  my $command = $config->{command}." 2>> ".$this->{stderr};
  $this->{command} = $command;
  print STDERR "SYSTEM:execute $this->{id} '$command'\n";
  $this->{pid} = open2($this->{reader}, $this->{writer}, $command);
  open($this->{logger}, ">> ".$this->{io}) || die "open $!";
  $this->{writer}->autoflush(1);
  bless $this;

  $this->send("usi\n");
  while (my $line = $this->read()) {
    if ($line =~ /^id\s+name\s+(.*)/) {
      $this->{name} = $1;
    } elsif ($line =~ /^id\s+author\s+(.*)/) {
      $this->{author} = $1;
    } elsif ($line =~ /^option\s+name\s+([^\s]*)\s+(.*)/) {
      $this->{options}->{$1} = $2;
    }
    last if $line =~ /^usiok/;
    die "usiok failed" unless $line;
  }
  return $this;
}

sub send ($$) {
  my ($this, $message) = @_;
  my ($writer, $logger) = ($this->{writer}, $this->{logger});
  print $logger "> $message";
  $logger->flush if important_message($message);
  my $ok = print $writer "$message";
  unless ($ok) {
    $errors[$this->{id}] = "write";
    warn "write $! id = $this->{id}";
    threads->exit(1);
  }
}

sub read ($@) {
  my ($this, $timeout) = @_;
  my ($reader, $logger) = ($this->{reader}, $this->{logger});
  my $selector = (defined $timeout) ? new IO::Select($reader) : undef;
  if (defined $timeout && $timeout < 0) {
    warn $timeout;
    $timeout = 0;
  }
  while (1) {
    my $len = index($this->{buf},"\n");
    if ($len >= 0) {
      my $line = substr($this->{buf}, 0, $len+1);
      $this->{buf} = substr($this->{buf}, $len+1);
      print $logger "< $line";
      $logger->flush if important_message($line);
      return $line;
    }
    return undef if ($selector && ! $selector->can_read($timeout));
    my $buf; $reader->sysread($buf, 65536);
    unless (defined $buf) {
      $errors[$this->{id}] = "read";
      warn "read $! id = $this->{id}";
      threads->exit(1);
    }
    $this->{buf} .= $buf;
    $timeout = 0;
  }
}

sub finish ($) {
  my ($this) = @_;
  $this->send("stop\n");
  $this->send("quit\n");
  waitpid $this->{pid}, 0;
}

sub init ($) {
  my ($this) = @_;
  $this->send("setoption name Verbose value 1\n")
    if (defined $this->{options}->{Verbose});
  $this->send("setoption name ErrorLogFile value\n")
    if (defined $this->{options}->{ErrorLogFile});
  $this->send("setoption name InputLogFile value\n")
    if (defined $this->{options}->{InputLogFile});
  $this->send("setoption name BookDepth value 0\n")
    if (defined $this->{options}->{BookDepth});
  $this->send("setoption name LimitDepth value $default_depth\n")
    if (defined $this->{options}->{LimitDepth});
  $this->send("setoption name UsiOutputPawnValue value 1000\n")
    if (defined $this->{options}->{UsiOutputPawnValue});
  $this->send("isready\n");
  while (my $line = $this->read()) {
    chomp $line;
    if ($line =~ /^readyok/) {
      $this->{initialized} = 1;
      return 1;
    }
  }
  die "isready failed";
  return undef;
}

sub set_depth ($$) {
  my ($this, $depth) = @_;
  $depth = int($depth);
  unless ($depth > 0) {
    warn "depth $depth";
    $depth = $default_depth;
  }
  $this->send("setoption name LimitDepth value $depth\n")
    if (defined $this->{options}->{LimitDepth});
}

sub set_position ($$) {
  my ($this,$position) = @_;
  die "gpsusi not initialized" unless $this->{initialized};
  $this->{position} = new UsiPosition($position->to_s());
  $this->send($position->to_s()."\n");
}

sub set_ignore_moves ($$) {
  my ($this,$ignores) = @_;
  die "gpsusi not initialized" unless $this->{initialized};
  $this->send("ignore_moves ".join(" ",@$ignores)."\n");
}

sub csashow ($$) {
  my ($this, $position) = @_;
  $this->set_position($position)
    if ($position);
  die "position not specified" unless $this->{position};
  my $ret = "";
  $this->send("csashow\n");
  while (my $line = $this->read()) {
    last if ($line =~ /^csashowok/);
    $ret .= $line;
  }
  return $ret;
}

sub csamove ($$) {
  my ($this,$move) = @_;
  return "%TORYO" if ($move =~ /resign/);
  return "%KACHI" if ($move =~ /win/);
  return "%TORYO" if ($move =~ /pass/);
  die "move parse error $move" unless $move =~ /^[A-Za-z0-9*+]{4,5}$/;
  die "position not specified" unless $this->{position};
  $this->send("csamove $move\n");
  while (my $line = $this->read()) {
    return $1 if ($line =~ /^csamove ([+-][0-9]{4}[A-Z]{2})$/);
    die "unexpected csamove $line\n";
  }
}

sub csamoves ($$) {
  my ($this,$moves) = @_;
  die "position not specified" unless $this->{position};
  my $original = $this->{position}->clone;
  my @ret;
  my $invalid = 0;
  foreach my $move (@$moves) {
    $invalid = 1 if ($move =~ /(resign|pass)/);
    if ($invalid) {
      push(@ret, $move);
      next;
    }
    push(@ret, $this->csamove($move));
    $this->{position}->make_move($move);
    $this->set_position($this->{position});
  }
  $this->set_position($original);
  return @ret;
}

sub read_csafile ($$) {
  my ($this, $filename) = @_;
  my $ret = "";
  $this->send("open $filename\n");
  while (my $line = $this->read()) {
    last if ($line =~ /^openok/);
    chomp $line;
    $ret .= $line;
  }
  return $ret;
}

sub file_to_usiposition ($$) {
  my ($this, $filename) = @_;
  if ($filename =~ /\.csa$/) {
    return new UsiPosition($this->read_csafile($filename));
  }
  # assume usi
  open (IN, $filename) || die "open $!";
  my $line = <IN>;
  die "usi parse failure" unless $line =~ /^(position\s?)*(startpos)?/;
  close IN;
  return new UsiPosition($line);
}

sub gobook ($@) {
  my ($this, $move_limit) = @_;
  die "position not specified" unless $this->{position};
  my $command = "go book";
  $move_limit = 35 if (! $move_limit && $this->{name} =~ /gpsshogi/);
  $command .= sprintf " %d", $move_limit if ($move_limit);
  $this->send($command."\n");
  while (my $line = $this->read()) {
    chomp $line;
    return $1
      if ($line =~ /^bestmove\s+([A-Za-z0-9+*]+)\s?$/);
    die "unexpected $line in gobook";
  }
  return undef;
}

sub godeclarewin ($@) {
  my ($this) = @_;
  die "position not specified" unless $this->{position};
  my $command = "go declare_win";
  $this->send($command."\n");
  while (my $line = $this->read()) {
    chomp $line;
    return $1
      if ($line =~ /^bestmove\s+([A-Za-z0-9+*]+)\s?$/);
    die "unexpected $line in godeclarewin";
  }
  return undef;
}

sub gomate ($%) {
  my ($this, %search_config) = @_;
  die "gpsusi not initialized" unless $this->{initialized};
  die "position not specified" unless $this->{position};
  my $msec = $search_config{msec} || $default_msec;
  $this->send(sprintf("go mate %d\n", int($msec)));
  while (my $line = $this->read()) {
    chomp $line;
    next if ($line =~ /^(info|ping)/);
    return ($1, []) if ($line =~ /^checkmate\s+(nomate|timeout)\s?$/);
    return ("checkmate", [split(/\s+/,$1)])
      if ($line =~ /^checkmate\s+((\s?[A-Za-z0-9+*])+)\s?$/);
    die "unexpected $line in gomate";
  }
  return undef;
}

sub fullsearch ($%) {
  my ($this, %search_config) = @_;
  my $msec = $search_config{msec} || $default_msec;
  die "position not specified" unless $this->{position};
  die "msec error" unless $msec > 0;
  die "gpsusi not initialized" unless $this->{initialized};
  my $go_command = undef;
  $go_command = sprintf("go byoyomi %d\n", int($msec));
  my $ret = $this->internal_wait_search($go_command);
  return $ret;
}

sub stop_by_pid ($$) {
  my ($id, $pid) = @_;
  $stop_flag[$id] = $pid;
}
sub stop ($) {
  my ($this) = @_;
  stop_by_pid($this->{id}, $go_count[$this->{id}])
}
sub stop_called ($) {
  my ($this) = @_;
  return defined $stop_flag[$this->{id}]
    && $stop_flag[$this->{id}] >= $go_count[$this->{id}];
}
sub receive_status_of_id ($) {
  my ($id) = @_;
  return ($last_received[$id], $last_ping[$id], $errors[$id]);
}

sub internal_wait_search ($$) {
  my ($this, $go_command) = @_;
  # $go_count[$this->{id}]++;
  $this->send($go_command);
  $last_received[$this->{id}] = StopWatch::current_time;
  my ($table, $last_depth, $last_node_count) = ({}, 0, 0);
  my $timeout = 0.1;
  my $stop_by_outside = undef;
  while ((my $line = $this->read($timeout)) || $timeout) {
    my $now = StopWatch::current_time;
    if ($timeout) {
      if ($this->stop_called && $timeout < 1.0) {
	$stop_by_outside = 1;
	$timeout = 1.0;
	$this->send("stop\n");
      }
      unless ($line) {
	if ($last_received[$this->{id}]+1000 < $now
	    && $last_ping[$this->{id}]+1000 < $now
	    && $last_received[$this->{id}] > $last_ping[$this->{id}]) {
	  $this->send("echo ping time=".$now."\n");
	  $last_ping[$this->{id}] = $now;
	}
	next;
      }
    }
    chomp $line;
    $last_received[$this->{id}] = $now;
    next if ($line =~ /^ping/);
    $last_depth = $1 if ($line =~ /^info.*\s+depth\s?([0-9]+)/);
    if ($line =~ /^info string forced move at the root:\s+([A-Za-z0-9+*]+)/) {
      $table->{$1} = { depth=>$last_depth, forced=>1 };
    }
    elsif ($line =~ /^info string loss by checkmate/) {
      $table->{resign} = { depth=>$last_depth };
    }
    elsif ($line =~ /^info.*\s+score\s+cp\s+([0-9-+]+).*\s+pv\s+((\s?[A-Za-z0-9+*]+)*)\s*$/) {
      my $value = $1;
      $value *= $this->{position}->turn_sign();
      my @pv = split(/\s+/, $2);
      my $depth = @pv+0;
      $depth = $1 if $line =~ /\s+depth\s+([0-9]+)/;
      die "pv not found" unless $pv[0];
      $table->{$pv[0]} = { depth=>$depth, value=>$value, pv=>[@pv] };
    }
    $last_node_count = $1
      if ($line =~ /^info.*nodes\s+([0-9]+)/ && $1 > $last_node_count);
    next if ($line =~ /^info/);
    return { bestmove => $1, last_depth=>$last_depth,
	     timeout => $stop_by_outside,
	     nodecount => $last_node_count,
	     $table->{$1} ? %{$table->{$1}} : (), }
      if ($line =~ /^bestmove\s+([A-Za-z0-9+*]+)\s?$/);
    die "unexpected $line in go";
  }
  return undef;
}

sub fullsearch_onereply_extension ($$%) {
  my ($this, $position, %search_config) = @_;
  $this->set_position($position);
  $this->set_ignore_moves($search_config{ignores}) if ($search_config{ignores});
  my $ret = $this->fullsearch(%search_config);
  my $forced_sequence = [];
  my $started = StopWatch::current_time;
  while (defined $ret->{forced}) {
    push(@$forced_sequence, $ret->{bestmove});
    $this->{position}->make_move($ret->{bestmove});
    $this->set_position($this->{position});
    $ret = $this->fullsearch(%search_config);
  }
  if (@$forced_sequence+0) {
    my $leaf = $ret;
    if (!$leaf->{pv}) {
	$leaf->{pv} = [];
	warn "empty pv id = $this->{id}" unless $this->stop_called;
    }
    $ret =
      {
       leaf => $leaf, forced_sequence => $forced_sequence,
       value => $leaf->{value},
       depth => $leaf->{depth} + @$forced_sequence,
       last_depth => $leaf->{last_depth} + @$forced_sequence,
       bestmove => $forced_sequence->[0],
       pv => [@$forced_sequence, @{$leaf->{pv}}],
       nodecount => $leaf->{nodecount},
      };
  }
  $ret->{elapsed} = StopWatch::current_time - $started;
  $ret->{position} = $position;
  $this->set_position($position);
  return $ret;
}

sub go_infinite ($$) {
  my ($this, $config) = @_;
  warn unless defined $config->{pid};
  $go_count[$this->{id}] = $config->{pid}; # note: extended interface (other go methods just increments $go_count)
  die unless $config->{position};
  $this->set_position(new UsiPosition($config->{position}));
  $this->set_ignore_moves($config->{ignores}) if ($config->{ignores});
  if ($config->{msec}) {
    $this->send(sprintf("go byoyomi %d\n", int($config->{msec})));
  } else {
    $this->send("go infinite\n");
  }
  $last_received[$this->{id}] = $config->{ts_go} = StopWatch::current_time;
  warn sprintf("go delayed %d %s id = %d", $config->{ts_go}-$config->{ts_enqueue},
	       $config->{ts_dequeue} ? ($config->{ts_dequeue}-$config->{ts_enqueue}) : "",
	       $this->{id})
    if ($config->{ts_enqueue} && $config->{ts_go}-$config->{ts_enqueue} > 200
	&& (! $config->{resign_confirm} || $config->{ts_go}-$config->{ts_enqueue} > 2000));
  my $timeout = 0.1;
  while ((my $line = $this->read($timeout)) || $timeout) {
    my $now = StopWatch::current_time;
    if ($timeout) {
      if ($this->stop_called && $timeout < 1.0) {
	$timeout = 1.0;
	$this->send("stop\n");
      }
      unless ($line) {
	if ($last_received[$this->{id}]+1000 < $now
	    && $last_ping[$this->{id}]+1000 < $now
	    && $last_received[$this->{id}] > $last_ping[$this->{id}]) {
	  $this->send("echo ping time=".$now."\n");
	  $last_ping[$this->{id}] = $now;
	}
	next;
      }
    }
    chomp $line;
    $last_received[$this->{id}] = $now;
    next if ($line =~ /^ping/);
    next if ($line =~ /^info\s+currmove\s+[A-Za-z0-9+*]+\s*$/)
	|| ($line =~ /^info\s+hashfull\s+[0-9]+\s*$/);
    next if ($line =~ /^info\s+depth\s+[0-9]+\s*$/);
    if ($config->{ignores}) {
      next if ($line =~ /^info.*pv\s+([^ ]+)/ && grep { $_ eq $1 } @{$config->{ignores}});
      $line = "bestmove resign" if ($line =~ /^bestmove\s+([^ ]+)/
				    && grep { $_ eq $1 } @{$config->{ignores}} );
    }
    $config->{out}->enqueue({%$config, line=>$line, ts_received=>$now});
    last if ($line =~ /^bestmove/);
  }
  return undef;
}

sub go_probe ($$) {
  my ($this, $config) = @_;
  warn unless defined $config->{pid};
  $go_count[$this->{id}] = $config->{pid}; # note: extended interface (other go methods just increments $go_count)
  die unless $config->{position};
  $config->{ts_go} = StopWatch::current_time;
  warn sprintf("go delayed %d %s id = %d", $config->{ts_go}-$config->{ts_enqueue},
	       $config->{ts_dequeue} ? ($config->{ts_dequeue}-$config->{ts_enqueue}) : "",
	       $this->{id})
    if ($config->{ts_enqueue} && $config->{ts_go}-$config->{ts_enqueue} > 200
	&& (! $config->{resign_confirm} || $config->{ts_go}-$config->{ts_enqueue} > 2000));
  my $position = new UsiPosition($config->{position});
  my $ret = $this->fullsearch_onereply_extension($position, %$config);
  $config->{ts_received} = StopWatch::current_time;
  if ($ret->{pv}) {
    my $bestmove = $ret->{pv}->[0];
    my $ignore = $config->{ignores} && grep { $_ eq $bestmove } @{$config->{ignores}};
    if ($ignore) {
      warn "ignored $bestmove, id = $this->{id}";
    } else {
      my $info = sprintf "info score cp %d nodes %d pv %s",
	$ret->{value}*$position->turn_sign(), $ret->{nodecount}, join(' ',@{$ret->{pv}});
      $config->{out}->enqueue({%$config, line=>$info});
    }
  }
  if ($ret->{bestmove} && $ret->{bestmove} eq "resign" && ! $ret->{nodecount}
      && ! $config->{resign_confirm}) {
    if ($this->stop_called) {
      warn "resign will not be confirmed in stop, id = $this->{id}";
      $ret->{nodecount} = 1;
    } else {
      warn "confirmation before resign, id = $this->{id}";
      $config->{resign_confirm} = 1;
      $this->go_probe($config);
      return;
    }
  }
  if ($ret->{bestmove} && $config->{ignores}
      && grep { $_ eq $ret->{bestmove} } @{$config->{ignores}}) {
    warn "ignored ".$ret->{bestmove}.", id = $this->{id}";
    $ret->{bestmove} = "resign";
  }
  unless ($ret->{bestmove}) {
    warn "bestmove not defined in go_probe, id = $this->{id}";
    $ret->{bestmove} = "resign";
  }
  my $bestmove = sprintf "bestmove %s", $ret->{bestmove};
  $config->{out}->enqueue({%$config, line=>$bestmove, ts_received=>StopWatch::current_time});
}

sub go_mate ($$) {
  my ($this, $config) = @_;
  warn unless defined $config->{pid};
  $go_count[$this->{id}] = $config->{pid}; # note: extended interface (other go methods just increments $go_count)
  die unless $config->{position};
  $config->{ts_go} = StopWatch::current_time;
  warn sprintf("go delayed %d %s id = %d", $config->{ts_go}-$config->{ts_enqueue},
	       $config->{ts_dequeue} ? ($config->{ts_dequeue}-$config->{ts_enqueue}) : "",
	       $this->{id})
    if ($config->{ts_enqueue} && $config->{ts_go}-$config->{ts_enqueue} > 200
	&& (! $config->{resign_confirm} || $config->{ts_go}-$config->{ts_enqueue} > 2000));
  my $position = new UsiPosition($config->{position});
  $this->set_position($position);
  my ($ret, $pv) = $this->gomate(%$config);
  $config->{ts_received} = StopWatch::current_time;
  if (!defined $ret
      || ($ret eq "checkmate" && (!$pv||@{$pv}==0||!$pv->[0]))) {
    $ret = "timeout";
    warn $ret;
  }
  my $line = sprintf("checkmate %s",
		     $ret eq "checkmate" ? $pv->[0] : $ret);
  $config->{out}->enqueue({%$config, line=>$line, ts_received=>StopWatch::current_time});
}

sub genmove ($) {
  my ($this) = @_;
  die "position not specified" unless $this->{position};
  $this->send("genmove\n");
  while (my $line = $this->read()) {
    chomp $line;
    die "unexpected $line in genmove" unless $line =~ /^genmove/;
    my (undef, @ret) = split(/\s+/, $line);
    return @ret;
  }
  return undef;
}

sub genmove_probability ($) {
  my ($this) = @_;
  die "gpsusi not initialized" unless $this->{initialized};
  die "position not specified" unless $this->{position};
  $this->send("genmove_probability\n");
  while (my $line = $this->read()) {
    chomp $line;
    die "unexpected $line in genmove_probability"
      unless $line =~ /^genmove_probability/;
    my (undef, @ret) = split(/\s+/, $line);
    my $table;
    foreach my $i (0..$#ret) {
      $table->{$ret[$i]} = $ret[$i+1] if ($i % 2 == 0);
    }
    return $table;
  }
  return undef;
}

# END
1;
