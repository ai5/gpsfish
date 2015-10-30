(ns gpsdashboard.test.server
  (:require [gpsdashboard.instance :as instance]
            [gpsdashboard.server :as server]
            [gpsdashboard.packet :as packet])
  (:use [midje.sweet]))


(fact "test handle-message"
  (server/handle-message {:ip 1 :message "macpro info time 123 nodes 456 nps 789"}) => truthy
  (provided
    (packet/add-packet) => true
    (instance/add-instance anything) => true))

