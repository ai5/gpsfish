(ns gpsdashboard.benchmark
  (:require [gpsdashboard.time :as gt]
            [gpsdashboard.util :as util]
            [clojure.contrib.command-line :as cmd]
            [clojure.contrib.logging :as log])
  (:import [java.net DatagramSocket DatagramPacket InetAddress]
           [java.nio ByteBuffer]
           [java.nio.charset Charset])
  (:gen-class))

;; ==================================================
;; Global variables
;; ==================================================

;; ==================================================
;; Functions
;; ==================================================

(defn ^ByteBuffer string->buffer
  ([^String s]
    (string->buffer s (Charset/forName "US-ASCII")))
  ([^String s ^Charset charset]
    (.encode charset s)))

(defn send-message
  [msg-str ^DatagramSocket socket inet port]
  (let [buf       (-> (string->buffer msg-str) .array)
        packet    (DatagramPacket. ^bytes buf ^Integer (count buf) ^InetAddress inet ^Integer port)]
    (.send socket packet)))

(defn do-main
  [host port]
  (let [inet (InetAddress/getByName host)
        socket (DatagramSocket.)]
    (send-message "benchmark position startpos moves 2g2f 3c3d" socket inet port)
    (dotimes [i 50000]
      (let [msg-str   (format "macpro info time %d nodes %d nps %d" i i i)]
        (send-message msg-str socket inet port))
      (let [msg-str   (format "macpro info depth %d seldepth %d time %d score cp %d nodes %d nps %d pv pv1 pv2" 4 7 i (int (/ i 100)) i i)]
        (send-message msg-str socket inet port)))))

(defn -main [& args]
  (cmd/with-command-line args
    "GPSShogi Clustor Dashboard - Benchmark client"
    [[host               "Server host"  "localhost"]
     [port               "Server port"  4120]
     remaining]
     (do-main host (util/parseInt port))
     (shutdown-agents)))

