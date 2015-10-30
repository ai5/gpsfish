(ns gpsdashboard.test.util
  (:use [gpsdashboard.util]
        [midje.sweet]))


(fact "test wait"
  (wait (fn [] false) 100) => false
  (wait (fn [] true)  100) => true)

