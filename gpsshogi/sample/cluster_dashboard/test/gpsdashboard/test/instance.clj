(ns gpsdashboard.test.instance
  (:import [gpsdashboard.message InfoTimeMessage StartposMessage InfoDepthMessage])
  (:require [gpsdashboard.time :as gt]
            [clojure.contrib.logging :as log])
  (:use [gpsdashboard.instance]
        [midje.sweet]))

(background
  (before :facts
    (do
      (reset-instances)
      (add-instance (StartposMessage. 123 "macpro"  "a1 b1"))
      (add-instance (StartposMessage. 124 "macpro2" "a1 b2")))))


(fact "test add-instance for StartposMessage"
  (reset-instances)
  (let [cols (filter-fresh-data 0)]
    (count-unique-ips cols) => 0)
  (add-instance (StartposMessage. 125 "macpro" "a1 b1"))
  (let [cols (filter-fresh-data 0)]
    (count-unique-ips cols) => 1))

(fact "test add-instance for InfoTimeMessage"
  (let [msg123 (InfoTimeMessage. 123 "macpro"  1 456 10)
        no-msg (InfoTimeMessage. 987 "macpro2" 1 456 20)]
    (let [cols (filter-fresh-data 0)]
      (count-unique-ips cols) => 2)
    (add-instance msg123)
    (let [cols (filter-fresh-data 0)]
      (count-unique-ips cols) => 2)
    (add-instance no-msg)
    (let [cols (filter-fresh-data 0)]
      (count-unique-ips cols) => 2)))

(fact "test add-instance for InfoDepthMessage"
  (let [msg123 (InfoDepthMessage. 123 "macpro"  4 7 1 1234 12 123456 "pv1 pv2")
        no-msg (InfoDepthMessage. 987 "macpro2" 4 7 1 1234 12 123456 "pv1 pv2")]
    (let [cols (filter-fresh-data 0)]
      (count-unique-ips cols) => 2)
    (add-instance msg123)
    (let [cols (filter-fresh-data 0)
          c123 (first (filter #(= 123 (:ip %)) cols))]
      (count-unique-ips cols) => 2
      (:depth c123) => 4
      (:seldepth c123) => 7
      (:score c123) => 1234
      (:nps c123) => 123456
      (:pv-str c123) => "pv1 pv2")
    (add-instance no-msg)
    (let [cols (filter-fresh-data 0)]
      (count-unique-ips cols) => 2)))

(fact "test count-nps courner cases"
  (let [msg123 (InfoTimeMessage. 123 "macpro"  1 456 10)
        msg124 (InfoTimeMessage. 124 "macpro2" 1 456 20)]
    (count-nps nil) => zero?
    (count-nps []) => zero?
    (count-nps [msg123 msg124]) => 30))

(fact "test count-unique-ips courner cases"
  (let [msg123 (InfoTimeMessage. 123 "macpro"  1 456 10)
        msg124 (InfoTimeMessage. 124 "macpro2" 1 456 20)]
    (count-unique-ips nil) => zero?
    (count-unique-ips []) => zero?
    (count-unique-ips [msg123 msg124]) => 2))


(fact "integration test; add instance"
  (let [msg123 (InfoTimeMessage. 123 "macpro"  10000 456 100)
        msg124 (InfoTimeMessage. 124 "macpro2" 10000 456 200)]
    (let [cols (filter-fresh-data 0)]
      (count-nps cols)        => 0
      (count-unique-ips cols) => 2)
    (let [cols (filter-fresh-data (gt/now-long))]
      (count-nps cols)        => 0
      (count-unique-ips cols) => 0)
    (add-instance msg123)
    (let [cols (filter-fresh-data 0)]
      (count-nps cols)        => 100
      (count-unique-ips cols) => 2)
    (add-instance msg123)
    (let [cols (filter-fresh-data 0)]
      (count-nps cols)        => 100
      (count-unique-ips cols) => 2)
    (add-instance msg124)
    (let [cols (filter-fresh-data 0)]
      (count-nps cols)        => 300
      (count-unique-ips cols) => 2)))


