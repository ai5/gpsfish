(ns gpsdashboard.test.collector
  (:require [gpsdashboard.packet :as packet])
  (:use [gpsdashboard.collector]
        [midje.sweet]))


(fact "integration test for collect"
  (collect) => anything
  (provided
    (packet/reset-packets) => 10))

