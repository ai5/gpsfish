(ns gpsdashboard.common
  (:import [java.util Arrays]))


(def DELIMITER ^bytes (.getBytes (str \u001B \# \0 \0) "US-ASCII"))

(defn delimiter?
  [^bytes buf]
  (Arrays/equals ^bytes DELIMITER ^bytes buf))

