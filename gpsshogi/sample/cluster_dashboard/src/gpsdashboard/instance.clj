(ns gpsdashboard.instance
  (:require [gpsdashboard.dump :as dump]
            [gpsdashboard.message :as message]
            [gpsdashboard.time :as gt]
            [clojure.contrib.logging :as log])
  (:import  [gpsdashboard.message GoIdMessage InfoTimeMessage StartposMessage
                                  InfoDepthMessage InfoStartNewMasterPositionMessage
                                  IgnoreMovesMessage]))


;; ==================================================
;; Global variables
;; ==================================================

;; {ip1 Instance1, ip2 Instance2, ...}
(def instances (atom {}))


;; ==================================================
;; Functions
;; ==================================================
;;
(declare filter-fresh-data)

(defrecord Instance [ip host updated ; basic
                     nps moves-str ; info time
                     depth seldepth score pv-str ; info depth
                     ignore-moves-str ; ignore_moves
                     ])

(defn new-instance
  "Constructor of Instance"
  ([ip host]
   (new-instance ip host {}))
  ([ip host ops]
   (let [{:keys [updated
                 nps moves-str
                 depth seldepth score pv-str
                 ignore-moves-str]
          :or {updated (gt/now-long)
               nps (int 0) moves-str ""
               depth (int 0) seldepth (int 0) score (int 0) pv-str ""
               ignore-moves-str ""}} ops]
     (Instance. (long ip) host (long updated)
                (int nps) moves-str
                (int depth) (int seldepth) (int score) pv-str
                ignore-moves-str))))

(defn reset-instances
  []
  (reset! instances {}))

(defmulti add-instance
  (fn [msg] (type msg)))

(defmethod add-instance StartposMessage
  [{:keys [ip host moves-str]}]
  "Reset a new instance for a specifyed IP.
   Only NPS is carried over from the previous one."
  (assert (not-empty moves-str))
  (let [nps      (or (get-in @instances [host :nps]) 0)
        instance (new-instance ip host {:nps nps :moves-str moves-str})]
    (swap! instances assoc host instance)))

(defmethod add-instance IgnoreMovesMessage
  [{:keys [ip host ignore-moves-str]}]
  (assert (not-empty ignore-moves-str))
  (if (get @instances host)
    (swap! instances update-in [host] #(merge % {:updated (gt/now-long)
                                               :ignore-moves-str ignore-moves-str}))
    (log/warn (str "Found no instance for host: " host))))

(defmethod add-instance InfoTimeMessage
  [{:keys [ip host nps]}]
  (assert (<= 0 nps))
  (if (get @instances host)
    (swap! instances update-in [host] #(merge % {:updated (gt/now-long) :nps nps}))
    (log/warn (str "Found no instance for host: " host))))

(defmethod add-instance InfoDepthMessage
  [{:keys [ip host depth seldepth score nps pv-str]}]
  (assert (<= 0 nps))
  (if (get @instances host)
    (swap! instances update-in [host] #(merge % {:updated (gt/now-long) :depth depth :seldepth seldepth
                                               :score score :nps nps :pv-str pv-str}))
    (log/warn (str "Found no instance for host: " host))))

(defmethod add-instance InfoStartNewMasterPositionMessage
  [{:keys [ip host moves-str]}]
  (log/info (str "INFO START NEW MASTER POSITION: " moves-str))
  (let [instance (new-instance ip host {:moves-str moves-str})]
    (swap! instances assoc host instance)))

(defmethod add-instance GoIdMessage
  [{:keys [ip host msg]}]
  (let [expiration-sec 60
        since (gt/now-long-before-sec expiration-sec)
        cols  (filter-fresh-data since)
        s     (format "server go id %s" msg)]
    (dump/dump cols)
    (reset-instances)
    (dump/write-log s)))

;; ==================================================
;; Collecting raw data
;; ==================================================

(defn filter-fresh-data
  "Return sequence of instances that are updated
   since the since (epoch in milliseconds) time"
  [since]
  {:pre [(<= 0 since)]}
  (letfn [(newer-than? [instance]
            (<= since (:updated instance)))]
    (if (empty? @instances)
      []
      (filter newer-than? (vals @instances)))))

(defn count-unique-ips
  [cols]
  (count cols))

(defn count-nps
  [cols]
  (apply + (map :nps cols)))

