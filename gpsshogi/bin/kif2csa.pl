#!/usr/local/bin/perl
if($#ARGV!=0){
  die "Usage: kif2csa.pl file";
}
%csakoma=('��','FU','��','KY','��','KE','��','GI','��','KI','��','KA',
	  '��','HI','��','OU','��','UM','ζ','RY','����','NY','����','NK',
	  '����','NG','��','TO');
%promotekoma=('��','��','��','����','��','����','��','����','��','��',
	      '��','ζ');
%zen2han=('��','1','��','2','��','3','��','4','��','5','��',6,'��',7,
	  '��','8','��','9','��','1','��','2','��','3','��','4','��','5',
          'ϻ','6','��','7','Ȭ','8','��','9');
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
  if($_ =~/^���䡧/){
    if($_ !~ /ʿ��/){
      die "ʿ��ʳ��μ���ϰ����ޤ���";
    }
  }
  if($_ =~ /^���/){
    $mode=1;
  }
  elsif($mode==1){
    ($n,$move,$t)=split;
    if($move =~ /^��λ$/){
      print "%CHUDAN\n";
      last;
    }
    elsif($move =~ /^(....)(.*)��$/){
      $frompos='00'; 
      $koma=$2;
      $topos=$1;
    }
    elsif($move =~ /^(....)(.*)��\(([0-9][0-9])\)/){
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
    if($topos =~ /^Ʊ��/){
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
