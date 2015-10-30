(ns gpsdashboard.time
  (:require [clj-time.core :as ctc]
            [clj-time.coerce :as ctcoerce]
            [clj-time.format :as ctf]
            [clj-time.local  :as ctl]))


(defn now-long
  "Return current milliseconds since epoc"
  []
  (-> (ctl/local-now)
      (ctcoerce/to-long)))

(defn now-long-before-sec
  [before-sec]
  {:pre [(pos? before-sec)]}
  (-> (ctc/minus (ctl/local-now) (ctc/secs before-sec))
      (ctcoerce/to-long)))

(defn now-string
  ([]
   (now-string (ctl/local-now)))
  ([date]
   (ctl/format-local-time date :date-hour-minute-second)))

(defn from-string
  [s]
  (ctl/to-local-date-time s))
