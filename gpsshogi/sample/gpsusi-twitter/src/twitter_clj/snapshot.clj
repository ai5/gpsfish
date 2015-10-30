;;; Copyright (C) 2011-2014 Team GPS.
;;; 
;;; This program is free software; you can redistribute it and/or modify
;;; it under the terms of the GNU General Public License as published by
;;; the Free Software Foundation; either version 2 of the License, or
;;; (at your option) any later version.
;;; 
;;; This program is distributed in the hope that it will be useful,
;;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;; GNU General Public License for more details.
;;; 
;;; You should have received a copy of the GNU General Public License
;;; along with this program; if not, write to the Free Software
;;; Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

(ns twitter_clj.snapshot
  (:require [twitter_clj.common :as common]
            [twitter_clj.draw_image :as image]
            [twitter_clj.ki2 :as ki2]
            [twitter_clj.twitter :as twitter]
            [clojure.core.async :as async]
            [clojure.string :as str]
            [clojure.tools.logging :as log]))

;; ==================================================
;; Global variables
;; ==================================================

(def timers
;  (concat [0 30] (iterate #(+ % 60) 90))) ; for NHK
  (concat [0 300] (iterate #(+ % 900) 900)))

(def wake-up-snapshot-chan (async/chan (async/sliding-buffer 1)))

;; ==================================================
;; Functions
;; ==================================================

(defn interrupt
  []
  (async/>!! wake-up-snapshot-chan 1))

(defn up-to-moves
  "Take at most up-to moves from moves-str delimitted by space"
  [up-to moves-str]
  (->> (str/split moves-str #" ")
    (take up-to)
    (str/join " ")))

(defn format-pv
  "Taking a map representing a PV sequence, format it into a string."
  [m]
  (let [ki2moves (if (= "resign" (:pv m))
                    ""
                    (ki2/show (->> (:pv m)
                                (up-to-moves 11))))]
    (format "%d %s" (:cp m) ki2moves)))

(defn draw-board-for-pv
  "Taking a map representing a PV sequence, draw a board."
  [m nmove seconds index]
  (let [line (str (:position m) " " (:pv m))]
    (image/draw (:image-generator @common/options)
                nmove
                (-> (:pv m)
                  (str/split  #"\s")
                  (count))
                seconds
                index
                line)))

(defn take-pv-sequences
  "Join each PV sequences"
  []
  (->> @common/pv
    (common/sort-pv)
    (map format-pv)
    (str/join " / ")))
  
(defn prn-snapshot
  "Generate a message to tweet for this snapshot."
  [nmove last-move sec]
  ; Draw board images for each pv
  (dorun
   (map-indexed (fn [idx m]
                  (draw-board-for-pv m nmove sec idx)) 
                (-> @common/pv
                  common/sort-pv)))
  ; Return a message tweet
  (format "[(%d) %s] <%ds> %s" nmove last-move sec (take-pv-sequences)))

(defn interval-seconds
  "Returns an interval in seconds from a start time to now."
  [start]
  (let [now (java.util.Date.)
        interval (quot (- (.getTime now) (.getTime ^java.util.Date start)) 1000)]
    interval))

(defn sleep-while
  "Blocks the current thread for millis and returns :interrupted or
  :timedout as a reason to be woken up."
  [millis]
  (when (pos? millis)
    (log/debug (format "Sleeping a while: %d secs..." (/ millis 1000)))
    (let [ret (async/alt!!
                wake-up-snapshot-chan  ([val ch] :interrupted)
                (async/timeout millis) ([val ch] :timedout))]
      (log/debug "Awake!!!")
      ret)))

(defn next-interval
  [i]
  (let [[t1 t2]  (take 2 (drop i timers))]
    (- t2 t1)))

(defn tweet-current-pv
  "Send the i-th tweet of the same possition. If i is zero, it is the first tweet of a position."
  [nmove last-move sec i]
  (let [s (prn-snapshot nmove last-move sec)]
    (twitter/post-move s nmove sec i)))

(defn pv-changed?
  "Checks if the current pv is changed from the last-pv. The last-pv can
  be null in a case of the first snapshot. Returns an array of the result
  (ture if there is any change; otherwise false) and the current pv
  (a sequence of usi movs)."
  [last-pv]
  (let [current-primary-pv (->> @common/pv
                             (common/sort-pv)
                             first
                             :pv
                             (up-to-moves 3))]
    [(not= current-primary-pv last-pv) current-primary-pv]))

(defn start-snapshot-thread
  "Starts a new snapshot thread and returns a channel."
  []
  (let [[nmove last-move] (ki2/current-info)
        start-time        (java.util.Date.)]
    (async/thread
      (try
        (log/debug "Started a new snapshot thread")
        (loop [i       0
               last-pv nil]
          (let [reason (sleep-while (* (next-interval i) 1000))
                [result last-pv] (pv-changed? last-pv)]
            (if result
              (let [this-time (if (= reason :timedout)
                                (nth timers (inc i))
                                (interval-seconds start-time))]
                (tweet-current-pv nmove last-move this-time i))
              (log/info "The pv is still unchanged. Skipped tweeting it."))
            (if (= reason :timedout)
              (recur (inc i) last-pv)
              (log/debug "The snapshot thread finished")))) ; base
        (catch Exception e
          (log/error e "Snapshot thread failed.")
          (throw e))))))
