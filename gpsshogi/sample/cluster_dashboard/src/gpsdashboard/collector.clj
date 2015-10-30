(ns gpsdashboard.collector
  (:require [gpsdashboard.dump :as dump]
            [gpsdashboard.instance :as instance]
            [gpsdashboard.packet :as packet]
            [gpsdashboard.time :as gt]
            [clojure.contrib.logging :as log])
  (:import [java.util.concurrent ScheduledThreadPoolExecutor TimeUnit])
  (:use gpsdashboard.server_protocol))


;; ==================================================
;; Global variables
;; ==================================================

;; ==================================================
;; Functions
;; ==================================================

(defn collect
  []
  (let [expiration-sec 60
        since (gt/now-long-before-sec expiration-sec)
        cols  (instance/filter-fresh-data since)
        pps   (packet/reset-packets)]
    (log/debug (format "% 9.2f packets/sec" (double pps)))
    (dump/dump cols)))

(defn new-collector
  "Return a new collector object, which summarizes received messages."
  [interval-sec]
  {:pre [(pos? interval-sec)]}
  (let [pool-size 1
        executor  (ScheduledThreadPoolExecutor. pool-size)]
    (reify ServerProtocol ; return an object
      (start [self]
        (log/info (format "Starting a collector with %d seconds interval..." interval-sec))
        (.scheduleAtFixedRate executor collect interval-sec interval-sec TimeUnit/SECONDS)) 
      (ready? [self]
        (not (.isShutdown executor)))
      (join [_]
        (.awaitTermination executor 1 TimeUnit/SECONDS))
      (shutdown [self]
        (log/info "Stopping a collector...")
        (.shutdown executor)
        (.join self)))))

