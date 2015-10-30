(defproject webinterface "1.1.0"
  :description "GPSShogi Web Interface"
  :url "http://gps.tanaka.ecc.u-tokyo.ac.jp/gpsshogi/"
  :license {:name "GPLv3"
            :url "http://www.gnu.org/copyleft/gpl.html"}
  :dependencies [[org.clojure/clojure "1.6.0"]
                 [org.clojure/tools.cli "0.3.1"]
                 [org.clojure/tools.logging "0.3.0"]
                 [clj-time "0.8.0"]
                 [compojure "1.1.9"]
                 [expectations "2.0.9"]
                 [environ "1.0.0"]
                 [hiccup "1.0.5"]
                 [http-kit "2.1.16"]
                 [ring/ring-defaults "0.1.2"]
                 [ring/ring-devel "1.3.1"]
                 [ring.middleware.logger "0.5.0"]]
  :min-lein-version "2.0.0"
  :plugins [[lein-environ "1.0.0"]
            [lein-ring "0.8.10"]
            [lein-expectations "0.0.8"]]
  :ring {:handler webinterface.web/app}
  :main webinterface.web
  :aliaces {"kifToCsa" ["run" "-m" "webinterface.kifToCsa"]}
  :aot [webinterface.gps
        webinterface.kifToCsa
        webinterface.shogiserver
        webinterface.web])
