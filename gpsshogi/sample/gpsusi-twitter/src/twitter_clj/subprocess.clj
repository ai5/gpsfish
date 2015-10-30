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

(ns twitter_clj.subprocess
  (:import (java.io InputStreamReader OutputStreamWriter))
  (:require [clojure.tools.logging :as log]))

;; ==================================================
;; Functions
;; ==================================================

(defn fork-usi
  "Fork a new sub-process."
  [cmd]
  (log/infof "Starting...  %s" cmd)
  (let [proc   (.exec (Runtime/getRuntime) cmd)
        stdout (clojure.java.io/writer (.getOutputStream proc))
        stdin  (clojure.java.io/reader (.getInputStream proc) :encoding "EUC-JP")
        stderr (clojure.java.io/reader (.getErrorStream proc))]
    [proc stdin stdout stderr]))

(defn write
  "Send a line to an output stream."
  [out line]
  (.write out line)
  (.newLine out)
  (.flush out))

(defn wait-for
  "Read lines from an input stream until getting a line."
  [in line]
  {:pre [(not-empty line)]}
  (loop [lines (line-seq in)]
    (if-let [s (first lines)]
      (if (= s line)
        s ;; base
        (recur (rest lines))))))

(defn re-wait-for
  "Read lines from an input stream until finding a line matched with a
   regular expression"
  [in re]
  (loop [lines (line-seq in)]
    (if-let [s (first lines)]
        (if-let [m (re-find re s)]
          (nth m 1) ;; base
          (recur (rest lines))))))

(defn dump
  [in]
  (dorun
    (map println (line-seq in))))
