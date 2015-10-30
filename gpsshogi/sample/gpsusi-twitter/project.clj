(defproject twitter_clj "1.0.1"
  :description "twitter bot using gpssui"
  :url "http://gps.tanaka.ecc.u-tokyo.ac.jp/gpsshogi/"
  :license {:name "GPLv2 or later"
            :comments "Copyright (C) 2014 Team GPS."}
  :repositories {"debian" {:url "file:///usr/share/maven-repo"}}
  :jvm-opts ["-server"]
  :dependencies [[org.clojure/clojure "1.6.0"]
                 [org.clojure/core.async "0.1.303.0-886421-alpha"]
                 [org.clojure/tools.logging "0.3.0"]
                 [org.clojure/tools.cli "0.3.1"]
                 [log4j "1.2.17"]
                 [org.twitter4j/twitter4j-core "4.0.2"]]
  :aot [twitter_clj.common
        twitter_clj.draw_image
        twitter_clj.engine
        twitter_clj.engine_controler
        twitter_clj.ki2
        twitter_clj.snapshot
        twitter_clj.subprocess
        twitter_clj.title
        twitter_clj.twitter
        twitter_clj.usi_converter]
  :global-vars {*warn-on-reflection* true
                *assert* false}
  :main twitter_clj.engine_controler
  :aliases {"usi_converter" ["trampoline" "run" "-m" "twitter_clj.usi_converter"]
            "engine"        ["trampoline" "run" "-m" "twitter_clj.engine_controler"]
            "tweet"         ["trampoline" "run" "-m" "twitter_clj.twitter"]})

