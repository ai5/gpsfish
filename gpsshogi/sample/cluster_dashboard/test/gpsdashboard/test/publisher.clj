(ns gpsdashboard.test.publisher
  (:require [gpsdashboard.common :as common]
            [gpsdashboard.instance :as instance]
            [gpsdashboard.publisher :as pub]
            [gpsdashboard.subscriber :as sub]
            [gpsdashboard.time :as gt]
            [clojure.java.io :as io]
            [clojure.contrib.logging :as log])
  (:use [midje.sweet])
  (:import [java.io ByteArrayInputStream InputStreamReader BufferedInputStream DataInputStream]
           [java.util Arrays]
           [java.util.zip GZIPInputStream]
           [java.net Socket]))


(against-background
  [(instance/filter-fresh-data anything) => [{:ip 1 :host "h1"} {:ip 2 :host "h2"}]]

  (fact "Publisher starts and a client receives an int"
    (let [publisher (pub/new-publisher {:interval-sec 1})]
      (.start publisher)
      (Thread/sleep 100)
      (.ready? publisher) => true
      (let [handler (fn [data] (prn data))
            subscriber (sub/new-subscriber handler {:interval-sec 1})]
        (assert subscriber)
        (.start subscriber)
        (Thread/sleep 300)
        (.ready? subscriber) => true
        (Thread/sleep 1000)
        (.shutdown subscriber))
      (.shutdown publisher)))

  (fact "Data round trip"
    (let [compressed-data (pub/get-data (gt/now-long))]
      (sub/parse-data compressed-data) => [{:ip 1 :host "h1"} {:ip 2 :host "h2"}])))

