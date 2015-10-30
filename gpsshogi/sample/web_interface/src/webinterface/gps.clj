(ns webinterface.gps
  (:require [clojure.java.io :as io]
            [clojure.string :as str]
            [clj-time.format :as tf]
            [clj-time.local :as tl]
            [clojure.tools.logging :as log]))

; There is at most a single GPS process for this Web application.
; If there already exists a running process when you try to spawn a new one,
; the current one is destroyed before the new one is created.
;
; Note that since this variable resides in memory it will be cleared upon
; restarting Jetty. In such cases, the administrator should manually kill a
; running process if any.
(def ^:dynamic *pid* (atom nil))


(defn get-pid
  "Return PID of a Process instance. This is kind of a hack because it
  dependes on internal implementation of a concrete class of Process."
  [^java.lang.Process process]
  (if (= (-> process .getClass .getName)
         "java.lang.UNIXProcess")
    (let [f (-> process .getClass (.getDeclaredField "pid"))]
      (.setAccessible f true)
      (.getInt f process))
    -1))

(defn generate-regular-game-name
  "Create a new regular game name."
  [thinktime byoyomi]
  (let [custom-formatter (tf/formatter "yyyyMMddHHmmss")
        now (tf/unparse custom-formatter (tl/local-now))]
    (format "ab_%s-%d-%d" now thinktime byoyomi)))

(defn generate-buoy-game-name
  "Create a new buoy game name."
  [thinktime byoyomi]
  (let [custom-formatter (tf/formatter "yyyyMMddHHmmss")
        now (tf/unparse custom-formatter (tl/local-now))]
    (format "buoy_%s-%d-%d" now thinktime byoyomi)))

(defn ps-gps
  "Return a sequence of PIDs of gps. Note that PIDs are String."
  []
  (for [line (-> (Runtime/getRuntime)
               (.exec "ps ax")
               (.getInputStream)
               (io/reader)
               (line-seq))
        :let [cols    (str/split (str/trim line) #"\s+")
              command (str/join " " (drop 4 cols))]
        :when (re-find #"csa2usi\.pl" command)]
    (do
      ;(log/info line)
      (Integer. (first cols)))))

(defn gps-running?
  "Return true if a GPS process is running; otherwise, false."
  []
  (not (empty? (ps-gps))))

(defn kill-gps
  "Terminate GPS process(es)."
  ([]
   (dorun
     (map kill-gps (ps-gps))))
  ([pid]
   (log/info (format "Killing the GPS process: %d" pid))
   (let [process (-> (ProcessBuilder. ["kill" "-KILL" (str pid)])
                   .start)]
     (.waitFor process)
     (doseq [getStream [#(.getInputStream %) #(.getErrorStream %)]]
       (doseq [line (-> process
                      getStream
                      io/reader
                      line-seq)]
         (log/info line))))))

(defn spawn-gps
  "Spawn a GPS Shogi process and return a map of :out, :err, :in and :future."
  [buoyname thinktime byoyomi human-turn]
  (log/info (format "Spawning a new GPS process for %s" buoyname))
  ;;; 1. If there already exists a running process, kill it first.
  (when (gps-running?)
    (log/warn (format "A GPS Shogi process is still running: %d" @*pid*))
    (kill-gps)
    (Thread/sleep (* 2 1000))
    (if (gps-running?)
      (throw (Exception. "Failed to stop GPS process."))))
  ;;; 2. Spawn a new GPS process
  (let [bw  (if (= human-turn "black") "W" "B")
        cmd ["/home/gps/bin/wrapper.sh" ;; use a wrapper script to detach a child process
            "/home/gps/up4.9/gpsshogi/sample/perl-cluster/csa2usi.pl"
            "--usi_engine"
            "/home/gps/up4.9/gpsfish_dev/src/gpsfishone"
            "--csa_id=AlphaBetaNew"
            "--logdir=/home/gps/up4.9/gpsshogi/sample/perl-cluster/log"
            (format "--csa_pw=%s-%s" buoyname bw)
            (format "--sec_limit=%d"    (max 0 (- thinktime 3)))
            (format "--sec_limit_up=%d" (max 0 (- byoyomi 3)))
            "--startup_usi=\"setoption name Hash value 8192\""]
        process (-> (ProcessBuilder. cmd)
                  (.directory (io/file "/home/gps/up4.9/gpsshogi/sample/perl-cluster"))
                  .start)]
    (reset! *pid* (get-pid process))
    (log/info (format "  process started: %d; %s" @*pid*, cmd))
    {:out (-> process
            .getInputStream
            io/reader)
    :err (-> process
            .getErrorStream
            io/reader)
    :in (-> process
          .getOutputStream
          io/writer)
    :future (future
              (try
                (.waitFor process)
                (finally
                  (log/info (format "GPS wrapper process finished: %d" @*pid*))
                  (doseq [line (line-seq (-> process .getInputStream io/reader))]
                    (log/info line))
                  (doseq [line (line-seq (-> process .getErrorStream io/reader))]
                    (log/error line))
                  (reset! *pid* nil))))}))

