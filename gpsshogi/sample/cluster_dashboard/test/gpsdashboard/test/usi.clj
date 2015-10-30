(ns gpsdashboard.test.usi
  (:use [gpsdashboard.usi]
        [midje.sweet]))


(fact "test-parse-moves-in-kanji"
  (let [usi-moves "7g7f 3c3d"]
    (usi-to-kanji usi-moves) => truthy))

