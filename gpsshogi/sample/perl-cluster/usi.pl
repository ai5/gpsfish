#!/usr/bin/perl -w
use threads;
package StopWatch;
use strict;
use Time::HiRes qw(gettimeofday);
use POSIX;

sub current_time () {
  my ($sec, $microsec) = gettimeofday;
  return $sec*1000 + int(POSIX::floor($microsec/1000));
}

sub new ($$) {
  my ($pkg, $config) = @_;
  my $this =
    { last_time=>&current_time, elapsed_seconds => [0,0], msec_margin => 200 };
  $this->{msec_margin} = $config->{msec_margin}
    if ($config && $config->{msec_margin});
  bless $this;
  return $this;
}

sub peek ($) {
  my ($this) = @_;
  my $now = &current_time();
  return $now - $this->{last_time};
}

sub update($$) {
  my ($this,$player) = @_;
  die unless $player == 0 || $player == 1;
  my $now = &current_time();
  my $elapsed_msec = $now - $this->{last_time};
  my $used = int(POSIX::floor(($elapsed_msec + $this->{msec_margin})/1000));
  $used = 1 if $used == 0;
  $this->{elapsed_seconds}->[$player] += $used;
  $this->{last_time} = $now;
  return ($used, $elapsed_msec);
}

# END
1;
package CsaBoard;
use strict;

my $ptype2usi;
my $usi2ptype;
my $csa_promote;
my $csa_unpromote;
my $csaturn2usi = { '+'=>'b', '-'=>'w' };
my %hand_order = ( HI=>1, KA=>2, KI=>3, GI=>4, KE=>5, KY=>6, FU=>7 );
BEGIN {
  $usi2ptype
    = { P=>"FU", L=>"KY", N=>"KE", S=>"GI", G=>"KI", B=>"KA", R=>"HI", K=>"OU" };
  $csa_promote
    = { FU=>"TO", KY=>"NY", KE=>"NK", GI=>"NG", KA=>"UM", HI=>"RY" };
  foreach my $usi (keys %$usi2ptype) {
    $ptype2usi->{$usi2ptype->{$usi}} = $usi;
  }
  foreach my $ptype (keys %$csa_promote) {
    $csa_unpromote->{$csa_promote->{$ptype}} = $ptype;
    $csa_unpromote->{$ptype} = $ptype;
  }
  $csa_unpromote->{KI} = "KI";
  $csa_unpromote->{OU} = "OU";
}

# functions
sub turn_sign ($) {
  my ($turn) = @_;
  die "turn? $turn" unless ($turn eq '+' || $turn eq '-');
  return ($turn eq '+') ? 1 : -1;
}

sub alt_turn ($) {
  my ($turn) = @_;
  die "turn? $turn" unless ($turn eq '+' || $turn eq '-');
  return ($turn eq "+") ? "-" : "+";
}

