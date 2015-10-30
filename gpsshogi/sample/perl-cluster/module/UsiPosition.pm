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
