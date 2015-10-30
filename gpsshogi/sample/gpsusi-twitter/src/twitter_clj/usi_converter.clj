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

(ns twitter_clj.usi_converter
  (:require [clojure.string :as str]
            [clojure.java.io :as io]
            [clojure.tools.cli :refer [parse-opts]]
            [clojure.tools.logging :as log]))

;; ==================================================
;; Global variables
;; ==================================================

(def cli-options
  [[nil "--end END" "Ending moves"
   :default 350
   :parse-fn #(Integer/parseInt %)]
   ["-f" "--file FILE" "kifu file"
    :default "target.kif"]
   [nil "--start START" "Starting moves"
    :default 1
    :parse-fn #(Integer/parseInt %)]
   ["-h" "--help"]])

;; Interval to read a file in seconds
(def READ_FILE_WAIT_SECONDS 3)

;; Holds commandline options
(def opts (atom {}))

(def row-positions
  {"1" "a", "2" "b", "3" "c", "4" "d", "5" "e", "6" "f", "7" "g", "8" "h", "9" "i",
   "一" "a", "二" "b", "三" "c", "四" "d", "五" "e", "六" "f", "七" "g", "八" "h", "九" "i",
   1 "a", 2 "b", 3 "c", 4 "d", 5 "e", 6 "f", 7 "g", 8 "h", 9 "i"})
   
(def column-positions
  {"1" "1", "2" "2", "3" "3", "4" "4", "5" "5", "6" "6", "7" "7", "8" "8", "9" "9",
   "１" "1", "２" "2", "３" "3", "４" "4", "５" "5", "６" "6", "７" "7", "８" "8", "９" "9",
   1 "1", 2 "2", 3 "3", 4 "4", 5 "5", 6 "6", 7 "7", 8 "8", 9 "9"})

(def pieces
 {"歩" "p", "香" "l", "桂" "n", "銀" "s", "金" "g", 
  "角" "b", "馬" "+b", "飛" "r", "龍" "+r", "竜" "+r"})

;; ==================================================
;; Functions
;; ==================================================

(defn check-file []
  "Checks exisntance of a file. If a file is not found, throws an error"
  (let [file (:file @opts)]
    (when-not (.exists (io/as-file file))
      (log/fatal (str "Kifu file not found: " file))
      (throw (java.io.FileNotFoundException. file)))
    file))

(defn parse-line
  "Converts a normal move in the Kifu format to the USI format."
  [move-str from last-move]
  {:pre [(pos? (count move-str))]
   :post [(<= 4 (count %))]}
  (let [move (cond
               (= \同 (first move-str))
                 (str (get column-positions (str (subs from 0 1)))
                      (get row-positions    (str (subs from 1 2)))
                      (subs last-move 2 4))
               :else
                 (str (get column-positions (str (subs from 0 1)))
                      (get row-positions    (str (subs from 1 2)))
                      (get column-positions (str (subs move-str 0 1)))
                      (get row-positions    (str (subs move-str 1 2)))))]
   (cond
     (.contains ^String move-str "不成")
       move
     (re-find #"成$" move-str)
       (str move "+")
     :else
       move)))

(defn parse-line-drop
  "Converts a drop move in the Kifu format to the USI format."
  [move-str]
  {:pre [(.contains move-str "打")]}
  (str (str/upper-case (get pieces (subs move-str 2 3)))
       "*"
       (get column-positions (str (subs move-str 0 1)))
       (get row-positions    (str (subs move-str 1 2)))))

(defn parse-moves
  [lines]
  (loop [current-move 1
         lines        lines
         ret          []]
    (if-not (seq lines)
      ret ;; base
      (let [line (str/trim (first lines))]
        (cond
          (re-find #"^\*" line) ;; comment
            (recur current-move (rest lines) ret)
          (re-find #"^(\d+) (.*?)\((\d\d)\)" line)
            (let [[_0 _1 move-str from] (re-find #"^(\d+) (.*?)\((\d\d)\)" line)
                  move (parse-line move-str from (last ret))]
              (recur (inc current-move) (rest lines) (conj ret move)))
          (re-find #"^(\d+) (.*?打)" line)
            (let [[_0 _1 move-str] (re-find #"^(\d+) (.*?打)" line)
                  move (parse-line-drop move-str)]
              (recur (inc current-move) (rest lines) (conj ret move)))
          (re-find #"^(\d+) 投了" line)
            (conj ret "resign") ;; base
        :else
          (recur current-move (rest lines) ret))))))

(defn accumulate-moves
  "Accumelates moves and returns a sequnece of lines.
   For example, for [m1 m2 m3] it will return [\"m1\" \"m1 m2\" \"m1 m2 m3\"]"
  [moves]
  (reduce (fn [v m]
            (if (empty? v)
              (conj v m)
              (conj v (str (last v) " " m))))
          []
          (filter #(not= "resign" %) moves)))

(defn main-loop
  [nstart nend]
  (let [contents (slurp (check-file) :encoding "Shift_JIS")
        moves    (parse-moves (str/split-lines contents))]
    (log/debugf "Read %d/%d moves..." (count moves) nstart)
    (cond
      (<= nend (count moves))
        nil ;; base
      (< (count moves) nstart)
        (do
          (log/tracef "Too eary to start. Sleeping for %d seconds..." READ_FILE_WAIT_SECONDS)
          (Thread/sleep (* 5 READ_FILE_WAIT_SECONDS 1000))
          (recur nstart nend))
      :else
        (do
          (dorun (map #(println (str "position startpos moves " %))
                      (drop (dec nstart) (accumulate-moves moves))))
          (if (= "resign" (last moves))
            (do
              (log/trace "A player resigned. Finishing...")
              (println "resign")) ;; base
            (do
              (Thread/sleep (* READ_FILE_WAIT_SECONDS 1000))
              (recur (inc (count moves)) nend)))))))

(defn usage
  [options-summary]
  (->> ["USAGE: lein usi_convert [options]"
        ""
        "Options:"
        options-summary]
    (str/join \newline)))

(defn error-msg
  [errors]
  (str "The following errors occurred while parsing your command:\n\n"
    (str/join \newline errors)))

(defn exit
  [status msg]
  (println msg)
  (System/exit status))

(defn -main
  [& args]
  (let [{:keys [options arguments errors summary]} (parse-opts args cli-options)]
    (reset! opts options)
    (cond
      (:help @opts) (exit 0 (usage summary))
      errors        (exit 1 (error-msg errors)))
    (main-loop (:start @opts) (:end @opts))))

