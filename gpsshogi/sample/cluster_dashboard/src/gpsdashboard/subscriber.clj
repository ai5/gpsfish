(ns gpsdashboard.subscriber
  (:require [gpsdashboard.common :as common]
            [clojure.data.json :as json]
            [clojure.contrib.logging :as log])
  (:use gpsdashboard.server_protocol)
  (:import [java.io ByteArrayInputStream InputStreamReader BufferedInputStream DataInputStream
                    IOException]
           [java.util.concurrent Executors ExecutorService RejectedExecutionException]
           [java.util.zip GZIPInputStream]
           [java.net Socket InetSocketAddress SocketException SocketTimeoutException]))


(def CONNECTION_TIMEOUT_SEC 5)


(defn parse-data
  "Deserialize compressed data that is received from the server"
  [data]
  (if (nil? data)
    nil
    (let [reader (-> (ByteArrayInputStream. data)
                     (GZIPInputStream.)
                     (InputStreamReader. "UTF-8"))]
      (json/read reader :key-fn keyword :eof-error? false :eof-value ""))))

(defn read-data
  "Receives data from socket, then call a callback function with the data"
  [^ExecutorService executor ^Socket socket callback]
  (letfn [(read-buffer [^DataInputStream in size]
            (when size
              (log/trace (format "Reading %d bytes..." size))
              (try
                (let [buf (byte-array size)]
                  (.readFully in buf)
                  buf)
                (catch SocketException se
                  (log/error (str "Error in subscriber: " (.getMessage se)))
                  nil))))
          (read-int [^DataInputStream in]
            (try
              (.readInt in)
              (catch RuntimeException re
                (log/error (str "Error in subscriber: " (.getMessage re)))
                nil)))
          (get-input-stream []
            (-> (.getInputStream socket)
                (BufferedInputStream.)
                (DataInputStream.)))
          (connection-alive? []
            (every? false? [(.isShutdown executor) (.isClosed socket)]))]
    (assert (.isConnected socket))
    (let [in (get-input-stream)]
      (loop []
        (when (and (connection-alive?)
                 (common/delimiter? (read-buffer in (count common/DELIMITER)))
                 (->> (read-int in)
                      (read-buffer in)
                      (parse-data)
                      (callback)))
          (recur))))))

(defn socket-connect
  "Create a scoket to connect to host:port, then return the socket"
  [^String host ^Integer port]
  {:pre [(not-empty host) (pos? port)]}
  (let [socket (Socket.)]
    (try
      (.connect socket (InetSocketAddress. host port) (* 1000 CONNECTION_TIMEOUT_SEC))
      (catch IOException ie
        (log/error (format "Failed to connect to %s:%d: %s"
                           host port (.getMessage ie))))
      (catch SocketTimeoutException ste
        (log/error (format "Timed out to connect to %s:%d: %s"
                           host port (.getMessage ste)))))
    socket))

(defn new-subscriber
  "Return a new subscriber object, which subscribes messages"
  [f {:keys [host port] :or {host "localhost" port 8240}}]
  (let [executor (Executors/newSingleThreadExecutor)
        socket   ^Socket (socket-connect host port)]
    (when (.isConnected socket)
      (reify ServerProtocol ; return an object
        (start [self]
          (.execute executor #(read-data executor socket f)))
        (ready? [self]
          (and (not (.isShutdown executor))
               (not (.isTerminated executor))))
        (join [_]
          (loop []
            (when (every? false? [(.isClosed socket) (.isShutdown executor)])
              (Thread/sleep (* 5 1000))
              (recur))))
        (shutdown [self]
          (.shutdown executor)
          (.close socket)
          (.awaitTermination executor 1000 java.util.concurrent.TimeUnit/MILLISECONDS))))))
