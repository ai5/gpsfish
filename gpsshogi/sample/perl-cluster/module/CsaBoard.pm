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
