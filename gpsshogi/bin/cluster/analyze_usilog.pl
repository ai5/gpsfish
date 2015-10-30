#!/usr/bin/perl
use strict;
# my $num_workers=314;
my $num_workers;
my @alllog=[];
my $debug=0;
my $debug1=0;
my %used=();
sub moves($)
{
    my ($s)=@_;
    my @ret=split(/-/,$s);
    return @ret;
}

sub translate_log ($)
{
    my ($i)=@_;
    my $r=open F,"$i.txt";
    return 0 if !$r;
    my $position='';
    my $cmd='';
    my $multipv;
    my $byoyomi;
    my $ignore_moves;
    my %info=();
    my $line=0;
    my $forced_move;
    my $info_depth;
    my @ret=();
    while(<F>){
	$line++;
	if ( $_=~ />>> position sfen \S*lnsgkgsnl\/1r5b1\/ppppppppp\/9\/9\/9\/PPPPPPPPP\/1B5R1\/LNSGKGSNL b \- 1 moves(.*)$/ ){
	    if($position ne ''){
		print "$i position and position at line $line\n";
	    }
	    $position=$1;
	    $position =~ s/^ //;
	    $position =~ s/ /-/g;
	    $ignore_moves='';
	}
	elsif ($_ =~ />>> go book/){
	    $cmd='book';
	}
	elsif ($_ =~ /<<< bestmove (\S*)/){
	    if($cmd eq 'book'){
		push @ret,['book',$position,$1];
	    }
	    elsif($cmd eq 'go'){
		my $inf=($forced_move ? 'forced' : $info{$1});
#		die "$i Line $line: undefined move $1" if !defined $inf;
		if(!defined $inf){
		    $inf='undefined';
		}
		if($multipv>0){
		    push @ret,['multipv',$position,$1,$inf];
		}
		elsif($ignore_moves ne ''){
		    push @ret,['ignore',$position,$1,$byoyomi,$ignore_moves,$inf];
		}
		else{
		    my $inf=($forced_move ? 'forced' : $info{$1});
		    if(!defined $inf){
			$inf='unknown';
		    }
#		    die "$i Line $line: undefined move $1" if !defined $inf;
		    push @ret,['go',$position,$1,$byoyomi,$inf];
		}
	    }
	    $position='';
	}
	elsif ($_ =~  />>> genmove_probability/){
	    $cmd='genmove_probability';
	}
	elsif ($_ =~ /<<< genmove_probability(.*)/){
	    my $genmove_probability=$1;
	    $genmove_probability =~ s/^ //;
	    $genmove_probability =~ s/ /-/g;
	    push @ret,['genmove_probability',$position,$genmove_probability];
	    $position='';
	}
	elsif ($_ =~  />>> genmove/){
	    $cmd='genmove';
	}
	elsif ($_ =~ /<<< genmove(.*)/){
	    die "genmove not after genmove" if $cmd ne 'genmove';
	    my $genmove=$1;
	    $genmove =~ s/^ //;
	    $genmove =~ s/ /-/g;
	    push @ret,['genmove',$position,$genmove];
	    $position='';
	}
	elsif ($_ =~ />>> setoption name MultiPVWidth value (\d*)/){
	    $multipv=$1;
	}
	elsif ($_ =~ />>> ignore_moves (.*)$/){
	    $ignore_moves=$1;
	    $ignore_moves=~ s/ /-/g;
	}
	elsif ($_ =~ />>> go byoyomi (\d*) byoyomi_extension /){
	    $cmd='go';
	    $byoyomi=$1;
	    $forced_move=0;
	    $info_depth=0;
	}
	elsif ($_ =~ />>> go mate/){
	    $cmd='mate';
	}
	elsif ($_ =~ /<<< checkmate (\S*)/){
	    push @ret,['checkmate',$position,$1];
	    $position='';
	}
	elsif ($_ =~ /update_info\((\S*),(\d*),(\-?\d*)\)/){
	    $info{$1}=($2,$3);
	}
	elsif ($_ =~ /info depth (\d*)/){
	    $info_depth=$1;
	}
	elsif ($_ =~ /info string forced move at the root:/){
	    if($info_depth>=7){
		$forced_move=1;
	    }
	}
	elsif ($_ =~ />>> query searchtime/){
	    $cmd='searchtime';
	}
	elsif ($_ =~ /<<< answer searchtime (\S*) /){
	    push @ret,['searchtime',$position,$1];
	    $position='';
	}
    }
    close F;
    return \@ret;
}

