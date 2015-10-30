(ns gpsdashboard.test.packet
  (:use [gpsdashboard.instance]
        [gpsdashboard.packet]
        [midje.sweet]))


(background
  (before :facts
    (reset-packets)))


(fact "test-add-packet"
  (count-packets) => 0
  (add-packet)
  (count-packets) => 1
  (reset-packets)
  (count-packets) => 0
  (add-packet)
  (add-packet)
  (count-packets) => 2
  (reset-packets)
  (count-packets) => 0)

