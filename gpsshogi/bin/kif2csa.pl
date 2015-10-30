#!/usr/local/bin/perl
if($#ARGV!=0){
  die "Usage: kif2csa.pl file";
}
%csakoma=(' ‚','FU','π·','KY','∑À','KE','∂‰','GI','∂‚','KI','≥—','KA',
	  '»Ù','HI','∂Ã','OU','«œ','UM','Œ∂','RY','¿Æπ·','NY','¿Æ∑À','NK',
	  '¿Æ∂‰','NG','§»','TO');
%promotekoma=(' ‚','§»','π·','¿Æπ·','∑À','¿Æ∑À','∂‰','¿Æ∂‰','≥—','«œ',
	      '»Ù','Œ∂');
%zen2han=('£±','1','£≤','2','£≥','3','£¥','4','£µ','5','£∂',6,'£∑',7,
	  '£∏','8','£π','9','∞Ï','1','∆Û','2','ª∞','3','ªÕ','4','∏ﬁ','5',
          'œª','6','º∑','7','»¨','8','∂Â','9');
open(IN,"nkf -e $ARGV[0] |");
$mode=0;
$lastpos=0;
print '
N+COMPUTER
N-RS232C
P1-KY-KE-GI-KI-OU-KI-GI-KE-KY
P2 * -HI *  *  *  *  * -KA * 
P3-FU-FU-FU-FU-FU-FU-FU-FU-FU
P4 *  *  *  *  *  *  *  *  * 
P5 *  *  *  *  *  *  *  *  * 
P6 *  *  *  *  *  *  *  *  * 
P7+FU+FU+FU+FU+FU+FU+FU+FU+FU
P8 * +KA *  *  *  *  * +HI * 
P9+KY+KE+GI+KI+OU+KI+GI+KE+KY
+
';
while(<IN>){
  chomp;
  s///;
  if($_ =~/^ºÍπÁ≥‰°ß/){
    if($_ !~ / øºÍ/){
      die " øºÍ∞ ≥∞§ŒºÍπÁ≥‰§œ∞∑§®§ﬁ§ª§Û";
    }
  }
  if($_ =~ /^ºÍøÙ/){
    $mode=1;
  }
  elsif($mode==1){
    ($n,$move,$t)=split;
    if($move =~ /^≈ÍŒª$/){
      print "%CHUDAN\n";
      last;
    }
    elsif($move =~ /^(....)(.*)¬«$/){
      $frompos='00'; 
      $koma=$2;
      $topos=$1;
    }
    elsif($move =~ /^(....)(.*)¿Æ\(([0-9][0-9])\)/){
      $frompos=$3; 
      $koma=$promotekoma{$2};
      $topos=$1;
    }
    elsif($move =~ /^(....)(.*)\(([0-9][0-9])\)/){
      $frompos=$3; 
      $koma=$2;
      $topos=$1;
    }
    else{
      print "illegal";
    }
    if($topos =~ /^∆±°°/){
      $topos=$lastpos;
    }
    elsif($topos =~ /^(..)(..)$/){
      $topos= $zen2han{$1}.$zen2han{$2};
    }
    if(($n % 2)==1){ print '+'; }
    else {print '-';}
    print "$frompos$topos$csakoma{$koma}\nT1\n";
    $lastpos=$topos;
  }
}
close(IN);
