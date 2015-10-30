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

(ns twitter_clj.common
  (:require [clojure.java.io :as io]
            [clojure.string :as str]
            [clojure.tools.logging :as log])
  (:import  [java.nio.file FileSystem FileSystems]
            [java.io File]))


;; ==================================================
;; Global variables
;; ==================================================

(def options (atom {}))
(def pv (atom []))

(def file-separator (System/getProperty "file.separator"))


;; ==================================================
;; Functions
;; ==================================================

(defn- unique-first-move
  "Grouping maps with the first move, take the last map in each group."
  [coll]
  (-> (reduce #(assoc %1 (:first-move %2) %2) {} coll)
    vals))

(defn- sort-by-cp [coll]
  "Sort maps by relative cp values. The highest one is a priority."
  (-> (apply sorted-map-by > (mapcat (fn [line]
                                          [(:relative-cp line) line])
                                        coll))
    vals))

(defn sort-pv
  "Sort multi-PV lines that gpsusi produces.
   On a same depth,
     - if the first move of pv differs each other, sort by cp
     - othersise, the most recent one is a priority"
   [coll]
  (if (seq coll)
    (-> (unique-first-move coll)
      (sort-by-cp))
    coll))

(defn nullif
  "Returns the first non-nil value."
  [& vals]
  (some #(not (nil? %)) vals))

(defn get-nmove
  "Counts number of moves in a USI position line."
  [line]
  {:pre  [(not-empty line)]
   :post [(pos? %)]}
  (let [m (re-find #"moves (.*)" line)]
    (assert m)
    (count (str/split (nth m 1) #" "))))

(defn glob [dir glob]
  (let [dir (io/as-file dir)
        path (str "glob:" (.getPath (io/file dir glob)))
        path (if (= file-separator "\\") (str/replace path "\\" "\\\\") path)
        m (.. ^FileSystem (FileSystems/getDefault) (getPathMatcher path))]
    (filter #(.matches m (.toPath ^File %)) (file-seq dir))))

