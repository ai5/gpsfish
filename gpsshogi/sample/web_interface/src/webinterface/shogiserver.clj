(ns webinterface.shogiserver
  (:import [java.io PrintWriter]
           [java.net Socket])
  (:require [clojure.java.io :as io]
            [clojure.string :as str]
            [clojure.tools.logging :as log]
            [environ.core :as env]))

(def *wdoor-port* 4081)

(defn- writeln
  "Write a line to out."
  [out msg]
  (.println out msg))

(defn- readln
  "Read a specified number (default 1) of lines from in."
  ([in]
   (read in 1))
  ([in i]
   (dotimes [_ i]
     (.readLine in))))

(defn set-specified-position
  "Register a new bouy game in the Shogi-server."
  [turn moves buoy-game-name]
  (log/infof "Setting a buoy game %s: turn %s, moves: %s"
              buoy-game-name
              turn
              (str/join " " moves))
  (with-open [socket (Socket. (env/env :wdoor-ip) *wdoor-port*)
              in     (-> socket
                       .getInputStream
                       io/reader)
              out    (-> socket
                       .getOutputStream
                       io/writer
                       (PrintWriter. true))]
    (writeln out (format "LOGIN %s %s x1" (env/env :shogiserver-user)
                                          (env/env :shogiserver-password)))
    (readln in 2)
    (writeln out (format "%%%%SETBUOY %s %s" buoy-game-name (str/join "" moves)))
    (readln in 1)
    (writeln out "LOGOUT")))

