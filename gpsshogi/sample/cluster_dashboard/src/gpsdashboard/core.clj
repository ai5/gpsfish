(ns gpsdashboard.core
  (:require [gpsdashboard.collector :as collector]
            [gpsdashboard.publisher :as pub]
            [gpsdashboard.server :as server]
            [gpsdashboard.util :as util]
            [clojure.contrib.command-line :as cmd]
            [clojure.contrib.logging :as log])
  (:import [gpsdashboard.server_protocol ServerProtocol])
  (:gen-class))

;; ==================================================
;; Global variables
;; ==================================================

;; ==================================================
;; Functions
;; ==================================================


(defn -main [& args]
  (cmd/with-command-line args
    "GPSShogi Clustor Dashboard - Server"
    [[expiration            "Ignore node data older than expiration seconds" 30]
     [port                  "UDP port"                              4120]
     [publisher-port        "publisher port"                        8240]
     [dump-interval         "interval seconds to dump collected states. (0 disable)"  0]
     [publish-interval      "interval seconds to publish messages." 3]
     remaining]
    (let [config    {:port (util/parseInt port)}
          server    (server/new-server config)
          collector (if (zero? (util/parseInt dump-interval))
                      nil
                      (collector/new-collector (util/parseInt dump-interval)))
          publisher (pub/new-publisher {:port           (util/parseInt publisher-port)
                                        :interval-sec   (util/parseInt publish-interval)
                                        :expiration-sec (util/parseInt expiration)})
          services  (remove nil? [server collector publisher])]
      (dorun (map #(.start ^ServerProtocol %) services))
      (dorun (map #(.join ^ServerProtocol %) (reverse services)))
      (shutdown-agents))))

