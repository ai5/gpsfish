#!/usr/bin/tclsh
package require cmdline
package require Thread

set options {
  {host.arg     "macpro"     "server host name to connect to"}
  {port.arg     4119         "server port number to connect to"}
  {slaves.arg   1            "number of slaves to run"}
}

set usage ": slave_mate.tcl \[options] \noptions:"

array set params [::cmdline::getoptions argv $options $usage]

set cmd "./gpsusi --io-stream tcp:$params(host):$params(port)"
tsv::set app cmd $cmd

for {set i 0} {$i < $params(slaves)} {incr i} {
  set tid [thread::create -joinable {
    while {true} {
      puts "run [thread::id]: [tsv::get app cmd]"
      if {[catch {exec {*}[tsv::get app cmd] 2>@ stderr} results options]} {
        puts "gpsfish finished abnormally"
      }
      after 15000
    }
  }]

  puts "Started thread: $tid"
  lappend threadIds $tid
}

foreach id $threadIds {
  thread::join $id
}
