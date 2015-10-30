(ns gpsdashboard.monitor_console
  (:require [gpsdashboard.instance :as instance]
            [gpsdashboard.packet :as packet]
            [gpsdashboard.subscriber :as sub]
            [gpsdashboard.time :as gt]
            [clojure.contrib.command-line :as cmd]
            [clojure.contrib.logging :as log])
  (:import [gpsdashboard.server_protocol ServerProtocol])
  (:gen-class))


(defn message-handler
  [cols]
  (println (format "%s  % 3d IPs  % ,8d nps"
                   (gt/now-string)
                   (instance/count-unique-ips cols)
                   (instance/count-nps cols)))
  true)


;; ==================================================
;; Global variables
;; ==================================================

;; ==================================================
;; Functions
;; ==================================================


(defn -main [& args]
  (cmd/with-command-line args
    "GPSShogi Clustor Dashboard - CUI Monitor"
    [[host "host" "localhost"]
     [port "port" 8240]
     remaining]
    (let [^ServerProtocol subscriber  (sub/new-subscriber message-handler {:port port :host host})]
      (.start subscriber)
      (.join subscriber)
      (.shutdown subscriber)
      (shutdown-agents))))