sub getline($){
    my ($i)=@_;
    my @log=@{$alllog[$i]};
    if($#log<0){ return ('end','');}
    my @line=@{shift @{$alllog[$i]}};
    if($#line<0){ return ('end','');}
    if($debug){
	print STDERR "$i [";
	foreach my $f (@line){
	    print STDERR $f.",";
	}
	print STDERR "]\n";
    }
    return @line;
}
sub peekline($$){
    my ($i,$n)=@_;
    my @log=@{$alllog[$i]};
    if($#log<0){ return ('end','');}
    my @line=@{$log[$n]};
    if($#line<0){ return ('end','');}
    if($debug){
	print STDERR "$i [";
	foreach my $f (@line){
	    print STDERR $f.",";
	}
	print STDERR "]\n";
    }
    return @line;
}

sub showline($){
    my ($i)=@_;
    my @line=peekline($i,0);
    print "$i : [";
    foreach my $f (@line){
	print $f.",";
    }
    print "]\n";
}


sub find_checkmove($$$){
    my ($posstr,$n,$f)=@_;
    for(my $i=$f+1;$i<$num_workers;$i++){
	next if($used{$i});
	my @line=peekline($i,$n);
	if($line[0] eq 'checkmate' &&
	   $posstr eq $line[1]){
	    for(my $j=$i+1;$j<$num_workers;$j++){
		my @line1=peekline($j,$n);
		if($line1[0] eq 'checkmate' &&
		   $posstr eq $line1[1]){
		    print "$i : @line\n";
		    print "$j : @line1\n";
		    print "multiple checkmate $i and $j\n";
		}
	    }
	    return $i;
	}
    }
    return -1;
}

sub find_search($$$){
    my ($posstr,$n,$f)=@_;
    for(my $i=$f+1;$i<$num_workers;$i++){
	next if($used{$i});
	my @line=peekline($i,$n);
	if($debug1){
	    print STDERR "$i : @line\n";
	}
	if(($line[0] eq 'multipv' || $line[0] eq 'go' || $line[0] eq 'genmove_probability') &&
	   $posstr eq $line[1]){
	    for(my $j=$i+1;$j<$num_workers;$j++){
		my @line1=peekline($j,$n);
		if(($line1[0] eq 'multipv' || $line1[0] eq 'go' || $line1[0] eq 'genmove_probability') &&
		   $posstr eq $line1[1]){
		    print "$i @line\n";
		    print "$j @line1\n";
		    print "multiple search $i and $j\n";
		}
	    }
	    return $i;
	}
    }
    print "search $posstr is not found\n";
    return -1;
}

sub search($$){
  my ($i,$prefix)=@_;
  my ($cmd,$posstr,@rest)=peekline($i,0);
  my @position=moves($posstr);
  if($cmd eq 'multipv' || $cmd eq 'genmove_probability'){
      getline($i);
      if($cmd eq 'multipv'){
	  my $j=find_checkmove($posstr,0,$i);
	  if($j<0){
	      $j=find_checkmove($posstr,1,$i);
	      if($j<0){
		  die "checkmove is not found" if !(0<=$j && $j<$num_workers);
	      }
	      getline($j);
	  }
	  my ($cmd1,$posstr1,@rest1)=getline($j);
	  print "$prefix multipv=($rest[0],$rest[1],$i),checkmate=($rest1[0],$j)\n";
	  if($rest1[0] ne 'nomate' && $rest1[0] ne 'timeout'){
	      return;
	  }
	  if($prefix eq '' && $rest[1] eq 'forced'){
	      my ($cmd1,$posstr1,$best_move1,$inf1)=getline($i);
	      die "not go after forced $cmd1" if $cmd1 ne 'multipv';
	      print "$prefix toplevel forced move\n";
	      return;
	  }
	  if($rest[1] eq 'forced'){
	      ($cmd,$posstr,@rest)=getline($i);
#	      die "not go after forced $cmd1" if $cmd ne 'multipv';
	      print "$prefix multipv extend\n";
	      search($i,$prefix."-($rest[0])");
	      return;
	  }
	  elsif($rest[0] eq 'resign'){
	      my ($cmd1,$posstr1,$best_move1,$byoyomi1,$inf1)=peekline($i,0);
	      if($cmd1 ne 'go' || $posstr1 ne $posstr){
		  print "\n$i $prefix not go after resign $cmd1\n";
		  return;
	      }
	      getline($i);
	      print $prefix." $best_move1 $inf1($i,$byoyomi1)\n";    
	      return;
	  }
      }
      else{
	  print "$prefix genmove_probability=$i\n";
      }
      ($cmd,$posstr,@rest)=peekline($i,0);
      if($cmd eq 'ignore'){
	  $used{$i}=1;
	  getline($i);
	  my ($best_move,$byoyomi,$ignore_str,$inf)=@rest;
	  print $prefix."-($best_move) $inf($i,$byoyomi)\n";    
	  if($inf eq 'forced'){
	      my ($cmd1,$posstr1,$best_move1,$byoyomi1,$inf1)=getline($i);
	      die "not go after forced $cmd1" if $cmd1 ne 'go';
      	      print $prefix."-$best_move $best_move1 $inf1($i,$byoyomi1)\n";    
	  }
	  my @ignore_moves=split(/-/,$ignore_str);
	  foreach my $m (@ignore_moves){
	      my $j=find_search($posstr."-".$m,0,$i);
	      if($j<0){
		  $j=find_search($posstr."-".$m,1,$i);
		  if($j<0){
		      for(my $k=0;$k<$num_workers;$k++){
			  showline($k);
		      }
		      die "search is not found" if !(0<=$j && $j<$num_workers);
		  }
		  getline($j);
	      }
	      search($j,$prefix."-".$m);
	  }
      }
      else{
	  print "not a ignore $cmd,$posstr,@rest\n";
	  return;
      }
  }
  elsif($cmd eq 'go'){
      $used{$i}=1;
      getline($i);
      my ($best_move,$byoyomi,$inf)=@rest;
      print $prefix." $best_move $inf($i,$byoyomi)\n";    
      if($inf eq 'forced'){
	  my ($cmd1,$posstr1,$best_move1,$byoyomi1,$inf1)=getline($i);
	  die "not go after forced $cmd1" if $cmd1 ne 'go';
	  print $prefix."-$best_move $best_move1 $inf1($i,$byoyomi1)\n";    
      }
  }
  else{
#      print STDERR "not a multipv ".@line;
      return;
  }
  
}
for(my $i=0;;$i++){
    my $r=translate_log($i);
    if(!$r){
	$num_workers=$i;
	last;
    }
    $alllog[$i]=$r;
}
my $sep="----------------\n";
# start 0 parse
my @log0=@{$alllog[0]};
my $is_sente;
while($#log0>=0){
  my @line=getline(0);
  my $cmd=$line[0];
  my @position=moves($line[1]);
  print "TOP: $cmd\n";
  if($cmd eq 'book'){
      $is_sente=( $line[1] eq '' ? 1 : 0) if !defined $is_sente;
      if($line[2] ne 'pass'){
	  print $sep."n_moves=".($#position+2)." book $line[2]\n";
      }
  }
  elsif($cmd eq 'genmove'){
      print $sep."n_moves=".($#position+2)." ponder ".$position[$#position]."\n";
      my @genmoves=moves($line[2]);
#      if($#genmoves > 0){
	  %used=();
	  search(0,'');
#      }
  }
  elsif($cmd eq 'searchtime'){
      next;
  }
  elsif($cmd eq 'ignore'){ # ponder
      next;
  }
  elsif($cmd eq 'go'){ # ponder best move='resign'
      next;
  }
  elsif($cmd eq 'end'){
      last;
  }
  else{
      die "Unexpected line @line in top level";
  }
#  print "[";
#  foreach my $f (@line){
#      print $f.",";
#  }
#  print "]\n";
}