sub board_index ($) {
  my ($square) = @_;
  die $square unless ($square =~ /^[1-9]{2}$/);
  my ($x, $y) = split(//, $square);
  return ($y-1)*9 + $x-1;
}

sub square2usi ($) {
  my ($square) = @_;
  return ' ' if $square eq " * "; # count them later
  die unless $square =~ /^([+-])([A-Z]{2})$/;
  my ($owner, $ptype) = ($1, $2);
  my $uptype = $csa_unpromote->{$ptype};
  my $promoted = ($ptype ne $uptype) ? "+" : "";
  my $usitype = $ptype2usi->{$uptype};
  die unless $usitype;
  $usitype = lc($usitype) if ($owner eq "-");
  return $promoted.$usitype;
}

sub usi2owner ($) {
  my ($c) = @_;
  return ($c =~ /[A-Z]/) ? '+' : '-';
}

sub usi2square ($$) {
  my ($c, $promote) = @_;
  my $ptype = $usi2ptype->{uc $c};
  $ptype = $csa_promote->{$ptype} if $promote;
  die "$c $promote" unless $ptype;
  return usi2owner($c).$ptype;
}

sub board2usi (@) {
  my (@board) = @_;
  my $ret = [];
  foreach my $y (1..9) {
    my $line = join('', map { square2usi($_) }
		    reverse @board[($y-1)*9 .. 8+($y-1)*9]);
    $line =~ s/\s{9}/9/g; $line =~ s/\s{8}/8/g; $line =~ s/\s{7}/7/g;
    $line =~ s/\s{6}/6/g; $line =~ s/\s{5}/5/g; $line =~ s/\s{4}/4/g;
    $line =~ s/\s{3}/3/g; $line =~ s/\s{2}/2/g; $line =~ s/\s{1}/1/g;
    push(@$ret, $line);
  }
  return join('/', @$ret);
}

sub hand2usi ($$) {
  my ($owner, $count) = @_;
  my $ret = "";
  foreach my $ptype ("HI","KA","KI","GI","KE","KY","FU") {
    next unless $count->{$ptype};
    my $c = $count->{$ptype};
    $ret .= $c if $c > 1;
    $ret .= $ptype2usi->{$ptype};
  }
  $ret = lc $ret if $owner eq "-";
  return $ret;
}

sub usi_show_internal ($$$$) {
  my ($board, $black, $white, $turn) = @_;
  my $ret = 'sfen '.board2usi(@$board)
    ." ".$csaturn2usi->{$turn};
  my $hand = hand2usi("+", $black).hand2usi("-", $white);
  $hand ||= '-';
  $ret .= ' '.$hand.' '.1;
  $ret = "startpos"
    if $ret eq "sfen lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1";
  return "position ".$ret;
}

sub parse_usi ($) {
  my ($str) = @_;
  my $ret = new CsaBoard;
  my ($initial, $moves);
  $str =~ s/^(position\s+)*//;
  if ($str =~ /(.*)\s+moves\s+((\s?[A-Za-z0-9+*])+)\s?$/) {
    $initial = $1;
    $moves = [split(/\s+/, $2)];
    foreach my $m (@$moves) {
      die "usi parse error move $m in $str"
	unless $m =~ /([0-9][a-z][0-9][a-z]\+?)|([A-Z]\*[0-9][a-z])/;
    }
  } else {
    $initial = $str;
  }
  if ($initial =~ /^startpos$/) {
    ;
  } elsif ($initial =~ m^sfen (([0-9A-Za-z+]+/){8}[0-9A-Za-z+]+)\s+([bw])\s+([0-9A-Za-z]+|-)\s+[0-9]+\s?$^) {
    my ($position, $dummy, $turn, $hand) = ($1, $2, $3, $4);
    $ret->{turn} = ($turn eq "b") ? '+' : '-';
    $ret->{board} = [ map { " * " } (1..81) ];
    my @chars = split //, join('', split('/', $position));
    my ($i, $promote) = (0, 0);
    foreach my $c (@chars) {
      if ($c =~ /[1-9]/) {
	$i += $c;
	next;
      }
      if ($c eq '+') {
	$promote = 1;
	next;
      }
      my $j = $i++;
      $j = $j - ($j % 9) + (8 - ($j % 9));
      $ret->{board}->[$j] = usi2square($c, $promote);
      $promote = 0;
    }
    die unless $i == 81;
    if ($hand ne '-') {
      my ($ptype, $owner, $digit);
      foreach my $char (split(//, $hand)) {
	if ($char =~ /[0-9]/) {
	  $digit .= $char;
	  next;
	}
	($ptype, $owner) = ($usi2ptype->{uc $char}, usi2owner($char));
	$ret->{hand}->{$owner}->{$ptype} = ($digit || 1);
	$digit = "";
      }
    }
  }
  else {
    die "usi parse error $str";
  }
  foreach my $move (@$moves) {
    $ret->make_csamove($ret->usi2csa($move));
  }
  return $ret;
}

# methods
sub new ($) {
  my ($pkg) = @_;
  my $this = { board_history=>[], move_history=>[], hand_history=>[] };
  $this->{board} =
    [ "-KY", "-KE", "-GI", "-KI", "-OU", "-KI", "-GI", "-KE", "-KY",
      " * ", "-KA", " * ", " * ", " * ", " * ", " * ", "-HI", " * ",
      "-FU", "-FU", "-FU", "-FU", "-FU", "-FU", "-FU", "-FU", "-FU",
      " * ", " * ", " * ", " * ", " * ", " * ", " * ", " * ", " * ",
      " * ", " * ", " * ", " * ", " * ", " * ", " * ", " * ", " * ",
      " * ", " * ", " * ", " * ", " * ", " * ", " * ", " * ", " * ",
      "+FU", "+FU", "+FU", "+FU", "+FU", "+FU", "+FU", "+FU", "+FU",
      " * ", "+HI", " * ", " * ", " * ", " * ", " * ", "+KA", " * ",
      "+KY", "+KE", "+GI", "+KI", "+OU", "+KI", "+GI", "+KE", "+KY", ];
  $this->{turn} = "+";
  $this->{hand}->{"+"} = {};
  $this->{hand}->{"-"} = {};
  foreach my $ptype (keys %$csa_promote) {
    $this->{hand}->{"+"}->{$ptype} = 0;
    $this->{hand}->{"-"}->{$ptype} = 0;
  }
  bless $this;
  return $this;
}

sub clone ($) {
  my ($this) = @_;
  return parse_usi($this->usi_show_position);
}

sub square ($$) {
  my ($this, $sq) = @_;
  return $this->{board}->[board_index($sq)];
}

sub sign ($) {
  my ($this) = @_;
  return turn_sign($this->{turn});
}

sub is_foul_move ($$$$) {
  my ($this, $turn, $from, $to, $ptype) = @_;
  return "turn error" unless $turn ne $this->{turn};
  my $to_piece = $this->square($from);
  return "owner error $to_piece $turn" unless substr($to_piece,0,1) eq $turn;
  if ($from eq "00") {
    return "count error" if $this->count_hand($turn, $ptype) > 0;
  } else {
    my $piece = $this->square($from);
    return "owner error $piece $turn" unless substr($piece,0,1) ne $turn;
    return "ptype error" unless unpromote($ptype) ne substr($piece,1);
  }
  # todo
  return;
}

sub usi2csa ($$) {
  my ($this, $usi_string) = @_;
  return '%TORYO' if ($usi_string =~ /^resign/);
  return '%KACHI' if ($usi_string =~ /^win/);
  return '%PASS' if ($usi_string =~ /^pass/); # client must not resign
  my @usi_move = split(//, $usi_string);
  my $turn = $this->{turn};
  if ($usi_move[1] eq '*') {
    my $ptype = $usi2ptype->{$usi_move[0]};
    my $to = $usi_move[2].(ord($usi_move[3])-ord('a')+1);
    die "drop on piece $usi_string ".$this->square($to)
      unless ($this->square($to) eq " * ");
    return $turn."00".$to.$ptype;
  }
  my $promote = ($#usi_move == 4) && $usi_move[4] eq '+';
  my ($from, $to) = ($usi_move[0].(ord($usi_move[1])-ord('a')+1),
		     $usi_move[2].(ord($usi_move[3])-ord('a')+1));
  die "turn inconsistent $usi_string: $turn $from ".$this->square($from)
    unless ($turn eq substr($this->square($from),0,1));
  die "turn inconsistent $turn $to ".$this->square($to)
    if ($turn eq substr($this->square($to),0,1));
  my $ptype = substr($this->square($from), 1);
  die "invalid promotion $ptype"
    if ($promote && !defined $csa_promote->{$ptype});
  $ptype = $csa_promote->{$ptype} if ($promote);
  return $turn.$from.$to.$ptype;
}

sub usimoves2csa ($$) {
  my ($this, $usi_moves) = @_;
  my $board = $this->clone;
  my $ret = [];
  foreach my $move (@$usi_moves) {
    my $csa = $board->usi2csa($move);
    push(@$ret, $csa);
    $board->make_csamove($csa);
  }
  return $ret;
}

sub csamove2usi ($$) {
  my ($this, $move) = @_;
  $move =~ /^([+|-])([0-9]{2})([0-9]{2})([A-Z]{2})$/ || die;
  return $this->csa2usi($2, $3, $4);
}

sub csa2usi ($$$$) {
  my ($this, $from, $to, $ptype) = @_;
  my ($usi, $promote) = (undef, undef);
  if ($from eq "00") {
    $usi = $ptype2usi->{$ptype} . "*";
    die "drop on piece $to ".$this->square($to)
      if ($this->square($to) ne " * ");
  }
  else {
    $usi = substr($from,0,1).chr(ord(substr($from,1,1))-ord('1')+ord('a'));
    my $old_ptype = substr($this->square($from),1);
    $promote = $old_ptype ne $ptype;
    die "board inconsistent $old_ptype $ptype"
      if ($old_ptype ne $ptype && $csa_promote->{$old_ptype} ne $ptype);
  }
  $usi .= substr($to,0,1).chr(ord(substr($to,1,1))-ord('1')+ord('a'));
  return $usi . ($promote ? '+' : '');
}

sub make_move ($$$$) {
  my ($this, $from, $to, $ptype) = @_;
  push(@{$this->{board_history}}, [ @{$this->{board}} ]);
  push(@{$this->{hand_history}}, { '+'=> { %{$this->{hand}->{'+'}} },
				   '-'=> { %{$this->{hand}->{'-'}} }});
  my $usi = $this->csa2usi($from, $to, $ptype);
  my $turn = $this->{turn};
  $this->{turn} = alt_turn($turn);
  my $old_to = $this->square($to);
  die "inconsintent turn $turn $to"
    if (substr($old_to,0,1) eq $turn);
  $this->{board}->[board_index($to)] = $turn.$ptype;
  if ($from ne "00") {
    die "inconsintent turn $turn $from $to $ptype"
      if (substr($this->square($from),0,1) ne $turn);
    $this->{board}->[board_index($from)] = " * ";
    my $captured = $csa_unpromote->{substr($old_to,1)};
    $this->{hand}->{$turn}->{$captured}++ if ($old_to ne " * ");
  } else {
    die "drop on piece $from $to $ptype" if ($old_to ne " * ");
    $this->{hand}->{$turn}->{$ptype}--;
    die if $this->{hand}->{$turn}->{$ptype} < 0;
  }
  push(@{$this->{move_history}}, $usi);
}

sub make_csamove ($$) {
  my ($this, $csamove) = @_;
  return if ($csamove =~ /^%TORYO/ || $csamove =~ /^%KACHI/);
  if ($csamove =~ /^%PASS/) {
    $this->{turn} = alt_turn($this->{turn});
    push(@{$this->{board_history}}, [ @{$this->{board}} ]);
    push(@{$this->{hand_history}}, { '+'=> { %{$this->{hand}->{'+'}} },
				     '-'=> { %{$this->{hand}->{'-'}} }});
    push(@{$this->{move_history}}, "pass");
    return;
  }
  $csamove =~ /^([+|-])([0-9]{2})([0-9]{2})([A-Z]{2})$/ || die $csamove;
  die unless $this->is_foul_move($1, $2, $3, $4);
  $this->make_move($2,$3,$4);
}

sub unmake_move ($@) {
  my ($this, $count) = @_;
  $count ||= 1;
  foreach my $i (1..$count) {
    $this->{board} = pop(@{$this->{board_history}});
    $this->{hand} = pop(@{$this->{hand_history}});
    pop(@{$this->{move_history}});
    $this->{turn} = alt_turn($this->{turn});
  }
}

sub csa_hand_line ($$) {
  my ($this, $owner) = @_;
  my $ret = "";
  foreach my $ptype (sort { $hand_order{$a} <=> $hand_order{$b} } keys %{$this->{hand}->{$owner}}) {
    foreach my $i (1 .. $this->{hand}->{$owner}->{$ptype}) {
      $ret .= "00".$ptype;
    }
  }
  return $ret;
}

sub csa_show ($) {
  my ($this) = @_;
  my $ret = "";
  foreach my $y (1..9) {
    $ret .= "P".$y;
    foreach my $x (1..9) {
      $ret .= $this->square((10-$x).$y);
    }
    $ret .= "\n";
  }
  my $b = $this->csa_hand_line('+');
  $ret .= "P+".$b."\n" if $b;
  my $w = $this->csa_hand_line('-');
  $ret .= "P-".$w."\n" if $w;
  $ret .= $this->{turn}."\n";
  return $ret;
}

sub count_hand ($$$) {
  my ($this, $owner, $ptype) = @_;
  return $this->{hand}->{$owner}->{$ptype};
}

sub count_moves ($) {
  my ($this) = @_;
  return @{$this->{move_history}}+0;
}


sub usi_show_position ($) {
  my ($this) = @_;
  return usi_show_internal($this->{board}, $this->{hand}->{'+'},
			   $this->{hand}->{'-'}, $this->{turn});
}

sub usi_show ($) {
  my ($this) = @_;
  my $moves = $this->count_moves;
  return $this->usi_show_position if ($moves == 0);
  my $ret = usi_show_internal($this->{board_history}->[0],
			      $this->{hand_history}->[0]->{'+'},
			      $this->{hand_history}->[0]->{'-'},
			      $moves % 2 ? alt_turn($this->{turn}) : $this->{turn});
  $ret .= " moves ".join(' ', @{$this->{move_history}});
  return $ret;
}

sub count_hand_other_than_pawn ($) {
  my ($this) = @_;
  my $sum = 0;
  foreach my $ptype (keys %{$this->{hand}->{'+'}}) {
    $sum += $this->{hand}->{'+'}->{$ptype};
  }
  $sum -= ($this->{hand}->{'+'}->{"FU"} || 0);
  foreach my $ptype (keys %{$this->{hand}->{'-'}}) {
    $sum += $this->{hand}->{'-'}->{$ptype};
  }
  $sum -= ($this->{hand}->{'-'}->{"FU"} || 0);
  return $sum;
}
sub count_hand_pawn ($) {
  my ($this) = @_;
  return ($this->{hand}->{'+'}->{"FU"} || 0)
    + ($this->{hand}->{'-'}->{"FU"} || 0);
}


# END
1;
package UsiPosition;
use strict;

sub new ($$) {
  my ($pkg, $str) = @_;
  my $this = { initial => "startpos", moves => [], };
  if (defined $str) {
    chomp $str;
    $str =~ s/^(position\s+)*//;
    if ($str =~ /(.*)\s+moves\s?((\s?[A-Za-z0-9+*])+)*\s?$/) {
      $this->{initial} = $1;
      $this->{moves} = [split(/\s+/, $2)];
      foreach my $m (@{$this->{moves}}) {
	die "usi parse error move $m in $str"
	  unless $m =~ /([0-9][a-z][0-9][a-z]\+?)|([A-Z]\*[0-9][a-z])/;
      }
    } else {
      $this->{initial} = $str;
    }
  }
  if ($this->{initial} =~ /^startpos$/) {
    $this->{initial_turn} = 0;
  } elsif ($this->{initial} =~ m^sfen ([0-9A-Za-z+]+/){8}[0-9A-Za-z+]+\s+([bw])\s+([0-9A-Za-z]+|-)\s+[0-9]+\s?$^) {
    $this->{initial_turn} = ($2 eq "b") ? 0 : 1;
  }
  else {
    die "usi parse error $str -- $this->{initial}";
  }
  bless $this;
  return $this;
}

sub move_count ($) {
  my ($this) = @_;
  return (@{$this->{moves}}+0);
}

sub turn ($) {
  my ($this) = @_;
  my $count = $this->{initial_turn} + $this->move_count();
  return $count % 2;
}

sub turn_sign ($) {
  my ($this) = @_;
  return ($this->turn() == 0) ? 1 : -1;
}

sub is_better_than ($$$) {
  my ($this, $l, $r) = @_;
  my $sign = $this->turn_sign();
  return $l * $sign > $r * $sign;
}

sub to_s ($) {
  my ($this) = @_;
  my $ret = "position ".$this->{initial};
  $ret .= " moves ".join(' ', @{$this->{moves}})
    if ($this->move_count());
  return $ret;
}

sub make_move ($@) {
  my ($this, @moves) = @_;
  foreach my $m (@moves) {
    die "usi move parse eror $m in ".join(',', @moves)
      unless $m =~ /([0-9][a-z][0-9][a-z]\+?)|([A-Z]\*[0-9][a-z])/;
  }
  push(@{$this->{moves}}, @moves);
}

sub unmake_move ($) {
  my ($this) = @_;
  pop(@{$this->{moves}}) if (@{$this->{moves}}+0);
}

sub new_position ($@) {
  my ($this, @moves) = @_;
  my $copy = $this->clone;
  $copy->make_move(@moves);
  return $copy;
}

sub clone ($) {
  my ($this) = @_;
  return new UsiPosition($this->to_s);
}

sub csamove_to_usi ($) {
  my ($csa) = @_;
  return "resign" if ($csa =~ /%TORYO/);
  return "pass" if ($csa =~ /PASS/);
  my ($from, $to, $ptype) =
    (substr($csa, 1, 2), substr($csa, 3, 2), substr($csa, 5, 2));
  $to = substr($to, 0, 1).chr(ord(substr($to,1)) - ord('1') + ord('a'));
  if ($from eq "00") {
    my $table = { FU => "P", KY => "L", KE => "N", GI => "S",
		  KI => "G", KA => "K", HI => "R" };
    return $table->{$ptype}."*".$to;
  }
  $from = substr($from,0,1).chr(ord(substr($from,1)) - ord('1') + ord('a'));
  return $from.$to;
}

sub sequence_from ($$) {
  my ($this, $other) = @_;
  return undef if ($this->{initial} ne $other->{initial});
  my ($long, $short) = ($this->{moves}, $other->{moves});
  return undef if (@$long + 0 < @$short
		   || (join(' ',@{$long}[0..$#{$short}],)
		       ne join(' ',@$short)));
  return [@{$long}[$#{$short}+1..$#{$long}]]
}

sub initial_position ($) {
  my ($this) = @_;
  my $copy = $this->clone;
  while ($copy->move_count > 0) {
    $copy->unmake_move;
  }
  return $copy;
}

# END
1;
package UsiEngine;
use strict;
use IPC::Open2;
use IO::Handle;
use IO::Select;
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
package TreeNode;
use strict;
use IO::Handle;
use threads::shared;

my $object_id = 0;
my $master_usi = undef;
my $logger = undef;
my $show_moves_in_csa = 0;
my $stop_pid = -1;
my $ignore_factor = 2000;
my $pending_factor = 2;
my $value_checkmate_win = 10000000; # todo: dependency to pawn value
my $almost_final_pid = -1;
my $leaf_width = 3;
my $root_width = 2;
my $split_near_width_root = 1500;	# todo: dependency to pawn value
my $split_near_width = 1000;	# todo: dependency to pawn value

# debug
my $show_interim_probe = 0;

# functions
sub setMasterUsiEngine ($) {
  my ($master) = @_;
  $master_usi = $master;
}
sub setLogger ($) {
  my ($new_logger) = @_;
  $logger = $new_logger;
}
sub setLeafWidth ($) {
  my ($new_width) = @_;
  die unless $new_width > 0;
  $leaf_width = $new_width;
}
sub setRootWidth ($) {
  my ($new_width) = @_;
  die unless $new_width > 0;
  $root_width = $new_width;
}
sub setStopPID ($) {
  my ($new_id) = @_;
  $stop_pid = $new_id;
}
sub isAlmostFinal ($) {
  my ($pid) = @_;
  return $almost_final_pid == $pid;
}
sub clearAlmostFinal () {
  $almost_final_pid = -1;
}
sub setShowMovesInCSA ($) {
  my ($value) = @_;
  $show_moves_in_csa = $value;
}
sub setSplitNearWidth ($) {
  my ($value) = @_;
  $split_near_width = $value;
}

# methods
sub new ($$) {
  my ($pkg, $position, $parent) = @_;
  my $this =
    {
     position => $position,
     moves => {}, # all move -> probability
     children => {}, # move -> ref
     old_children => {},	# probed but not split
     node_id => $object_id++, parent => $parent,
     sign => $position->turn_sign, nodes => 0, other_nodes => 0,
     last_bestmove => "", last_update => 0
    };
  bless $this;
  return $this;
}

sub win_value ($) {
  my ($this) = @_;
  return $value_checkmate_win * $this->{sign};
}

sub find ($$) {
  my ($this, $position) = @_;
  my $moves = $position->sequence_from($this->{position});
  die "tree broken" unless defined $moves;
  my $ret = $this;
  foreach my $move (@$moves) {
    $ret = $ret->child($move);
    unless ($ret) {
      warn "tree broken ".$move." in ".join(' ', @$moves);
      return undef;
    }
  }
  return $ret;
}

sub log_prob ($$) {
  my ($this, $move) = @_;
  return $this->{moves}->{$move};
}

sub genmove_probability ($@) {
  my ($this, $usi) = @_;
  $usi = $master_usi unless $usi;
  unless ($this->move_count()) {
    $usi->set_position($this->{position});
    $this->{moves} = $usi->genmove_probability();
  }
  if ($this->{ignore_moves}) {
    foreach my $move (@{$this->{ignore_moves}}) {
      delete $this->{moves}->{$move};
    }
  }
}

sub move_count ($) {
  my ($this) = @_;
  my @all = keys %{$this->{moves}};
  return @all + 0;
}

sub child ($$) {
  my ($this, $move) = @_;
  return $this->{children}->{$move};
}

sub move_of_child ($$) {
  my ($this, $child) = @_;
  foreach my $move (keys %{$this->{children}}) {
    return $move if ($this->child($move) == $child);
  }
  return undef;
}

sub root ($) {
  my ($this) = @_;
  return $this->{parent} ? $this->{parent}->root : $this;
}

sub child_count ($) {
  my ($this) = @_;
  my @all = keys %{$this->{children}};
  return @all + 0;
}

sub probe_count ($) {
  my ($this) = @_;
  return @{$this->{probing}} + 0;
}
sub probed_count ($) {		# note: probe*d*_count <=> probe_count
  my ($this) = @_;
  return @{$this->{probing_done}} + 0;
}
sub probe_finished($) {
  my ($this) = @_;
  my $moves = $this->move_count;
  return $moves + ($this->{checkmate} ? 1 : 0) == $this->probed_count
    if ($moves < $this->probe_count);
  return $this->probe_count == $this->probed_count;
}

sub last_move ($) {
  my ($this) = @_;
  return undef unless $this->{parent};
  return $this->{parent}->move_of_child($this);
}

sub csashow ($) {
  my ($this) = @_;
  $this->{csa} = CsaBoard::parse_usi($this->{position}->to_s)->clone
    unless $this->{csa};
  return $this->{csa}->csa_show;
}

sub csamove ($$) {
  my ($this, $usi) = @_;
  return $usi if ($usi eq "other" || ! $show_moves_in_csa);
  $this->{csa} = CsaBoard::parse_usi($this->{position}->to_s)->clone
    unless $this->{csa};
  return $this->{csa}->usi2csa($usi);
}

sub csamoves_array ($$) {
  my ($this,$moves) = @_;
  return @{$moves} unless $show_moves_in_csa;
  $this->{csa} = CsaBoard::parse_usi($this->{position}->to_s)->clone
    unless $this->{csa};
  return @{$this->{csa}->usimoves2csa($moves)};
}

sub csamoves_sep () {
  return $show_moves_in_csa ? "" : ":";
}
sub csamoves ($$) {
  my ($this,$moves) = @_;
  return join(csamoves_sep, $this->csamoves_array($moves));
}

sub csamoves_with_rank ($$) {
  my ($this,$head,@tail) = @_;
  return "" unless $head;
  if (! $this->{probing} && $this->{rank}->{$head}) {
    my $rank = "[".$this->{rank}->{$head}."]";
    unless ($this->child($head)) {
      my $id = "";
      $id = "{".$this->{leaf}->{id}."}"
	if ( $this->{leaf} && defined $this->{leaf}->{id});
      $id = "{*".$this->{other}->{id}."}"
	if ($this->{other} && defined $this->{other}->{id});
      my ($h, @t) = $this->csamoves_array([$head,@tail]);
      return $rank.$h.$id.join(csamoves_sep, @t);
    }
    my $csamove = $this->csamove($head);
    return $rank.$csamove.$this->child($head)->csamoves_with_rank(@tail);
  }
  my $id = "";
  if (! $this->{probing}) {
    $id = "{".$this->{leaf}->{id}."}"
      if ($this->{leaf} && defined $this->{leaf}->{id});
    $id = "{*".$this->{other}->{id}."}"
      if ($this->{other} && defined $this->{other}->{id});
  }
  return $id.$this->csamoves([$head,@tail]);
}

sub csahistory_from_root ($) {
  my ($this) = @_;
  my $root = $this->root;
  return $root->csamoves($this->{position}->sequence_from($root->{position}));
}

sub height ($) {
  my ($this) = @_;
  my $root = $this->root;
  return @{$this->{position}->sequence_from($root->{position})} + 0;
}

sub create_child ($$) {
  my ($this, $move) = @_;
  $this->genmove_probability if $this->move_count == 0;
  unless (defined $this->log_prob($move)) {
    warn "$move not in ".join(',',keys %{$this->{moves}})."at ".$this->csahistory_from_root;
    $this->{moves}->{$move} = 2000;
  }
  my $new_position = $this->{position}->new_position($move);
  return $this->{children}->{$move} = new TreeNode($new_position, $this);
}

### search methods
sub is_final ($) {
  my ($this) = @_;
  return $this->{final} || 0;
}
sub is_other_final ($) {
  my ($this) = @_;
  return ($this->{other} && $this->{other}->{final})
    || $this->{noothermoves} || 0;
}

sub counter_move ($$) {
  my ($this, $my_move) = @_;
  my $list = $this->{sorted_results};
  return undef unless $list;
  foreach my $pv (@$list) {
    my $move = $pv->{pv}->[0];
    next unless $move eq $my_move;
    return $pv->{pv}->[1];
  }
  return undef;
}

sub sort_moves ($) {
  my ($this) = @_;
  $this->genmove_probability if ($this->move_count == 0);
  my $counter = $this->last_move && $this->{parent}->counter_move($this->last_move);
  if ($counter && $counter ne "resign" && !defined $this->{moves}->{$counter}) {
    if (defined $this->log_prob($counter.'+')) {
      # todo: nopromote moves of pawn, bishop, rook
      my $msg = sprintf "pv has nopromote move %s (%s) at %s\n",
	$counter, $this->csamove($counter), $this->csahistory_from_root;
      $logger->enqueue($msg) if $logger;
      print STDERR $msg;
      $this->{moves}->{$counter} = $this->log_prob($counter.'+');
    } else {
      warn "counter $counter invalid at ".$this->csahistory_from_root
	if ($counter ne "pass");
      $counter = undef;
    }
  }
  my @ret = sort { $this->log_prob($a) <=> $this->log_prob($b) } keys %{$this->{moves}};
  @ret = ($counter, grep { $_ ne $counter } @ret) if $counter;
  return @ret;
}

sub update_leaf ($$$) {
  my ($this, $ret, $request) = @_;
  my $now = StopWatch::current_time;
  ### handle checkmate result (for probe/searchinfinite, possibly full root search)
  if ($ret->{line} =~ /^checkmate\s+(.*)/) {
    my $checkmate = $1;
    $this->{checkmate} = $checkmate;
    $this->{checkmate_pid} = $ret->{pid};
    push(@{$this->{probing_done}}, $ret->{id})
      if ($this->{probing});
    return;
  }
  ### full root search
  if (! $this->{parent} && (!$ret->{ignores} || $this->{full_root_search})) {
    if ($ret->{line} =~ /^bestmove\s+(.*)/) {
      $this->{final} = $1;
      return $ret->{line};
    }
    if ($ret->{line} =~ /^info.*score\s+cp\s+(-?[0-9]+).*pv\s+([0-9A-z+* ]+)/) {
      my ($value, $pv) = ($1*$this->{sign}, $2);
      $this->{leaf} = {value=> $value, pv=>[split(/\s+/,$pv)], nodes=>$this->{nodes},
		       update=>$now};
      $this->{full_root_search_last_score} = $ret->{line};
    }
    if ($ret->{line} =~ /^info\s+/) {
      return $ret->{line};
    }
    return undef;
  }
  ### others
  die unless $ret->{line};
  if ($ret->{line} =~ /^info.*nodes\s+([0-9]+)/) {
    if ($ret->{ignores}) {
      $this->{other_nodes} = $1;
    } else {
      $this->{nodes} = $1;
    }
  }
  if ($ret->{line} =~ /^info.*score\s+cp\s+(-?[0-9]+).*pv\s+([0-9A-z+* ]+)/) {
    my ($value, $pv) = ($1*$this->{sign}, $2);
    if ($ret->{ignores}) {
      $this->{other} = {value => $value, pv => [split(/\s+/,$pv)],nodes=>$this->{other_nodes},
			update=>$now, id=>$ret->{id}};
      delete $this->{other_maybe_forced};
      return $this->update($ret->{pid});
    } else {
      $this->{leaf} = {value=> $value, pv=>[split(/\s+/,$pv)], nodes=>$this->{nodes},
		       update=>$now, id=>$ret->{id}};
      delete $this->{maybe_forced};
      return $this->{parent}->update($ret->{pid});
    }
  }
  if ($ret->{line} =~ /^info.*loss by checkmate/) {
    if ($ret->{ignores}) {
      $this->{other} = {value => -$this->win_value, pv => ["resign"],
			nodes=>$this->{nodes} || 1, final=>1, update=>$now, id=>$ret->{id}};
      delete $this->{other_maybe_forced};
      return $this->update($ret->{pid});
    } else {
      $this->{leaf} = {value => -$this->win_value, pv => ["resign"],
		       nodes=>$this->{nodes} || 1, final=>1, update=>$now, id=>$ret->{id}};
      delete $this->{maybe_forced};
      return $this->{parent}->update($ret->{pid});
    }
  }
  # handle bestmove
  if ($ret->{line} =~ /^bestmove\s+(.*)/) {
    my $bestmove = $1;
    if ($ret->{ignores}) {
      if ($this->{other} && $this->{other}->{pv}->[0] eq $bestmove) {
	$this->{other}->{final} = 1;
	$this->{other}->{update} = $now;
      } else {
	my $value = undef;
	$value = 0 if $stop_pid >= $ret->{pid};
	if (!defined $value) {	# forced moves => seacrh extension
	  if (($this->{other_maybe_forced} || "N/A") eq $bestmove) {
	    $this->{noothermoves} = [$bestmove, [@{$ret->{ignores}}]];
	    delete $this->{other};
	    if ($this->child($bestmove) || $bestmove eq "resign") {
	      warn "id = $ret->{id} $bestmove";
	    } else {
	      my $child = $this->create_child($bestmove);
	      my $new_position = $child->{position};
	      if ($this->{probing}) {
		$child->{probing} = [$ret->{id}];
		$request->{$ret->{id}}->enqueue
		  ({pid=>$ret->{pid}, command=>"go_probe", position=>$new_position->to_s});
	      } else {
		$request->{$ret->{id}}->enqueue
		  ({pid=>$ret->{pid}, command=>"go", position=>$new_position->to_s});
	      }
	      my $msg = "extend $ret->{id} $bestmove (other)";
	      $msg .= " in probe" if $this->{probing};
	      print STDERR $msg."\n";
	      $logger->enqueue($msg."\n") if $logger;
	    }
	    return;
	  }
	  if ($bestmove eq "resign") {
	    $value = -$this->win_value;
	  } elsif ($bestmove eq "win") {
	    $value = $this->win_value;
	  } elsif ($this->{other} && defined $this->{other}->{value}
		   && $this->{other}->{value}*$this->{sign} <= -$value_checkmate_win) {
	    warn "treat bestmove $bestmove as resign";
	    $value = $this->{other}->{value};
	    $bestmove = "resign";
	  } else {
	    warn "bestmove changed without pv, id = $ret->{id} $bestmove";
	  }
	  die "$this->{other_maybe_forced} $bestmove, id = $ret->{id}"
	    if $this->{other_maybe_forced};
	}
	warn unless $bestmove;
	$this->{other} = {value=>$value, pv=>[$bestmove], nodes=>$this->{nodes} || 1,
			  final=>1, update=>$now, id=>$ret->{id}};
      }
      die unless $this->is_other_final;
      # end of "other" move (already returned if re-search is needed)
      push(@{$this->{probing_done}}, $ret->{id})
	if ($this->{probing});
      return $this->update($ret->{pid});
    } else {
      $this->{final} = $bestmove;
      $this->{leaf}->{update} = $now;
      if ($this->{leaf} && $this->{leaf}->{pv}->[0]
	  && $this->{leaf}->{pv}->[0] eq $bestmove) {
	;
      } else {
	my $value = undef;
	$value = 0 if $stop_pid >= $ret->{pid};
	if (!defined $value) {	# forced moves => seacrh extension
	  if (($this->{maybe_forced} || "N/A") eq $bestmove) {
	    $this->{noothermoves} = [$bestmove, []];
	    $this->{children} = {};
	    my $child = $this->create_child($bestmove);
	    my $new_position = $child->{position};
	    if ($this->{probing}) {
	      $child->{probing} = [$ret->{id}];
	      $request->{$ret->{id}}->enqueue
		({pid=>$ret->{pid}, command=>"go_probe", position=>$new_position->to_s});
	    } else {
	      $request->{$ret->{id}}->enqueue
		({pid=>$ret->{pid}, command=>"go", position=>$new_position->to_s});
	    }
	    delete $this->{leaf};
	    my $msg = "extend $ret->{id} $bestmove";
	    $msg .= " in probe" if $this->{probing};
	    print STDERR $msg."\n";
	    $logger->enqueue($msg."\n") if $logger;
	    return;
	  }
	  if ($bestmove eq "resign") {
	    $value = -$this->win_value;
	  } elsif ($bestmove eq "win") {
	    $value = $this->win_value;
	  } elsif ($this->{leaf} && defined $this->{leaf}->{value}
		   && $this->{leaf}->{value}*$this->{sign} <= -$value_checkmate_win) {
	    warn "treat bestmove $bestmove as resign";
	    $value = $this->{leaf}->{value};
	    $bestmove = "resign";
	  } else {
	    warn "bestmove changed without pv, id = $ret->{id} $bestmove pid $ret->{pid}";
	  }
	  die "$this->{maybe_forced} $bestmove, id = $ret->{id}"
	    if $this->{maybe_forced};
	}
	$this->{leaf} = {value=>$value, pv=>[$bestmove], nodes=>$this->{nodes} || 1,
			 final=>1, update=>$now};
      }
      die unless $this->is_final;
      push(@{$this->{parent}->{probing_done}}, $ret->{id})
	if ($this->{parent}->{probing});
      return $this->{parent}->update($ret->{pid});
    }
    # end of bestmove
  }
  if ($ret->{line} =~ /^info.*forced move at the root: ([A-Za-z0-9+*]+)/) {
    if ($ret->{ignores}) {
      delete $this->{other};
      $this->{other_maybe_forced} = $1;
    } else {
      delete $this->{leaf};
      $this->{maybe_forced} = $1;
    }
    return;
  }
  return undef if ($ret->{line} =~ /^info/ && $ret->{line} !~ /(string|score|pv)/);
  warn "ignored *** $ret->{line} *** id= $ret->{id} position = $ret->{position}";
  return undef;
}

# internal node
sub update ($$) {
  my ($this, $pid) = @_;
  warn unless defined $pid;
  my $now = StopWatch::current_time;
  my $list = [];
  my $all_final = $this->is_other_final
    || $this->child_count == $this->move_count;
  my $node_count = ($this->{other_nodes}||0);
  $node_count = $this->{old_other}->{nodes}
    if ($this->{old_other} && ($this->{old_other}->{nodes}||0) > $node_count);
  my $max_child_nodes = $this->{other_nodes};
  my $last_update = ($this->{other} && $this->{other}->{update}) || 0;
  my $num_last_results = $this->{sorted_results} ? (@{$this->{sorted_results}}+0) : 0;
  # count all children
  foreach my $move (keys %{$this->{children}}) {
    unless (defined $this->log_prob($move)) {
      warn "$move at ".$this->csahistory_from_root;
      next;
    }
    my $child = $this->child($move);
    $node_count += $child->{nodes};
    $max_child_nodes = $child->{nodes} if ($max_child_nodes < $child->{nodes});
    $all_final = ($all_final && $child->is_final
		  && ($stop_pid == $pid || ! $child->{probing}));
    my $cpv = $child->{leaf};
    if (!$cpv
	|| ($child->{sorted_results} # ignore unstable results just after split
	    && $child->{sorted_results}->[0]
	    && ($child->{nodes} >= $child->{leaf}->{nodes}*$pending_factor
		|| ($child->{sorted_results}->[0]->{final}
		    && ! $child->{sorted_results}->[0]->{probing})))) {
      $cpv = $child->{sorted_results}->[0];
    }
    next unless $cpv && $cpv->{pv};
    warn "$move pid $pid at ".$this->csahistory_from_root
      unless defined $cpv->{value};
    push(@$list, {value=>$cpv->{value}, pv=>[$move, @{$cpv->{pv}}], nodes=>$cpv->{nodes}, final=>$child->is_final, probing=>$child->{probing}, update=>$cpv->{update}});
    $last_update = $cpv->{update} if ($last_update < $cpv->{update});
  }
  if ($this->{other} && ! $this->{noothermoves}) {
    my $cpv = $this->{other};
    if ($this->{old_other}) {
      die "logic error: old_other must be created after completion of probing"
	if ($cpv->{final} && $this->{probing});
      $cpv = $this->{old_other}
	if (! $cpv->{final}
	    && (($this->{old_other}->{nodes} || 0)*$pending_factor
		> ($this->{other}->{nodes} || 0)));
    }
    warn "other at ".$this->csahistory_from_root unless defined $cpv->{value};
    my $move = $cpv->{pv}->[0];
    if (! defined $this->log_prob($move)
	&& defined $this->log_prob($move.'+')) {
      # todo: nopromote moves of pawn, bishop, rook
      my $msg = sprintf "nopromote move %s (%s) is returned at %s\n",
	$move, $this->csamove($move), $this->csahistory_from_root;
      $logger->enqueue($msg) if $logger;
      print STDERR $msg;
      $this->{moves}->{$move} = $this->log_prob($move.'+');
    }
    unless ($move eq "resign" || $move eq "win" || defined $this->log_prob($move)) {
      warn "$move at ".$this->csahistory_from_root." in ".join(',', keys %{$this->{moves}})." v.s. ".join(',', keys %{$this->{children}});
    } else {
      push(@$list, {value=>$cpv->{value}, pv=>[ @{$cpv->{pv}} ],
		    nodes=>($cpv->{nodes} || $this->{other_nodes}),
		    final=>$cpv->{final}, update=>$cpv->{update}});
    }
  }
  $list = [sort { my ($va, $vb) = (int($a->{value}*$this->{sign}), int($b->{value}*$this->{sign}));
		  ($va != $vb) ? $vb <=> $va : $b->{update} <=> $a->{update} } @$list];
  $this->{nodes} = $node_count;
  unless ($stop_pid >= $pid) {
    while (@$list + 0 && $list->[0]->{nodes}*$ignore_factor < $max_child_nodes
	   && $list->[0]->{value} *$this->{sign} < $value_checkmate_win/2
	   && ! $list->[0]->{final}) {
      my $removed = shift @$list;
      if ($now - $this->root->{start_time} > 1000) {
	my $msg = "IGNORED $removed->{pv}->[0]: $removed->{nodes} * $ignore_factor < $max_child_nodes at ".($this->csahistory_from_root||"root")."\n";
	$logger->enqueue($msg) if $logger && !$this->{parent};
	print STDERR "$msg" if !$this->{parent};
      }
    }
  }
  return undef unless @$list+0;
  if ($this->{parent}		# special treatment at the root
      && ! defined $this->{win_move}
      && ! $all_final && ! $this->{probing} && $list->[0]->{final}
      && $list->[0]->{value}*$this->{sign} >= $value_checkmate_win
      && ! $list->[0]->{probing}) {
    my $msg = sprintf "win found by %s at %s\n",
      $list->[0]->{pv}->[0], $this->csahistory_from_root;
    $logger->enqueue($msg) if $logger;
    print STDERR $msg;
    # better to stop siblings here
    $all_final = 1;
    $this->{win_move} = $list->[0]->{pv}->[0];
  }
  my ($update, $bestmove_become_final);
  if (! $this->{parent}) { # root
    $bestmove_become_final = $list->[0]->{final}
      && (!$this->{sorted_results} || ! $this->{sorted_results}->[0]->{final});
    $update = (! $this->{sorted_results}) || (($this->is_final&&1) != ($all_final&&1))
      || (! $this->{sorted_results}->[0]->{final} && $list->[0]->{final})
      || $this->{last_bestmove} ne ($list->[0]->{pv}->[0] || "nopv"); # move
    $update ||= $this->{sorted_results}->[0]->{value} != $list->[0]->{value} # eval
      if ($now > $this->{start_time}+800);
    $update ||= ((@$list+0) >= $this->child_count) && ($num_last_results < $this->child_count);
    if ($now > $this->{start_time}+10000 && ! $update) {
      $update ||= $this->{sorted_results}->[1]->{value} != $list->[1]->{value}
	if $this->{sorted_results}->[1];
    }
  }
  $this->{sorted_results} = $list;
  $this->{update} = $last_update;
  $this->{final} = $all_final;
  $this->{final} = $this->{sorted_results}->[0]->{pv}->[0]
    if ($all_final && $this->{sorted_results} && $this->{sorted_results}->[0]->{pv});

  if ($this->{probing} && $logger && ($show_interim_probe || $this->is_final)) {
    my $summary = sprintf "%s probe result at %s -- %s\n",
      $this->is_final ? "final" : "interim",
	$this->csahistory_from_root || "root",
	  $this->is_final ? $this->csamove($this->is_final) : "not yet";
    $summary .= $this->child_summary(300);
    $logger->enqueue($summary);
  }

  if ($this->{parent}) {
    return $this->{parent}->update($pid)
      if ($this->{final} || (@{$this->{sorted_results}}+0) >= $this->child_count); # todo other
  } else {
    # root
    if ($update) {
      my $elapsed = $now-$this->{start_time};
      if (! $this->{probing}) {
	my $summary = sprintf "\nroot update %.1fk %.2fs. nps %.1fk children %d\n", $node_count/1000.0,
	  $elapsed/1000.0, 1.0*$node_count/($now-$this->{start_time}),
	    $this->{sorted_results} ? @{$this->{sorted_results}}+0 : 0;
	$summary .= $this->child_summary(4, $stop_pid >= $pid);
	print STDERR $summary
	  if ($stop_pid < $pid
	      && ((@{$this->{sorted_results}}+0) >= $this->child_count # todo: other
		  || $this->is_final || $bestmove_become_final));
	$logger->enqueue($summary) if $logger;
      }
      my $bestpv = $list->[0];
      if ($this->is_final) {
	my $bestmove = $bestpv->{pv}->[0];
	if (! $this->{probing}
	    && $this->{almost_final_bestmove}
	    && $this->{almost_final_bestmove} ne $bestmove) {
	  my $msg = sprintf "bestmove changed via stop from %s to %s",
	    $this->{almost_final_bestmove}, $bestmove;
	  warn $msg;
	  $logger->enqueue($msg."\n") if $logger;
	  $bestmove = $this->{almost_final_bestmove};
	}
	elsif ($stop_pid >= $pid
	       && $this->{last_bestmove}
	       && $this->{last_bestmove} ne $bestmove
	       && $this->{last_bestmove} ne "resign") {
	  my $msg = sprintf
	    "stick to last bestmove %s rather than %s after stop",
	      $this->{last_bestmove}, $bestmove;
	  warn $msg;
	  $logger->enqueue($msg."\n") if $logger;
	  $bestmove = $this->{last_bestmove};
	}
	my $bestmove_msg = "bestmove ".$bestmove;
	if (! $this->{probing}) {
	  my $comment = "";
	  $comment = $this->{last_summary}->{$bestmove}
	    if $this->{last_summary} && $this->{last_summary}->{$bestmove};
	  $logger->enqueue("returning ".$bestmove_msg.$comment."\n")
	    if $logger;
	  print STDERR "returning ".$bestmove_msg.$comment."\n";
	}
	return $bestmove_msg;
      }
      my $is_almost_final = $bestpv->{final} && ! $this->{probing}
	&& $bestpv->{value} * $this->{sign} >= $value_checkmate_win;
      if ($bestmove_become_final || $is_almost_final
	  || (((@{$this->{sorted_results}}+0) >= $this->child_count # todo: other
	       || !$this->{probing})
	      && ($this->{last_bestmove} ne $bestpv->{pv}->[0]
		  || $this->{last_update}+1000 < $now))) {
	$this->{last_update} = $now;
	$this->{last_bestmove} = $bestpv->{pv}->[0]
	  if ($stop_pid < $pid || ! $this->{last_bestmove}
	      || $this->{last_bestmove} eq "resign");
	return undef
	  if ($is_almost_final && $this->{almost_final_bestmove});
	if ($is_almost_final) {
	  $almost_final_pid = $pid;
	  $this->{almost_final_bestmove} = $bestpv->{pv}->[0];
	  my $msg = "almost final situation at root by ".$this->{almost_final_bestmove}."\n";
	  $logger->enqueue($msg) if $logger;
	  print STDERR $msg;
	}
	return $this->usi_info_pv;
      }
    }
  }
  return undef;
}

sub usi_info_pv ($@) {
  my ($this, $bestmove) = @_;
  return undef if $this->{full_root_search};
  my $list = $this->{sorted_results};
  my $bestpv = $list->[0];
  $bestpv = $this->{leaf} unless $bestpv;
  my $elapsed = StopWatch::current_time - $this->{start_time};
  unless ($bestpv->{pv}) {
    warn "pv not found";
    $bestpv->{pv} = ["resign"];
  } elsif ($bestmove && $bestpv->{pv}->[0] ne $bestmove) {
    for (my $i=1; $i<@{$list}+0; ++$i) {
      if ($list->[$i]->{pv}
	  && $list->[$i]->{pv}->[0] eq $bestmove) {
	if (abs($list->[$i]->{value} - $bestpv->{value}) >= 1) {
	  my $msg =
	    sprintf "different value for %s (%d) %d and %s (0) %d",
	      $bestmove, $i, $list->[$i]->{value},
		$bestpv->{pv}->[0], $bestpv->{value};
	  warn $msg;
	  $logger->enqueue($msg) if $logger;
	}
	$bestpv = $list->[$i];
	last;
      }
    }
  }
  sprintf "info time %d seldepth %d score cp %d nodes %d pv %s", $elapsed,
	  @{$bestpv->{pv}}+0, $bestpv->{value} * $this->{sign}, $this->{nodes},
	    join(' ', @{$bestpv->{pv}});
}

sub assign_by_probe ($) {
  my ($this) = @_;
  die unless $this->{probing};
  my @slaves = @{$this->{probing}};
  die if @slaves+0 == 0;
  return [["other", [$slaves[0]]]] if @slaves+0 == 1;
  my $list = [];
  my $rank = 1;
  $this->{rank} = {};
  foreach my $pv (@{$this->{sorted_results}}) {
    delete $pv->{final};
    my $move = $pv->{pv}->[0];
    next if ($move eq "resign");
    my $child = $this->child($move);
    if ($child) {
      delete $child->{final};
    } else {
      if (! defined $this->{moves}->{$move}) {
	warn $move;
	next;
      }
      $this->create_child($move);
      if ($this->{other}->{pv} && $this->{other}->{pv}->[0] eq $move) {
	$this->child($move)->{leaf} = { %{$this->{other}} };
	$this->child($move)->{leaf}->{pv} = [ @{$this->{other}->{pv}} ];
	shift @{$this->child($move)->{leaf}->{pv}};
      }
    }
    $this->{rank}->{$move} = $rank++;
    push(@$list, $pv);
  }
  delete $this->{probing};
  delete $this->{final};
  delete $this->{leaf}->{final} if $this->{leaf};
  $this->{old_other} = {};
  $this->{old_other} = $this->{other} if $this->{other};
  delete $this->{old_other}->{final} if $this->{old_other};
  delete $this->{other};
  my $test_width = $leaf_width;
  $test_width = $root_width if ($root_width > $test_width && ! $this->{parent});
  if ($#slaves < $test_width && @$list + 0 > $#slaves) {
    my $ret = [];
    foreach my $i (0..$#slaves) {
      my $move = ($i < $#slaves) ? $list->[$i]->{pv}->[0] : "other";
      push(@$ret, [ $move, [$slaves[$i]] ]);
    }
    return $ret;
  }
  # todo bestmove is "win" or checkmate
  my $width = $this->{parent} ? 2 : $root_width;
  my $top = $list->[0]->{value};
  my $near = 1;
  my $near_width = $this->{parent} ? $split_near_width : $split_near_width_root;
  foreach my $i (1..((@$list)-1)) {
    my $e = $list->[$i];
    last if (!$e || abs($e->{value} - $top) > $near_width);
    ++$near;
  }
  $near = (@slaves+0)/4 if $near > (@slaves+0)/4;
  $width = $near if $near > $width;
  $width = $#slaves if $width > $#slaves;
  my @moves = map { $list->[$_] && $list->[$_]->{pv}->[0] } (0..$width-1);
  foreach my $i (0..$#moves) {
    if (! $moves[$i]) {
      $#moves = $i-1;
      last;
    }
    $this->child($moves[$i])->genmove_probability
      if ($this->child($moves[$i])->move_count == 0);
    return [[$moves[$i], [$slaves[0]]], ["other", [$slaves[1]]]] # almost win
      if ($this->child($moves[$i])->move_count == 0);
  }
  foreach my $j (0..$#moves) {
    my $i = $#moves - $j;	# $#moves..0
    next if $i && $list->[$i]->{value}*$this->{sign} <= -$value_checkmate_win; # losing
    my @assign = map { [] } (0..$i);
    foreach my $i (1..$#slaves) {
      push(@{$assign[($i-1) % $width]}, $slaves[$i]);
    }
    my $ret = [ map { [$moves[$_], $assign[$_]] } (0..$i) ];
    push(@$ret, ["other", [$slaves[0]]]);
    return $ret;
  }
  die "assign failed";
}

sub hide_children ($) {
  my ($this, @survived) = @_;
  foreach my $move (keys %{$this->{children}}) {
    next if grep { $_ eq $move } @survived;
    $this->{old_children}->{$move} = $this->{children}->{$move};
    delete $this->{children}->{$move};
  }
}

sub has_checkmate_win ($) {
  my ($this) = @_;
  my $move = $this->{checkmate};
  return $move && ($move ne "nomate") && ($move ne "timeout")
    && ($move ne "trying");
}

sub update_checkmate_win ($) {
  my ($this) = @_;
  return unless $this->{checkmate};
  my $move = $this->{checkmate};
  my $msg = sprintf "checkmate win %s (%s) at %s\n",
    $move, $this->csamove($move), $this->csahistory_from_root;
  $logger->enqueue($msg) if $logger;
  print STDERR $msg;

  delete $this->{sorted_results};
  delete $this->{probing};
  $this->{old_other} = {};
  if ($this->{other}) {
    $this->{old_other} = $this->{other};
    delete $this->{old_other}->{final};
    delete $this->{other};
  }
  $this->hide_children;
  $this->{leaf} = {value=>$this->win_value,
		   pv=>[$move], nodes=>$this->{nodes} || 1,
		   final=>$move, update=>StopWatch::current_time};
  $this->{final} = $move;
  return "bestmove $move"
    unless $this->{parent};
  return $this->{parent}->update($this->{checkmate_pid});
}


sub child_summary ($$@) {
  my ($this, $maximum, $is_stopping) = @_;
  my $list = $this->{sorted_results};
  my $summary = "";
  return $summary unless $list;
  $this->{last_summary} = {} unless $this->{last_summary};
  foreach my $i (0..@$list-1) {
    my $pv = $list->[$i];
    my $move = $pv->{pv}->[0];
    unless ($move) {
      warn sprintf "move undefined at %d in %s at %s",
	$i, join(' ', map { ($pv->{pv}->[0]||"-").":".($pv->{value}||'?') } @$list),
	  $this->csahistory_from_root;
      next;
    }
    my $is_other = ! $this->child($move);
    my $pv_with_rank = $this->csamoves_with_rank(@{$pv->{pv}});
    my $line = sprintf "%10.1f (%2d) %s (%.1fk) %s %s", $pv->{value}/10.0,
      @{$pv->{pv}}+0, $pv_with_rank,
	($is_other ? $this->{other_nodes} : $this->child($move)->{nodes}) / 1000.0,
	  $is_other ? '*' : "",
	    ($is_other ? $this->is_other_final : $this->child($move)->is_final)
	      ? ($this->{probing} ? "probed" : "final") : "";
    $summary .= $line."\n";
    $this->{last_summary}->{$move} = $line unless ($is_stopping);
    last if ($i >= $maximum);
  }
  return $summary;
}

# END
1;
package TreeSearch;
use strict;
use IO::Handle;
use FileHandle;
use File::Path;
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
package main;
#!/usr/bin/perl -w
use strict;
use threads;
use threads::shared;
use Thread::Queue;
use Getopt::Std;
use FileHandle;
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
