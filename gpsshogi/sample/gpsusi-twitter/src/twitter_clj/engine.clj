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

(ns twitter_clj.engine
  (:require [twitter_clj.common :as common]
            [twitter_clj.subprocess :as sub]
            [clojure.tools.logging :as log]))

;; ==================================================
;; Global variables
;; ==================================================

(def multi-pv-width 50)

(def hash-in-mb 8192) ;; MB

;; ==================================================
;; Functions
;; ==================================================

(defn start-engine
  "Start a gpsusi process"
  [cmd]
  (let [[proc stdin stdout stderr] (sub/fork-usi cmd)]
    (sub/write stdout "usi")
    (let [name (sub/re-wait-for stdin #"^id name (.*)$")]
      (swap! common/options assoc :id-name name))
    (sub/wait-for stdin "usiok")
    (sub/write stdout (format "setoption name Hash value %d" hash-in-mb))
    (sub/write stdout (format "setoption name MultiPVWidth value %d" multi-pv-width))
    (sub/write stdout (format "setoption name OwnBook value false"))
    (sub/write stdout "isready")
    (sub/wait-for stdin "readyok")
    (sub/write stdout "usinewgame")
    [proc stdin stdout stderr]))

(defn stop-engine
  "Stop a gpsusi process"
  [proc stdin stdout stderr]
  (sub/write stdout "quit")
  (.close stdout)
  (.close stdin)
  (.close stderr)
  (.waitFor proc))

