(ns gpsdashboard.test.endToEnd
  (:require [gpsdashboard.test.gpsusi_stub :as gpsusi]
            [gpsdashboard.collector :as collector]
            [gpsdashboard.instance :as instance]
            [gpsdashboard.monitor_console :as cui]
            [gpsdashboard.packet :as packet]
            [gpsdashboard.publisher :as pub]
            [gpsdashboard.server :as server]
            [gpsdashboard.subscriber :as sub]
            [gpsdashboard.util :as util])
  (:use [midje.sweet]))


(background
  (before :facts
    (do
      (instance/reset-instances)
      (packet/reset-packets))))


(fact "server receives a submitted info time message"
  (letfn [(stop-service-later [service]
            (send-off (agent service)
                      (fn [service]
                        (Thread/sleep 500)
                        (.shutdown service))))]
    (let [config     server/default-server-config
          server     (server/new-server config)
          collector  (collector/new-collector 1)
          publisher  (pub/new-publisher {:interval-sec 1})
          subscriber (sub/new-subscriber cui/message-handler {:interval-sec 1})
          services [server collector publisher subscriber]]
      (dorun (map #(.start %) services))
      (dorun (map (fn [s]
                    (util/wait #(.ready? s) 100))
                  services))
      (Thread/sleep 100)
      (gpsusi/send-info-time-message (:port config) 1 123 456)
      (Thread/sleep 200)
      (gpsusi/send-info-time-message (:port config) 1 123 456)
      (Thread/sleep 500)
      (gpsusi/send-info-time-message (:port config) 1 123 456)
      (Thread/sleep 500)
      (packet/get-average) => pos?
      (dorun (map stop-service-later (reverse services)))
      (dorun (map #(.join %) (reverse services)))
      (shutdown-agents))))

