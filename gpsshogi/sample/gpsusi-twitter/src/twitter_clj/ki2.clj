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

(ns twitter_clj.ki2
  (:require [twitter_clj.common :as common]
            [twitter_clj.engine :as engine]
            [twitter_clj.subprocess :as sub]
            [clojure.string :as str]
            [clojure.tools.logging :as log]))

;; ==================================================
;; Global variables
;; ==================================================

(def proc   (atom nil))
(def stdin  (atom nil))
(def stdout (atom nil))
(def stderr (atom nil))


;; ==================================================
;; Functions
;; ==================================================

(defn start-ki2-engine
  "Start a ki2 engine converting usi moves to Kanji ones.
   Note that this engine has its own state."
  [gpsusi-cmd]
  {:pre [(every? nil? [@proc @stdin @stdout @stderr])]}
  (log/info "Starting the ki2 engine...")
  (let [[new-proc new-stdin new-stdout new-stderr] (engine/start-engine gpsusi-cmd)]
    (reset! proc   new-proc)
    (reset! stdin  new-stdin)
    (reset! stdout new-stdout)
    (reset! stderr new-stderr)))

(defn stop-ki2-engine
  "Stop a ki2 engine."
  []
  {:pre [(not (nil? @proc))]}
  (log/info "Stopping the ki2 engine...")
  (engine/stop-engine @proc @stdin @stdout @stderr)
  (reset! proc   nil)
  (reset! stdin  nil)
  (reset! stdout nil)
  (reset! stderr nil))

(defn is-valid-position
  "Validates a position. If it is valid, return true; false otherwise."
  [position]
  {:pre [(not (nil? @proc))]}
  (log/debugf "Validate a position: %s" position)
  (sub/write @stdout (format "isvalidposition %s" position))
  (let [ret (.readLine ^java.io.BufferedReader @stdin)]
    (= "valid" ret)))

(defn set-position
  "Set a usi position to the engine."
  [position]
  {:pre [(not (nil? @proc))]}
  (log/debugf "Set ki2 engine: %s" position)
  (sub/write @stdout position))

(defn filter-threatmate
  [moves]
  (loop [moves moves
         ret   []]
    (let [i (.indexOf ^String moves "(")]
      (if (neg? i)
        [moves ret] ;; base
        (let [before (subs moves 0 i)
              c      (dec (count (str/split before #" ")))
              after  (subs moves (inc (.indexOf ^String moves ")")))]
          (assert (= \^ (nth moves (inc i))))
          (recur (str before after) (conj ret c)))))))

(def TSUMERO "(詰めろ)")

(defn add-threatmate
  [moves i]
  {:pre [(not-empty moves) (<= 0 i)]}
  (loop [p 0
         i (inc i)]
    (assert (<= 0 p (dec (count moves))) ) 
    (if (= p (dec (count moves)))
      (do
        (assert (zero? i))
        (str moves TSUMERO))
      (if (some #(= % (nth moves p)) [\▲ \△])
        (if (zero? i)
          (str (subs moves 0 p) TSUMERO (subs moves p)) ;; base
          (recur (inc p) (dec i)))
        (recur (inc p) i)))))

(defn show
  "Convert usi moves to Kanji moves by using the ki2 engine."
  [usi-moves]
  {:post [(not-empty %)]}
    (log/debugf "Converting usi moves to Kanji: %s" usi-moves)
    (let [[moves indexes] (filter-threatmate usi-moves)] 
      (sub/write @stdout (format "ki2moves %s" moves))
      (let [line      (.readLine ^java.io.BufferedReader @stdin)
            m         (re-find #"^ki2moves (.*)$" line)
            _         (assert m)
            ki2-moves (nth m 1)]
        (loop [moves   ki2-moves
               indexes indexes]
          (if (seq indexes)
            (recur (add-threatmate moves (first indexes)) (rest indexes))
            moves)))))

(defn current-info
  "Retrieve the latest move etc. from the engine."
  []
  (sub/write @stdout "ki2currentinfo")
  (let [line (.readLine ^java.io.BufferedReader @stdin)
        m (re-find #"^ki2currentinfo (\d+) (.*)$" line)]
    (if m
      [(Integer. ^String (nth m 1)) (nth m 2)]
      [0 ""])))

