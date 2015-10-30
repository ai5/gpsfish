package CsaClient;
use IO::Handle;
use Socket;
use strict;
use POSIX;

### TODO:
### CSA or X1
### logging or not

sub new ($$$@) {
    my ($pkg, $name, $password, $config) = @_;

    my $this = { name => $name,
		 password => $password,
		 verbose => 0,
		 in_game => 0,
		 lockfile => $ENV{HOME}."/gpsshogi-in-use",
		 start_hook => $ENV{HOME}."/bin/start_gpsshogi",
		 finish_hook => $ENV{HOME}."/bin/finish_gpsshogi",
		 config => ($config || {})
	       };
    delete $this->{start_hook} unless (-x $this->{start_hook});
    delete $this->{finish_hook} unless (-x $this->{finish_hook});
    bless $this;
    return $this;
}

sub send ($$)
{
    my ($this, $message) = @_;

    my $socket = $this->{socket};
    die "not connected"
	unless $socket;
    print $socket "$message";
    if ($message ne "\n") {
	print STDERR "SEND:$message";
    } else {
	print STDERR '.';
    }
}

my $read_error = 0;
sub try_read_in_sec ($$)
{
    my ($this,$sec) = @_;
    my $socket = $this->{socket};
    my ($rin, $rout);

    $rin = '';
    vec($rin,fileno($socket),1) = 1;
    while (1) {
	my $nfound = select($rout=$rin, undef, undef, $sec);
	return undef
	    unless ($nfound);

	my $line = $this->read_force();
	if (! defined $line) {
	    ++$read_error;
	    die "connection closed?"
		if ($read_error > 10);
	}
	else {
	    $read_error = 0;
	}
	unless ($line eq "\n") {
	  $this->{last_opmove_time} = time;
	  return $line;
	}
	# print STDERR "read again\n";
    }
}

sub try_read ($)
{
    my ($this) = @_;
    return $this->try_read_in_sec(0.001);
}

