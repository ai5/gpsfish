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

(ns twitter_clj.engine_controler
  (:import (java.io InputStreamReader OutputStreamWriter))
  (:require [twitter_clj.common :as common]
            [twitter_clj.draw_image :as image]
            [twitter_clj.engine :as engine]
            [twitter_clj.ki2 :as ki2]
            [twitter_clj.snapshot :as snapshot]
            [twitter_clj.subprocess :as sub]
            [twitter_clj.twitter :as twitter]
            [clojure.core.async :as async]
            [clojure.string :as str]
            [clojure.tools.cli :refer [parse-opts]]
            [clojure.tools.logging :as log]))

;; ==================================================
;; Global variables
;; ==================================================

(def cli-options
  [[nil "--dry-run" "Do not tweet"]
   [nil "--force-think SECS" "Think at least n seconds for each move"
    :default 20
    :parse-fn #(Integer/parseInt %)]
   [nil "--force-update" "Update possile exisiting tweets"]
   [nil "--gpsusi PATH"  "Path to gpsusi"
    :default "../../gpsshogi/bin/gpsusi"]
   [nil "--gpsfish PATH" "Path to gpsfish (optional)"
    :default nil]
   ["-h" "--help"]
   [nil "--image-generator PATH" "Path to sampleImageGenerator (optional)"
    :default nil]])

;; A channel to notify the Monitor thread to get ready for the next move.
(def monitor-reset-channel (async/chan (async/sliding-buffer 1)))

(def start-time (atom nil))

;; ==================================================
;; Functions
;; ==================================================

(defmacro parse-integer
  [key-str ret xs]
  `(let [symbl# (keyword ~key-str)
         ret# ~ret
         xs#  ~xs]
     (recur (merge ret# {symbl# (Long/parseLong (second xs#))}) (rest (rest xs#)))))

(defn- get-first-move
  [pv]
  (first (str/split pv #"\s")))

(defn- append-first-move
  "Append a map col with :first-move, which is the first move in :pv."
  [col]
  (assert (:pv col))
  (let [pv (:pv col)]
    (assoc col :first-move (get-first-move pv))))

(defn- parse-info-depth-message
  "Parse an info depth message and return a corresponding map with keys:
  :info, :depth, :seldepth, :score, :cp, :nodes, :nps, :time :pv"
  [msg]
  (loop [ret {}
         xs  (str/split msg #"\s")]
    (if-not (seq xs)
      ret
      (condp = (first xs)
        "info"     (recur ret (rest xs))
        "depth"    (parse-integer "depth"    ret xs)
        "seldepth" (parse-integer "seldepth" ret xs)
        "score"    (recur ret (rest xs))
        "cp"       (parse-integer "cp"       ret xs) ; relative value
        "nodes"    (parse-integer "nodes"    ret xs)
        "nps"      (parse-integer "nps"      ret xs)
        "time"     (parse-integer "time"     ret xs)
        "pv"       (recur (merge ret {:pv (str/join " " (rest xs))})
                          nil)
        (log/errorf "Unknown attributes for InfoDepthMessage: %s" xs)))))

(defn- absolute-cp-value
  "If it is White's tern, negiate the cp value, keeping an orvinal cp as
  :relative-cp."
  [nthmove col]
  (let [col (assoc col :relative-cp (:cp col))]
    (if (odd? nthmove)
      col ; the current turn is Black
      (update-in col [:cp] * -1))))

(defn parse-line
  "Parse a raw line that gpsusi produces. 'cp' is adjusted to an abosulte
  evaluation value."
  [nthmove message]
  (try
    (condp (comp seq re-seq) message
      #"^info depth (\d+)$" nil
      #"^info depth (\d+) " (->> message
                                parse-info-depth-message
                                (absolute-cp-value nthmove)
                                append-first-move)
      nil)
    (catch Exception e
      (log/error (str e "\nFailed to parse message: " message))
      nil)))

(defn start-monitor
  "Start a monitor thread reading what gpsusi produces. The turn is a player of nth-move."
  [stdin nthmove position]
  (log/debug "Starting a new monitor...")
  (reset! start-time (java.util.Date.))
  (future
    (reset! common/pv [])
    (loop [lines (line-seq stdin)]
      (if-let [line (first lines)]
        (condp (comp seq re-seq) line
          #"warning stop ignored" (common/sort-pv @common/pv)  ; base
          #"bestmove (.*)"        :>> #(if (empty? @common/pv) ; base
                                         (reset! common/pv [{:cp 0 :pv (-> % first (nth 1))}])
                                         (common/sort-pv @common/pv))
          (do
            (if-let [m (parse-line nthmove line)]  ; else
              (let [m             (assoc m :position position) ; append :position
                    current-depth (or (:depth (first @common/pv)) 0)
                    depth         (:depth m)]
                ; m is a map with keys: (a) keys in USI messages (:info, :depth, :seldepth,
                ; :score, :cp, :nodes, :nps, :time and :pv) and (b) additional keys
                ; (:first-move, :relative-cp and :position)
                (if (< current-depth depth)
                  (do
                    (log/debug (format "PV deepened: %s" m))
                    (reset! common/pv [m]))
                  (do
                    (log/debugf "Appended a new PV: size %d, first move %s, relative cp %s"
                                (count (str/split (:pv m) #"\s"))
                                (first (str/split (:pv m) #"\s"))
                                (:relative-cp m))
                    (swap! common/pv conj m)))))
            (recur (rest lines))))))))

(defn resign?
  [line]
  (= "resign" line))

(defn stop-monitor
  "Stop a monitor (which is a future)."
  [stdout monitor]
  {:pre [(not (nil? monitor))]}
  (sub/write stdout "stop")
  (log/debug "waiting monitor")
  (log/debug (format "Monitor finished: %s" @monitor)))

(defn usage
  [options-summary]
  (->> ["USAGE: lein engine_controler [options]"
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

(defn force-think
  "If --forth-think is set, let the engine think for a while."
  []
  (if (and (pos? (:force-think @common/options)) @start-time)
    (let [interval (:force-think @common/options)
          consumed (snapshot/interval-seconds @start-time)]
      (when (< consumed interval)
        (log/debugf "Force to think for %d secs (more %d secs)..." interval (- interval consumed)) 
        (Thread/sleep (* 1000 (- interval consumed)))))))

(defn start-engine
  "Starts a USI thinking eninge and returns [proc stdin stdout stderr]"
  []
  (let [gpsusi-cmd (if (:gpsfish @common/options)
                     (:gpsfish @common/options)
                     (str (:gpsusi @common/options) " --extended-usi 1 -N 7"))] ;; extended mode
    (engine/start-engine gpsusi-cmd))) ;; thinking engine

(defn -main
  [& args]
  (let [{:keys [options arguments errors summary]} (parse-opts args cli-options)]
    (reset! common/options options)
    (cond
      (:help @common/options) (exit 0 (usage summary))
      errors                  (exit 1 (error-msg errors)))
    (let [[proc stdin stdout stderr] (start-engine)]
      (ki2/start-ki2-engine (:gpsusi @common/options)) ;; normal mode
      (twitter/post-version (:id-name @common/options))
      (twitter/post-title)
      ; Now ready to read positions
      (loop [monitor         nil
             snapshot-thread nil
             dummy           nil
             lines           (line-seq (java.io.BufferedReader. *in*))]
        (if (seq lines) ; the next move has come
          (let [line (first lines)]
            (if (resign? line)
              (log/infof ">>> %s" line)
              (log/infof ">>> [%d] %s" (common/get-nmove line) line))
            (when snapshot-thread
              (force-think)
              (snapshot/interrupt)
              (async/<!! snapshot-thread))   ;; snapshot has finished
            (when monitor
              (stop-monitor stdout monitor)) ;; monitor has finished
            (if (resign? line)
              (recur nil nil dummy nil)      ;; Will quit at the next loop
              (if (and (twitter/moves-file-exists? (common/get-nmove line))
                       (not (:force-update @common/options)))
                (do
                  (log/info "Found a twitter log file for this move. Skip it.")
                  (recur nil nil dummy (rest lines)))
                (do
                  (log/debug "Sending the move to the engine...")
                  (if-not (ki2/is-valid-position line)
                    (do
                      (log/warnf "Read an invalid position. Skip it: %s" line)
                      (recur nil nil dummy (rest lines)))
                    (do ; think this line/position until the next line comes
                      (ki2/set-position line)
                      (sub/write stdout line)
                      (if (:image-generator @common/options)
                        (image/draw (:image-generator @common/options)
                                    (common/get-nmove line) line))
                      (recur (start-monitor stdin (inc (common/get-nmove line)) line)
                             (snapshot/start-snapshot-thread)
                             (sub/write stdout "go infinite")
                             (rest lines))))))))
          (do ; else
            (log/info "Finished reading.")
            (when snapshot-thread
              (snapshot/interrupt))
            (when monitor
              (stop-monitor stdout monitor))
            nil))) ; end loop
      (ki2/stop-ki2-engine)
      (engine/stop-engine proc stdin stdout stderr))
    (shutdown-agents)))

