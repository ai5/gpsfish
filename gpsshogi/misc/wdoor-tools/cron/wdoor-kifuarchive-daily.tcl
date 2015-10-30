#!/usr/bin/tclsh

proc log {msg} {
  set str "$::argv0: $msg" 
  exec logger $str
  puts $str
}

# Today by default
set day [expr {[clock seconds]}]
set dir [clock format $day -format "%Y/%m/%d"]
set ymd [clock format $day -format "%Y%m%d"]

if {$::argc >= 3} {
  # Sepcific day
  set year  [lindex $::argv 0]
  set month [lindex $::argv 1]
  set day   [lindex $::argv 2]
  set dir   [format "%d/%02d/%02d" $year $month $day]
  set ymd   [format "%d%02d%02d" $year $month $day]
} elseif {$::argc == 1 && [lindex $::argv 0] == "yesterday"} {
  # Yesterday
  set day [expr {[clock seconds] - 3600*24}]
  set dir [clock format $day -format "%Y/%m/%d"]
  set ymd [clock format $day -format "%Y%m%d"]
}

set rootdir $::env(HOME)/kifuarchive-daily
set basedir $rootdir/$ymd

log "Archiving /var/www/shogi/x/$dir/ to $basedir/..."

file mkdir $basedir
cd $rootdir

# Copy Kifu files
foreach f [glob /var/www/shogi/x/$dir/wdoor+floodgate*.csa] {
  file copy -- $f $basedir 
}

# Archive the directory
exec 7z a wdoor$ymd.7z $ymd
file copy -force wdoor$ymd.7z /var/www/shogi/x/daily
file delete -force $basedir
