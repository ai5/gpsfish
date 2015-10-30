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

(ns twitter_clj.title
  (:require [clojure.java.io :as io]
            [clojure.tools.logging :as log]))

(def kif-info (atom {}))

(def full-to-ascii
  {\１ \1 \２ \2 \３ \3 \４ \4 \５ \5 \６ \6 \７ \7 \８ \8 \９ \9 \０ \0})

(defn convert-full-to-ascii
  "Convert full-with integer characters in a string into corresponding half-width characters."
  [string]
  (loop [coll (seq string)
         ret  []]
    (if (seq coll)
      (let [c (first coll)]
        (if-let [a (full-to-ascii c)]
          (recur (rest coll) (conj ret a))
          (recur (rest coll) (conj ret c))))
      (apply str ret))))

(defn swap-kif-info
  "Update a map swap-info with a key k and value v."
  [k v]
  (swap! kif-info assoc k (convert-full-to-ascii v)))

(defn parse-kif
  "Parse a .kif file and stores information into kif-info."
  [file]
  (with-open [rdr (io/reader file :encoding "Windows-31J")]
    (doseq [line (line-seq rdr)]
      (condp (comp seq re-seq) line
        #"^棋戦：(第.*?戦.*?級.*?戦)" :>> #(let [x (first %)]
                                             (swap-kif-info :title (nth x 1)))
        #"^棋戦：(第.*?戦).*(第.*局)" :>> #(let [x (first %)
                                                 title (format "%s %s" (nth x 1) (nth x 2))]
                                             (swap-kif-info :title title))
        #"^先手：(.*)"          :>> #(let [x (first %)]
                                       (swap-kif-info :black (nth x 1)))
        #"^後手：(.*)"          :>> #(let [x (first %)]
                                       (swap-kif-info :white (nth x 1)))
        :ignore))))

(defn twitter-title
  "Return a game title for Twitter."
  []
  (format "%s (先手: %s, 後手: %s) [gpsfish]"
          (:title @kif-info)
          (:black @kif-info)
          (:white @kif-info)))

