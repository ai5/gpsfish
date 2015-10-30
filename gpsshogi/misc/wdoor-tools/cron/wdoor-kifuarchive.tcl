#!/usr/bin/tclsh

proc log {msg} {
  set str "wdoor-kifuarchive.tcl: $msg" 
  exec logger $str
  puts $str
}

set yesterday [expr {[clock seconds] - 3600*24}]
set dir       [clock format $yesterday -format "%Y/%m/%d"]
set year      [clock format $yesterday -format "%Y"]
set rootdir   $::env(HOME)/kifuarchive
set basedir   $rootdir/$year

log "Archiving /var/www/shogi/x/$dir to $basedir..."

file mkdir $basedir
cd $rootdir

# Copy new Kifu files
foreach f [glob /var/www/shogi/x/$dir/wdoor+floodgate*.csa] {
  exec cp -af $f $year/
}

# Archive the directory
exec 7z a wdoor$year.7z $year
file copy -force wdoor$year.7z /var/www/shogi/x/
