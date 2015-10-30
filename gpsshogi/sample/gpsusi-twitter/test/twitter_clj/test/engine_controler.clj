(ns twitter_clj.test.engine_controler
  (:use [twitter_clj.engine_controler])
  (:use [clojure.test]))

(deftest test-parse-line []
  (is (= {:depth 6, :seldepth 10, :time 11528, :cp 524, :nodes 769810, :nps 66777
          :pv "1h4e S*4d 3e3f 4d4e 3g4e L*8d 7g8h 5b5d 3d3c+ 2a3c", :first-move "1h4e"}
         (parse-line "info depth 6 seldepth 10 time 11528 score cp 524 nodes 769810 nps 66777 pv 1h4e S*4d 3e3f 4d4e 3g4e L*8d 7g8h 5b5d 3d3c+ 2a3c")))
  (is (= {:depth 6, :seldepth 10, :time 11528, :cp -524, :nodes 769810, :nps 66777,
          :pv "1h4e S*4d 3e3f 4d4e 3g4e L*8d 7g8h 5b5d 3d3c+ 2a3c", :first-move "1h4e"}
         (parse-line "info depth 6 seldepth 10 time 11528 score cp -524 nodes 769810 nps 66777 pv 1h4e S*4d 3e3f 4d4e 3g4e L*8d 7g8h 5b5d 3d3c+ 2a3c"))))


(deftest test-get-nmove []
  (is (= 6 (get-nmove "position startpos moves 2g2f 3c3d 7g7f 5c5d 2f2e 8b5b"))))


