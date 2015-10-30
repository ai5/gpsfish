(ns gpsdashboard.test.ema
  (:use [gpsdashboard.ema]
        [midje.sweet])
  (:import [gpsdashboard.ema EMA]))


(fact "test ema update"
  (let [v #^EMA (new-EMA 0.1)]
    (.value #^EMA v) => zero?
    (.value #^EMA (.update #^EMA v 100)) => 10.0))


