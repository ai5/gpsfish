;;; Copyright (C) 2014 Team GPS.
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

(ns twitter_clj.draw_image
  (:require [twitter_clj.subprocess :as sub]
            [twitter_clj.title :as title]
            [clojure.tools.logging :as log]))

;; ==================================================
;; Global variables
;; ==================================================


;; ==================================================
;; Functions
;; ==================================================

(defn file-name
  "Return a file name that corresponds to the n-th move.
   ex. board_image_12.png"
  ([nmove]
   (format "board_image_%d.png" nmove))
  ([nmove seconds index]
   (format "board_image_%d_%d_%d.png" nmove seconds index)))

(defn- draw-image
  [exe file title position]
  (let [cmd [exe "-d" "."
                 "-t" title
                 "-b" (:black @title/kif-info)
                 "-w" (:white @title/kif-info)
                 "-o" file
                 position]
        proc ^Process (.exec (Runtime/getRuntime) (into-array String cmd))
        stdin  (clojure.java.io/reader (.getInputStream proc))
        stderr (clojure.java.io/reader (.getErrorStream proc))]
    (log/info cmd)
    (sub/dump stdin)
    (sub/dump stderr)))

(defn draw
  "Generates a board image, the file name of which is board_image_<nth>.png"
  ([exe nth position]
   (let [file  (file-name nth) 
         title (format "%s [%s手目まで]" (:title @title/kif-info) nth)]
     (draw-image exe file title position)))
  ([exe nth depth seconds index position]
   (let [file  (file-name nth seconds index)
         title (format "%s 想定図%d [%d+%d手]"
                       (:title @title/kif-info)
                       (inc index) ; starting with 1
                       nth
                       depth)]
     (draw-image exe file title position))))

