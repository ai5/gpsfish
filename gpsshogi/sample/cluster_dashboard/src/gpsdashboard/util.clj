(ns ^{:doc "Utilities"}
  gpsdashboard.util
  (:use [clojure.string :only (split join)]))

(defn wait
  "Wait for f to return true within timeout-millisec, then return true;
   otherwise return false"
  [f timeout-millisec]
  {:pre [(<= 0 timeout-millisec)]}
  (let [unit-millisec 10
        i (quot timeout-millisec unit-millisec)]
    (loop [i i]
      (if (zero? i)
        false ; base
        (if (f)
          true ; base
          (do
            (Thread/sleep unit-millisec)
            (recur (dec i))))))))

(defn parse-moves-str
  "Parses a string representation of moves into an array of moves."
  [moves-str]
  (if (empty? moves-str)
    []
    (split moves-str #"\s")))

(defn join-moves
  "Convert an array of moves into a string representation."
  [moves]
  {:pre [(not-empty moves)]}
  (join " " moves))

(defn parseInt
  [x]
  (cond
    (string? x) (Integer/parseInt x)
    :else       (int x)))
