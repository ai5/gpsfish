(ns twitter_clj.test.snapshot
  (:use [twitter_clj.snapshot])
  (:use [clojure.test])
  (:require [twitter_clj.common :as common]
            [twitter_clj.engine_controler :as ec]))


(deftest test-take-pv []
  (reset! common/pv [(ec/parse-line "info depth 5 seldepth 0 time 34 score cp -1148897 nodes 3802 nps 111823 pv resign")])
  (is (= "-1148897 " (take-pv 104))))
  
(deftest upt-to-moves []
  (let [s "1 2 3"]
    (is (= s (up-to-moves 3 s)))
    (is (= s (up-to-moves 4 s)))
    (is (= "1 2" (up-to-moves 2 s)))
    (is (= "1"   (up-to-moves 1 s)))
    (is (= ""    (up-to-moves 0 s)))
    (is (= ""    (up-to-moves 1 "")))))
