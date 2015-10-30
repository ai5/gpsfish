package TreeNode;
use strict;
use IO::Handle;
use module::UsiPosition;
use module::UsiEngine;
use module::StopWatch;
use module::CsaBoard;
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
