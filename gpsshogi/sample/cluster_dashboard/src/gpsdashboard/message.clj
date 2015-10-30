(ns gpsdashboard.message
  (:require [gpsdashboard.util :as util] 
            [clojure.contrib.logging :as log])
  (:use [clojure.string :only (split)]))


;; ========== InfoTimeMessage

(defrecord InfoTimeMessage [^long ip ^String host
                            ^int time ^int nodes ^int nps])

(defn new-info-time-message
  []
  (InfoTimeMessage. 0 "" 0 0 0))

;; ========== StartposMessage

(defrecord StartposMessage [^long ip ^String host
                            ^String moves-str])

;; ========== InfoDepthMessage

(defrecord InfoDepthMessage [^long ip ^String host
                             ^int depth ^int seldepth ^int time ^int score ^int nodes ^int nps ^String pv-str])

(defn new-info-depth-message
  []
  (InfoDepthMessage. 0 "" 0 0 0 0 0 0 ""))

;; ========== InfoStartNewMasterPositionMessage

(defrecord InfoStartNewMasterPositionMessage [^long ip ^String host
                                              ^String moves-str])

;; ========== IgnoreMovesMessage

(defrecord IgnoreMovesMessage [^long ip ^String host
                               ^String ignore-moves-str])

(defn parse-startpos-message
  [ip msg]
  (let [matched (re-find #"(.*?) position startpos moves (.*)" msg)]
    (StartposMessage. ip
                      (get matched 1)
                      (get matched 2))))

(defmacro parse-integer
  [key-str ret xs]
  `(let [symbl# (keyword ~key-str)
         ret# ~ret
         xs#  ~xs]
     (recur (merge ret# {symbl# (Integer/parseInt (second xs#))}) (rest (rest xs#)))))

(defmacro parse-string
  [key-str ret xs]
  `(let [symbl# (keyword ~key-str)
         ret# ~ret
         xs#  ~xs]
     (recur (merge ret# {symbl# (second xs#)}) (rest (rest xs#)))))

(defn parse-info-time-message
  [ip msg]
  (let [xs  (split msg #"\s")]
    (assert (= "info"  (nth xs 1)))
    (assert (or (= "nodes" (nth xs 2))
                (= "time"  (nth xs 2))))
    (assert (< 4 (count xs)))
    (loop [ret (merge (new-info-time-message) {:ip ip :host (nth xs 0)})
           xs  (rest xs)]
      (if-not (seq xs)
        ret
        (condp = (first xs)
          "info"     (recur ret (rest xs))
          "time"     (parse-integer "time"  ret xs)
          "nodes"    (parse-integer "nodes" ret xs)
          "nps"      (parse-integer "nps"   ret xs)
          "hashfull" (recur ret (rest (rest xs))) ; TODO ignored
          (log/error (str "Unknown attributes for InfoTimeMessage: " xs)))))))

(defn parse-info-depth-message
  [ip msg]
  (let [xs  (split msg #"\s")]
    (assert (= "info"  (nth xs 1)))
    (assert (= "depth" (nth xs 2)))
    (assert (< 4 (count xs)))
    (loop [ret (merge (new-info-depth-message) {:ip ip :host  (nth xs 0)})
           xs  (rest xs)]
      (if-not (seq xs)
        ret
        (condp = (first xs)
          "info"     (recur ret (rest xs))
          "depth"    (parse-integer "depth"    ret xs)
          "seldepth" (parse-integer "seldepth" ret xs)
          "score"    (recur ret (rest xs))
          "cp"       (parse-integer "score"    ret xs)
          "nodes"    (parse-integer "nodes"    ret xs)
          "nps"      (parse-integer "nps"      ret xs)
          "time"     (parse-integer "time"     ret xs)
          "pv"       (recur (merge ret {:pv-str (util/join-moves (rest xs))})
                            nil)
          (log/error (str "Unknown attributes for InfoDepthMessage: " xs)))))))

(defn parse-info-start-new-master-position-message
  [ip msg]
  (let [matched (re-find #"(.*?) info start new master position position startpos moves (.*)" msg)]
    (InfoStartNewMasterPositionMessage.
      ip
      (get matched 1)
      (get matched 2))))

(defn parse-ignore-moves-message
  [ip msg]
  (let [matched (re-find #"(.*?) ignore_moves (.*)" msg)
        ignore-moves-str (str (get matched 2) " *")]
    ;(log/debug msg)
    (IgnoreMovesMessage.
      ip
      (get matched 1)
      ignore-moves-str)))

;; ========== GoIdMessage

(defrecord GoIdMessage [^long ip ^String host
                        ^String msg])

(defn parse-go-id-message
  [ip msg]
  (GoIdMessage. ip "server" msg))

(defn parse-message
  [{:keys [ip message]}]
  {:pre [(not-empty message)]}
  (try
    (condp (comp seq re-seq) message
      #"^\S+? info time"                    (parse-info-time-message ip message)
      #"^\S+? info nodes"                   (parse-info-time-message ip message)
      #"^\S+? info depth (\d+)$"            nil
      #"^\S+? info depth (\d+) "            (parse-info-depth-message ip message)
      #"^\S+? info start new master position (.*)$" (parse-info-start-new-master-position-message ip message)
      #"^\S+? position startpos moves (.*)" (parse-startpos-message ip message)
      #"^\S+? ignore_moves"                 (parse-ignore-moves-message ip message)
      #"^\S+? bestmove"                     nil ;; TODO
      #"^\S+? setoption"                    nil
      #"^\S+? ping"                         nil
      #"^\S+? echo ping"                    nil
      #"^\S+? isready"                      nil
      #"^\S+? readyok"                      nil
      #"^\S+? usinewgame"                   nil
      #"^\S+? server go id "                (parse-go-id-message)
      #"^\S+? go "                          nil
      #"^\S+? stop"                         nil
      #"^\S+? checkmate"                    nil ; checkmate node
      #"^\S+? server game finished"         nil
      (do
        (log/debug (format "Received a message of unknown type [%d bytes, from %d]: %s"
                           (count message) ip message))
        nil))
    (catch Exception e
      (log/error (str "Failed to parse message: [" ip "] " message))
      nil)))
