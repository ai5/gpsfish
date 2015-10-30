(defproject gpsdashboard "1.2.1"
  :description "GPSShogi Clustor Dashboard"
  :warn-on-reflection true
  :jvm-opts ["-d64" "-server"]
  ;:repositories {"LocalDebian" "file:///usr/share/maven-repo"}
  :dependencies [[org.clojure/clojure "1.5.1"]
                 [org.clojure/clojure-contrib "1.2.0"]
                 [org.clojure/data.json "0.2.1"]
                 [clj-time/clj-time "0.4.4"]
                 [org.jfree/jfreechart "1.0.14"]
                 [ch.randelshofer/treeviz "1.1.1"]
                 [net.java.dev.jna/jna "3.2.4"]
                 [log4j "1.2.17"]
                 [midje "1.5.1"]]
  :aot [gpsdashboard.usi_tree_node       ; deftype
        gpsdashboard.usi_tree_node_info  ; deftype
        net.n01se.clojure_jna
        gpsdashboard.benchmark
        gpsdashboard.ema
        gpsdashboard.collector
        gpsdashboard.common
        gpsdashboard.core
        gpsdashboard.dump
        gpsdashboard.instance
        gpsdashboard.message
        gpsdashboard.monitor_console
        gpsdashboard.monitor_swing
        gpsdashboard.network_util
        gpsdashboard.packet
        gpsdashboard.publisher
        gpsdashboard.server
        gpsdashboard.server_protocol
        gpsdashboard.subscriber
        gpsdashboard.time
        gpsdashboard.treemap
        gpsdashboard.usi]
  :profiles {:dev {:plugins [[lein-midje "3.0.0"]]}}
  :aliases {"gui" ["run" "-m" "gpsdashboard.monitor_swing"]}
  :main gpsdashboard.core)

