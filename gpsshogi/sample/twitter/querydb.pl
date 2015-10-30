#!/usr/bin/perl -w
use strict;
use NKF;
my ($kif, $moves) = @ARGV;

my $query = "../recorddb/query";
print STDERR "$query -d ../recorddb/all.tch -m $moves $kif\n";
open (ALL, "$query -d ../recorddb/all.tch -m $moves $kif|") || die $!;
my ($black_win, $black_loss) = split(/\s+/, <ALL>);
close ALL;

exit 0
  if ($black_win == 0 && $black_loss == 0);

if ($black_win + $black_loss > 20) {
  print "¿�����жɤ˸���Ƥ�����̡� \n";
  print "��Ψ����꤬�㴳ͭ������ \n"
    if ($black_win*1.0 / ($black_win + $black_loss) > 0.55);
}
elsif ($black_win + $black_loss > 1) {
  print "����Τ�����̡� \n";
}

open (GPS, "$query -d ../recorddb/floodgate.tch -m $moves $kif|") || die $!;
my ($black_com_win, $black_com_loss, $black_gps_win, $black_gps_loss) = split(/\s+/, <GPS>);
close GPS;

exit 0
  if ($black_com_win == 0 && $black_com_loss == 0);
print "GPS������Bonanza���жɤǤϡ�";
printf "��꤬GPS����������%d��%d�ԡ�", $black_gps_win, $black_gps_loss
  if ($black_gps_win + $black_gps_loss);
printf "��꤬Bonanza������%d��%d�ԡ�", ($black_com_win - $black_gps_win), ($black_com_loss - $black_gps_loss)
  if (($black_com_win - $black_gps_win) + ($black_com_loss - $black_gps_loss));
print " \n";
