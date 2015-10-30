package CsaFileSession;
use strict;
use IO::Handle;
use Time::HiRes;
use Time::Piece;
use Sys::Hostname;

BEGIN {
  $| = 1;
}

sub new ($$$) {
  my ($pkg, $path, $initial) = @_;
  my $this = { path => $path, cur => $initial, };

  bless $this;
  return $this;
}

sub host() {
  my $n = hostname();
  $n =~ s/\..*//;
  return sprintf("%8s", $n);
}

sub write ($$) {
  my ($this, $message) = @_;
  my ($path, $id) = ($this->{path}, sprintf("%04d", $this->{cur}));
  my $now = Time::Piece::localtime;
  chomp $message;
  open (OUT, "> $path/$id.part") || die "open $path/$id $!";
  print OUT $message."\n";
  close OUT;
  print STDERR $now->datetime." ".host()." W $id $message\n";
  rename("$path/$id.part", "$path/$id.txt") || die "mv $id $!";
  # print STDERR "finish $id\n";
  $this->{cur} += 1;
}

sub try_read ($) {
  # todo: inotify
  my ($this) = @_;
  my ($path, $id) = ($this->{path}, sprintf("%04d", $this->{cur}));
  if (open (IN, "$path/$id.txt")) {
    my $now = Time::Piece::localtime;
    my $message = <IN>;
    chomp $message;
    close IN;
    print STDERR $now->datetime." ".host()." R $id $message\n";
    $this->{cur} += 1;
    return $message;
  }
  return undef;
}

sub read ($) {
  my ($this) = @_;
  while (1) {
    if (my $message = $this->try_read()) {
      return $message;
    } else {
      Time::HiRes::usleep(50000);
    }
  }
}

sub id ($) {
  my ($this) = @_;
  return $this->{cur};
}

# END
return 1;
