(ns gpsdashboard.publisher
  (:require [gpsdashboard.common :as common]
            [gpsdashboard.instance :as instance]
            [gpsdashboard.time :as gt]
            [clojure.data.json :as json]
            [clojure.contrib.logging :as log])
  (:use gpsdashboard.server_protocol)
  (:import [java.io ByteArrayOutputStream OutputStreamWriter PrintWriter DataOutputStream BufferedOutputStream]
           [java.util.zip GZIPOutputStream]
           [java.util.concurrent Executors ExecutorService RejectedExecutionException]
           [java.net ServerSocket Socket SocketException]))


;; ==================================================
;; Global variables
;; ==================================================

(def MAX_THREDS 10)


;; ==================================================
;; Functions
;; ==================================================

(defn obj-to-byte-array
  "Serialize and compress an object by using Java's Serialize."
  [obj]
  (with-open [baos   (ByteArrayOutputStream.)]
    (with-open [writer (-> (GZIPOutputStream. baos)
                           (OutputStreamWriter. "UTF-8")
                           (PrintWriter.))]
      (json/write obj writer :escape-unicode false)
      (.close writer)) ; required
    (.toByteArray baos)))

(defn get-data
  [since]
  (let [cols (instance/filter-fresh-data since)]
    (obj-to-byte-array cols)))

(defn publish
  "Send data to a client socket"
  [#^Socket socket data]
  {:pre [(not-empty data)]}
  (when-not (.isClosed socket)
    (let [out (-> (.getOutputStream socket)
                  (BufferedOutputStream.)
                  (DataOutputStream.))]
      (doto out
        (.write    common/DELIMITER 0 (count common/DELIMITER))
        (.writeInt (int (count data)))
        (.write    data 0 (count data))
        (.flush)))))

(defn handle-client
  [#^Socket client interval-sec expiration-sec]
  (log/info "A client connected")
  (try
    (loop []
      (when-not (.isClosed client)
        (let [data (get-data (gt/now-long-before-sec expiration-sec))]
          (log/trace (format "Sending %d bytes..." (count data)))
          (publish client data)
          (log/trace (format "Sent %d bytes." (count data))))
        (Thread/sleep (* 1000 interval-sec))
        (recur)))
    (catch Exception e
      (log/info (str "Socket closed by the client peer." (.getMessage e))))
    (finally
      (log/info "A client disconnected"))))

(defn #^Socket accept
  "Accept clients connecting to a server socket."
  [#^ServerSocket socket]
  (try
    (.accept socket)
    (catch SocketException se
      (log/error (.getMessage se))
      nil)))

(defn new-publisher
  "Return a new publisher object, which publishes data to clients."
  [{:keys [port interval-sec expiration-sec] :or {port 8240 interval-sec 3 expiration-sec 30}}]
  {:pre [(pos? port)]}
   (letfn [(create-thread [#^ExecutorService executor #^ServerSocket socket]
             (Thread.
               (fn []
                 (log/info (format "Starting to publish messages newer than %d seconds from port %d each %d seconds."
                                   expiration-sec port interval-sec))
                 (try
                   (loop []
                     (when-not (.isShutdown executor)
                       (when-let [client (accept socket)]
                         (.execute executor #(handle-client client interval-sec expiration-sec))
                         (recur))))
                   (catch RejectedExecutionException ignored)
                   (finally
                     (log/info "Stopping the publisher"))))))]
     (let [executor (Executors/newFixedThreadPool MAX_THREDS)
           socket (ServerSocket. port)
           #^Thread thread (create-thread executor socket)]
       (reify ServerProtocol ; return an object
         (start [self]
           (.start thread))
         (ready? [self]
           (.isAlive thread))
         (join [_]
           (.join thread))
         (shutdown [self]
           (.shutdown executor)
           (.close socket)
           (.awaitTermination executor 1000 java.util.concurrent.TimeUnit/MILLISECONDS)
           (.join thread))))))

