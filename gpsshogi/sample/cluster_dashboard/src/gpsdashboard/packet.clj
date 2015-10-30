(ns gpsdashboard.packet
  (:require [gpsdashboard.ema :as ema]
            [gpsdashboard.time :as gt]
            [clojure.contrib.logging :as log]
            [clojure.string :as str]
            [clj-time.core :as ctc])
  (:import  [gpsdashboard.ema EMA]))


(def counter (atom 0))
(def start-time (atom (ctc/now)))

(defn count-packets
  []
  @counter)

(defn add-packet
  []
  (swap! counter inc))

(defn packets-per-sec
  []
  (let [interval-secs (-> (ctc/interval @start-time (ctc/now))
                          (ctc/in-secs))]
    (if (pos? interval-secs)
      (/ @counter interval-secs)
      0)))

(def average (atom (ema/new-EMA 0.1)))

(defn reset-packets
  []
  (let [pps (packets-per-sec)]
    (reset! counter 0)
    (reset! start-time (ctc/now))
    (swap! average #(.update #^EMA % pps))
    pps))

(defn get-average
  []
  (.value #^EMA @average))

