(ns gpsdashboard.test.time
  (:use [gpsdashboard.time]
        [midje.sweet]))


(fact "test now-long"
  (now-long) => pos?)
