(ns gpsdashboard.test.message
  (:use [gpsdashboard.message]
        [midje.sweet]))


(fact "test-parse-message"
  (let [s {:ip 123 :message "macpro info time 123 nodes 456 nps 789"}]
    (parse-message s) => true
    (provided
      (gpsdashboard.message/parse-info-time-message anything anything) => true))
  (let [s {:ip 123 :message "macpro position startpos moves 2g2f 3c3d 7g7f"}]
    (parse-message s) => true
    (provided
      (gpsdashboard.message/parse-startpos-message anything anything) => true)))

(fact "test-parse-startpos-message"
  (let [s "macpro position startpos moves 2g2f 3c3d 7g7f"]
    (parse-startpos-message 1 s) => (contains {:ip 1, :moves-str "2g2f 3c3d 7g7f"})))

(fact "test-parse-info-time-message"
  (let [s "macpro info time 123 nodes 456 nps 789"]
    (parse-info-time-message 1 s) => (contains {:ip 1, :host "macpro", :time 123, :nodes 456, :nps 789})))

(fact "test-parse-info-depth-message"
  (let [s "macpro info depth 4 seldepth 7 score cp 1281 nodes 17028 nps 41735 time 408 pv 4b3c 6g6f 7c7d 6h7i 8a7c 5h6g 4c4d"]
    (parse-info-depth-message 1 s) => (contains {:ip 1 :host "macpro" :depth 4 :seldepth 7 :time 408 :score 1281 :nodes 17028 :nps 41735 :pv-str "4b3c 6g6f 7c7d 6h7i 8a7c 5h6g 4c4d"}))
  (let [s "macpro info depth 4 seldepth 7 score cp -1281 nodes 17028 nps 41735 time 408 pv 4b3c 6g6f 7c7d 6h7i 8a7c 5h6g 4c4d"]
    (parse-info-depth-message 1 s) => (contains {:ip 1 :host "macpro" :depth 4 :seldepth 7 :time 408 :score -1281 :nodes 17028 :nps 41735 :pv-str "4b3c 6g6f 7c7d 6h7i 8a7c 5h6g 4c4d"})))

(fact "test-go-id-message"
  (let [s "server go id 12345 dummy"]
    (parse-go-id-message 1 s) => (contains {:ip 1, :host "server", :msg "server go id 12345 dummy"})))
