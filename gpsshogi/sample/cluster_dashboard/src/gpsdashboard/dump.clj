(ns gpsdashboard.dump
  (:require [gpsdashboard.time :as gt]
            [clojure.java.io :as io]
            [clojure.data.json :as json]
            [clojure.contrib.logging :as log])
  (:import [java.io Writer]))


;; ==================================================
;; Global variables
;; ==================================================

(def dump-file
  (let [a (agent nil)]
    (set-error-mode! a :continue)
    (set-error-handler! a (fn [agt ex] (log/error (str "Error in the dump agent: " ex))))
    a))

;; ==================================================
;; Functions
;; ==================================================

(defn write-log
  "Write a body string to the bump log file."
  [body]
  (letfn [(construct-line [s]
            (format "%s %s\n" (gt/now-string) s))] 
    (let [file-name "dump.log"
          ^String line (construct-line body)]
      (send-off dump-file
        (fn [agt]
          (with-open [^Writer w (io/writer file-name :append true)]
            (.write w line))
          true)))))

(defn dump
  "Write the current state cols to the dump log file."
  [cols]
  (let [s (json/write-str cols)]
    (write-log s)))


