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

(ns webinterface.kifToCsa
  (:require [clojure.string :as str]
            [clojure.java.io :as io]
            [clojure.tools.cli :refer [parse-opts]]
            [clojure.tools.logging :as log]))

;; ==================================================
;; Global variables
;; ==================================================

(def cli-options
  [["-f" "--file FILE" "kifu file"
    :default "target.kif"]
   ["-h" "--help"]])

;; Holds commandline options
(def opts (atom {}))

;; Mapping of row representations from Kif's stlye to CSA's
(def row-positions
  {"1" "1", "2" "2", "3" "3" "4" "4", "5" "5", "6" "6", "7" "7", "8" "8", "9" "9",
   "一" "1", "二" "2", "三" "3", "四" "4", "五" "5", "六" "6", "七" "7", "八" "8", "九" "9",
   1 "1", 2 "2", 3 "3", 4 "4", 5 "5", 6 "6", 7 "7", 8 "8", 9 "9"})
   
;; Mapping of column representations from Kif's style  to CSA's
(def column-positions
  {"1" "1", "2" "2", "3" "3", "4" "4", "5" "5", "6" "6", "7" "7", "8" "8", "9" "9",
   "１" "1", "２" "2", "３" "3", "４" "4", "５" "5", "６" "6", "７" "7", "８" "8", "９" "9",
   1 "1", 2 "2", 3 "3", 4 "4", 5 "5", 6 "6", 7 "7", 8 "8", 9 "9"})

;; Mapping of piece representations from Kif's style to CSA's
(def pieces
 {"歩" "FU", "香" "KY", "桂" "KE", "銀" "GI", "金" "KI", "と" "TO",
  "角" "KA", "馬" "UM", "飛" "HI", "龍" "RY", "竜" "RY",
  "王" "OU", "玉" "OU"})

;; Mapping of normal pieces to promoted ones.
(def promote
  {"FU" "TO", "KY" "NY", "KE" "NK", "GI" "NG", "KA" "UM", "HI" "RY"})

;; ==================================================
;; Functions
;; ==================================================

(defn check-file
  "Checks exisntance of a file. If a file is not found, throws an error"
  []
  (let [file (:file @opts)]
    (when-not (.exists (io/as-file file))
      (log/fatal (str "Kifu file not found: " file))
      (throw (java.io.FileNotFoundException. file)))
    file))

(defn remove-japanese-white-space
  "Removes full-width white spaces."
  [s]
  (apply str (remove #(= \　 %) (seq s))))

(defn parse-piece
  "Parses a piece representation, taking (un)promotion into account."
  [s]
  (if (= (first s) \成)
    (promote (get pieces (subs s 1 2)))  ; prmoted piece
    (let [p (get pieces (subs s 0 1))]
      (if (<= 0 (.indexOf s "不成"))
        p
        (if (<= 0 (.indexOf s "成"))
          (get promote p)                ; promoting move
          p)))))                         ; unpromoting move

(defn parse-line
  "Converts a normal move in the Kifu format to the USI format."
  [move-str from last-move]
  {:pre [(pos? (count move-str))]
   :post [(<= 4 (count %))]}
  (let [move-str (remove-japanese-white-space move-str)
        ;_ (prn move-str)
        [move p] (cond
                   (= \同 (first move-str))
                     [(str (get column-positions (subs from 0 1))
                           (get row-positions    (subs from 1 2))
                           (subs last-move 3 5))
                      (parse-piece (subs move-str 1))]
                   :else
                     [(str (get column-positions (subs from 0 1))
                           (get row-positions    (subs from 1 2))
                           (get column-positions (subs move-str 0 1))
                           (get row-positions    (subs move-str 1 2)))
                      (parse-piece (subs move-str 2))])]
    (str move p)))

(defn parse-line-drop
  "Converts a drop move in the Kifu format to the USI format."
  [move-str]
  (str "00"
       (get column-positions (subs move-str 0 1))
       (get row-positions    (subs move-str 1 2))
       (get pieces (subs move-str 2 3))))

(defn parse-moves
  [lines]
  (loop [current-move 1
         lines        lines
         ret          []]
    (if-not (seq lines)
      ret ;; base
      (let [line (str/trim (first lines))
            turn (if (odd? current-move) "+" "-")]
        (cond
          (re-find #"^\*" line) ;; comment
            (recur current-move (rest lines) ret)
          (re-find #"^(\d+) (.*?)\((\d\d)\)" line)
            (let [[_0 _1 move-str from] (re-find #"^(\d+) (.*?)\((\d\d)\)" line)
                  move (parse-line move-str from (last ret))]
              (recur (inc current-move) (rest lines) (conj ret (str turn move))))
          (re-find #"^(\d+) (.*?打)" line)
            (let [[_0 _1 move-str] (re-find #"^(\d+) (.*?)打" line)
                  move (parse-line-drop move-str)]
              (recur (inc current-move) (rest lines) (conj ret (str turn move))))
          (re-find #"^(\d+) 投了" line)
            (conj ret "%TORYO") ;; base
        :else
          (recur current-move (rest lines) ret))))))

(defn convert-kifu
  "Converts a kifu file in ki2 format into moves in CSA format by using an external command.
   Retruns a sequence of moves in CSA format."
  [kifufile]
  (let [contents (slurp kifufile :encoding "Shift_JIS")]
    (parse-moves (str/split-lines contents))))

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
    (-> (check-file)
      convert-kifu
      println)))