sub read ($)
{
    my ($this) = @_;
    my $line;
    while (1) {
        my $timeout = $this->{x1} ? 90.0 : 531.0;
	return $line
	    if (defined ($line = $this->try_read_in_sec($timeout))
		&& $line !~ /^%%%/ && $line !~ /^#+\[LOBBY/);
	$this->send("\n");
	if ($this->{host} =~ /81dojo/ && $this->{in_game}) {
	  if (time > $this->{totaltime}+$this->{byoyomi}
	      + $this->{last_opmove_time}) {
	    # 81dojo will not handle timeup for disconnected opponent
	    $this->send("%%%DECLARE\n");
	  }
	}
    }
}

sub read_force ($)
{
    my ($this) = @_;
    my $socket = $this->{socket};
    # equivalent to "my $line = <$socket>;"
    my $line = undef;
    my $char;
    while (sysread($socket, $char, 1) == 1) {
	$line .= $char;
	last
	    if ($char eq "\n");
    }

    if (defined $line) {
	if ($line ne "\n") {
	    print STDERR "RECV:$line";
	} else {
	    print STDERR ",";
	}
    } else {
	warn "read from server failed! $!"
    }
    return $line;
}

sub read_skip_chat ($)
{
    my ($this) = @_;
    while (1) {
	my $line = $this->read();
	return $line
	    unless ((defined $line) && ($line =~ /^\#\#/));
	return $line
	  if ($line =~ /^(\#\#\[CHALLENGE\])/);
	return $line
	  if ($line =~ /^(\#\#\[STUDY\])/ && ! $this->{in_game});
	print STDERR "IGNORED ".$line;
	$line =~ s/\#\#\[CHAT\]\[[A-Za-z0-9_@-]+\]\s+//;
	if ($line =~ /^([A-Za-z0-9_@-])+\s+(verbose|silent)/) {
	    my $command = $2;
	    if ($this->{name} =~ /$1/) {
		if ($command eq "verbose") {
		    print STDERR "verbose\n";
		    $this->{verbose} = 1;
		} else {
		    print STDERR "silent\n";
		    $this->{verbose} = 0;
		}
	    }
	}
    }
}

sub read_or_gameend ($)
{
    my ($this) = @_;
    my $line0 = $this->read_skip_chat();
    my $line = $line0;
    my @skipped;
    if ($line =~ /^\#/ || $line =~ /^\%/ ) {
        $this->{in_game} = 0;
	do
	{
	    push(@skipped, $line);
	    if ($line =~ /^\#(WIN|LOSE|DRAW|CHUDAN|MAX_MOVES)/) {
		if ($line =~ /^\#WIN/) {
		    $this->chat_com("kachi-mashita");
		} elsif ($line =~ /^\#DRAW/) {
		    $this->chat_com("hikiwake-deshita");
		} elsif ($line =~ /^\#LOSE/) {
		    $this->chat_com("make-mashita");
		}
		return (1,$line0,@skipped);
	    }
	    $line = $this->read_skip_chat();
	} while ($line =~ /^\#/ || $line =~ /^\%KACHI/);
	print STDERR "CsaClient: unknown path $line\n";
	return (1, $line,@skipped);	# このパスなんだろう, timeup?
    }
    return (0, $line);
}

sub connect ($$$)
{
    my ($this, $host, $port) = @_;
    print STDERR "SYSTEM:connect to $host:$port\n";

    die "No port" unless $port;
    my $iaddr   = inet_aton($host)               || die "no host: $host";
    my $paddr   = sockaddr_in($port, $iaddr);

    my $proto   = getprotobyname('tcp');
    my $sock;

    socket($sock, PF_INET, SOCK_STREAM, $proto)  || die "socket: $!";
    connect($sock, $paddr)    || die "connect: $!";

    $sock->autoflush (1);
    $this->{socket} = $sock;
    print STDERR "SYSTEM:connected\n";

    sleep 1;
    $this->{host} = $host;
    $this->{port} = $port;
}

sub login_x1 ($)
{
    my ($this) = @_;
    my $user = $this->{name};
    my $pass = $this->{password};
    $this->send("LOGIN $user $pass x1\n");

    my $line;
    my $user_without_trip = $user;
    $user_without_trip =~ s/,.*//;
    do
    {
	$line = $this->read();
    }
    while ($line !~ /LOGIN:($user_without_trip|incorrect)/);

    die "ERR :$line" 
	unless $line =~ /OK/;
    do
    {
	$line = $this->read();
    }
    while ($line !~ /\#\#\[LOGIN\] \+OK x1/);
    die "ERR :$line" 
	unless $line =~ /OK/;
    print STDERR "SYSTEM:login ok\n";
    $this->{x1} = 1;
}

sub login ($)
{
    my ($this) = @_;
    my $user = $this->{name};
    my $pass = $this->{password};
    $this->send("LOGIN $user $pass\n");

    my $line;
    do
    {
	$line = $this->read();
    }
    while ($line !~ /LOGIN:($user|incorrect)/);

    die "ERR :$line" 
	unless $line =~ /OK/;
}

sub logout($)
{
    my ($this) = @_;
    $this->send("LOGOUT\n");

    my $line;
    while (defined ($line = $this->try_read()))
    {
	print $line;
    }
}

sub disconnect ($)
{
    my ($this) = @_;
    close($this->{socket});
}

sub chat ($$)
{
    my ($this, $message) = @_;
    $this->send('%%CHAT ' . $message . "\n");
}
sub chat_com ($$)
{
    my ($this, $message) = @_;
    $this->chat("com: $message")
	if ($this->{verbose});
}

sub game_chat ($$)
{
  my ($this, $message) = @_;
  return unless $this->{host} =~ /81dojo/;
  $message = '%%GAMECHAT'.sprintf(" %s %s\n", $this->{gameid}, $message);
  $this->send($message);
}

sub csa2arrow ($$)
{
  my ($this, $move) = @_;
  return unless $move && $move =~ /^[+-][0-9]{4}[A-Z]{2}/;
  my ($t, $f0, $f1, $t0, $t1, $ptype) = split(//, $move, 6);
  my $msg;
  if ($f0) {
    $msg = sprintf("[##ARROW]%d,%d,%d,%d,%d,%s", -1,
		   $f0, $f1, $t0, $t1, "0xfefefe");
  } else {
    my $ptype_id = { HI=>1, KA=>2, KI=>3, GI=>4, KE=>5, KY=>6, FU=>7 };
    $msg = sprintf("[##ARROW]%d,%d,%d,%d,%d,%s", ($t eq '+' ? 0 : 1),
		   $ptype_id->{$ptype}, 0, $t0, $t1, "0xfefefe");
  }
  $this->game_chat($msg);
}


sub offer_game_x1 ($$$)
{
    my ($this,$gamename,$sente_string) = @_;
    my ($sec,$min,$hour,$mday,$mon,$year) = localtime(time);
    $year += 1900; $mon += 1;
    print STDERR "SYSTEM:offer_game ($year-$mon-$mday $hour:$min)\n";

    my $game_string = "%%GAME $gamename $sente_string\n"; 
    $this->send($game_string);
}

sub wait_opponent ($$@)
{
    my ($this, $csafile_basename, $allow_multiple_invocation) = @_;
    print STDERR "SYSTEM: wait_opponent\n";
    my $sente = -1;
    my $line;
    my $timeleft = 0;
    my $byoyomi = 0;
    my $sente_name = "unknown";
    my $gote_name  = "unknown";

    my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime(time);
    $year += 1900; $mon += 1;
    my $record_base = 
	sprintf ('%s%04d%02d%02d-%02d-%02d-%02d',
		 $csafile_basename,$year,$mon,$mday,$hour,$min,$sec);
    my $csafilename = "$record_base-$$.init";
    $this->{record_file} = "$record_base-comm-$$.csa";

    while (1)
    {
	$line = $this->read_skip_chat();
	return
	    if (! defined $line);

	if ($line =~ /^\#/) {
	  if ($line =~ /^\#\#\[CHALLENGE/) {
	    $this->send("ACCEPT\n");
	    next;
	  }
	    warn "unexpected $line";
	    next;
	}

	if ($line =~ /^Name\+:(.+)/)
	{
	    $sente_name = $1;
	}
	elsif ($line =~ /^Name\-:(.+)/)
	{
	    $gote_name = $1;
	}
	elsif ($line =~ /^Your_Turn:(?)/)
	{
	    $sente = 1 if $line =~ /\+/;
	    $sente = 0 if $line =~ /\-/;
	    print STDERR "SYSTEM: we are $sente\n";
	}
	elsif ($line =~ /^Total_Time:(.+)/)
	{
	    #we assume that Time_Unit is 1sec
	    $timeleft = $1;
	}
	elsif ($line =~ /^Byoyomi:(.+)/)
	{
	    $byoyomi = $1;
	    $byoyomi -= int(POSIX::floor($byoyomi / 10))
		if ($byoyomi / 10 > 0);
	}
	elsif ($line =~ /^BEGIN Position/)
	{
	    print STDERR "SYSTEM:$line";
	    open CSAFILE, "> $csafilename"
		|| die "CsaClient: open $!";
	    print CSAFILE "N+$sente_name\n";
	    print CSAFILE "N-$gote_name\n";
	    my $moves = 0;
	    while (1)
	    {
		$line = $this->read_skip_chat();
		next
		    if ($line =~ /^Jishogi_Declaration:/);
		last
		    if ($line =~ /^END Position/);
		print CSAFILE $line;
		++$moves if ($line =~ /^[+-][0-9]{4}[A-Z]{2}/);
	    }
	    close CSAFILE;
	    system "cp", "$csafilename", $this->{record_file};
	    chmod 0644, $this->{record_file};
	    $this->{initial_moves} = $moves;
	}
	last
	    if ($line =~ /^END Game_Summary/);
    }
    sleep 2;
    my $my_reject = 0;
    if ($allow_multiple_invocation
	|| symlink($$, $this->{lockfile})) {
	$this->send("AGREE\n");
	system($this->{start_hook}) if $this->{start_hook};
    } else {
        warn "send reject!";
	$this->send("REJECT\n");
	$my_reject = 1;
    }
    $line = $this->read_skip_chat();
    if ($line =~ /^REJECT/) {
	$sente = -2;
	unlink($this->{lockfile}) unless $my_reject;
    }
    if ($sente >= 0) {
      $this->{in_game} = 1;
      $this->{sente} = $sente;
      chomp $line;
      $this->{gameid} = ($line =~ /^START:([^:]+)/) ? $1 : "";
    }
    $this->{last_eval} = 0;
    $this->{totaltime} = $timeleft;
    $this->{byoyomi} = $byoyomi;
    $this->{opname} = ($sente ? $gote_name : $sente_name);
    return ($sente, $csafilename, $this->{opname},
	    $timeleft, $byoyomi);
}

sub record_move ($$) {
    my ($handle, $move) = @_;
    chomp $move;
    if ($move =~ /^[#%]/) {
	print $handle "'",$move."\n";
    }
    elsif ($move =~ /^(.*),(T\d+)$/) {
	print $handle $1."\n";
	print $handle $2."\n";
    }
    else {
	print $handle $move."\n";
    }
}

#
# $program : new GpsShogi したもの(など)
sub play ($$) {
    my ($this, $program) = @_;
    if (! $this->{in_game}) {
	warn "CsaClient: cannot play not in_game status\n";
	unlink($this->{lockfile});
	system($this->{finish_hook}) if ($this->{finish_hook});
	return;
    }
    $program->set_master_record($this->{record_file});
    my $record_handle;
    open $record_handle, ">> ".$this->{record_file}
	|| die "open $! " . $this->{record_file}."\n";
    $record_handle->autoflush(1);

    if (($this->{sente} == 0 && $this->{initial_moves} %2 == 0)
	|| ($this->{sente} && $this->{initial_moves} %2 == 1)) {
	# op turn
	my ($gameend,$line,@skip) = $this->read_or_gameend();
	record_move($record_handle, $line);
	map { record_move($record_handle, $_) } @skip;
	if ($gameend) {
	    $program->send ('%TORYO'."\n");
	    unlink($this->{lockfile});
	    system($this->{finish_hook}) if ($this->{finish_hook});
	    return;
	}
	$line =~ s/,T(\d+)$// unless $program->need_servertime;
	$program->send ($line);
    }

    while (1) {
	# my turn
	my ($line,$gameend,@skip,$timeout);
	while (1) {
	  ($line, $timeout) = $program->read_or_timeout(90.0);
	  last unless $timeout;
	  $this->send("\n") if $this->{x1};
	}
	last
	    if (! defined $line);

	if (defined $line && $line !~ /^%CHUDAN/) { # %TORYOの時は undefined
	  sleep $this->{config}->{sleep_before_move}
	    if $this->{config}->{sleep_before_move};
	  $this->send($line);
	  if ($this->{host} =~ /81dojo/
	      && $line =~ /^[+-][0-9]{4}[A-Z]{2},\s*\'\*\s+(-?[0-9]+)(\s+.*)?/) {
	    my ($eval, $pv) = ($1, $2);
	    if ($this->{gameid} =~ /\+nr_/ # non rated
		&& defined $this->{last_eval} && abs($eval-$this->{last_eval}) > 150
		&& ((abs($eval) < 500 && abs($this->{last_eval}) < 500)
		    || abs($eval-$this->{last_eval}) >= 1000)) {
	      my $sym = (($eval-$this->{last_eval})*($this->{sente}?1:-1)<-100) ? "!" : "?";
	      $this->game_chat(sprintf("%s (%d => %d)", $sym, $this->{last_eval}, $eval));
	    }
	    if ($pv) {
	      $pv =~ s/^\s*//;
	      my ($pv1, $pv2) = split(/\s+/, $pv);
	      $this->csa2arrow($pv1);
	      $this->csa2arrow($pv2) if $pv2;
	    }
	    $this->{last_eval} = $eval;
	  }
	}
	($gameend,$line,@skip) = $this->read_or_gameend();
	$program->send ($line) if $program->need_servertime;
	record_move($record_handle, $line);
	map { record_move($record_handle, $_) } @skip;

	if ($gameend) {
	  $program->send ('%TORYO'."\n");
	  last;
	}

	# op turn
	($gameend,$line,@skip) = $this->read_or_gameend();
	record_move($record_handle, $line);
	map { record_move($record_handle, $_) } @skip;
	if ($gameend) {
	    $program->send ('%TORYO'."\n");
	    last;
	}

	$line =~ s/,T(\d+)$// unless $program->need_servertime;
	$program->send ($line);
	if ($line =~ /^%/) {
	    ($gameend,$line,@skip) = $this->read_or_gameend();
	    warn "game does not end $line"
		unless $gameend;
	    last
		if ($gameend);
	}
    }
    close $record_handle;
    unlink($this->{lockfile});
    system($this->{finish_hook}) if ($this->{finish_hook});
}

sub study ($$) {
  my ($this, $program) = @_;
  $this->game_chat("[##GIVEHOST]$this->{opname}");
  $this->{last_study} = time;
  my $id = 0; while (1) {
    my $line = $this->try_read_in_sec(120.0);
    return unless $line;
    return if $line =~ /^\[\#\#(LEAVE|DISCONNECT)\]$this->{opname}$/;
    while (my $tmp = $this->try_read_in_sec(0.1)) {
      return if $tmp =~ /^\[\#\#(LEAVE|DISCONNECT)\]$this->{opname}$/;
      $line = $tmp if $tmp =~ /\[\#\#STUDY\]/;
    }
    unless ($line =~ /^\#\#\[GAMECHAT\]\[.*\]\s*\[\#\#STUDY\]\s*([0-9]+)\/(\*|([0-9A-Z\/+-]+))$/) {
      return if time > $this->{last_study}+300;
      next;
    }
    $this->{last_study} = time;
    ++$id;
    $this->game_chat("[##ARROW]CLEAR");
    my ($moves, $additonal) = ($1, $2);
    my ($turn, $filename) = $this->make_studyfile($id, $moves, $additonal);
    my $ret = $program->newsearch_for_study($filename, $turn);
    if ($ret =~ /^([+-][0-9]{4}[A-Z]{2})/) {
      my $response = $1;
      $this->csa2arrow($response);
      if ($ret =~ /^[+-][0-9]{4}[A-Z]{2},\s*\'\*\s+(-?[0-9]+)\s+(.+)/) {
	my $pv = $2;
	if ($pv) {
	  $pv =~ s/^\s*//;
	  my ($pv1) = split(/\s+/, $pv);
	  $this->csa2arrow($pv1);
	}
      }
    }
  }
}


sub make_studyfile ($$$$) {
  my ($this, $id, $moves, $additonal) = @_;
  my $filename = $this->{record_file};
  $filename =~ s/-comm-[0-9]+.csa$/-study-$id.csa/;
  open (IN, $this->{record_file}) || die "open $!";
  open (OUT, ">".$filename) || die "open $!";
  my $count = 0;
  my $last_move = undef;
  while (<IN>) {
    chomp;
    next if /^%/;
    if (/^[+-][0-9]/) {
      last if $count++ >= $moves;
      $last_move = $_;
    }
    print OUT $_."\n";
  }
  close IN;
  map { $last_move = $_; print OUT $_."\n" } split(/\//, $additonal)
    if ($additonal ne '*');
  close OUT;
  my $turn = ($last_move && $last_move =~ /^\+/) ? "-" : "+";
  return ($turn, $filename);
}


# END
return 1;
