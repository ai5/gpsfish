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
