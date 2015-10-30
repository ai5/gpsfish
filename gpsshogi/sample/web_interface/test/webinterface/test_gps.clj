(ns webinterface.test_gps
  (:use expectations)
  (:require [webinterface.gps :as gps]))

(expect 1 1)

;(expect-let [buoyname (gps/generate-buoy-game-name)
;             p        (gps/spawn-gps buoyname "black")]
;  #(not (nil? %)) (:process p))

(expect "" (gps/ps-gps))
(expect false (gps/gps-running?))

;(expect "" (gps/kill-gps))
