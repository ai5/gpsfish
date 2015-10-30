(ns gpsdashboard.server
  (:require [gpsdashboard.instance :as instance]
            [gpsdashboard.message :as message]
            [gpsdashboard.network_util :as net]
            [gpsdashboard.packet :as packet]
            [clojure.contrib.logging :as log])
  (:use gpsdashboard.server_protocol)
  (:import [java.util.concurrent Executor Executors]
           [java.net DatagramSocket InetSocketAddress]
           [java.nio ByteBuffer CharBuffer]
           [java.nio.charset Charset]
           [java.nio.channels AsynchronousCloseException DatagramChannel]))


;; ==================================================
;; Global variables
;; ==================================================

(def default-server-config
  {:port 4120
   :reuse-address true
   :receive-buffer-size (* 32 1024 1024) ; 32MB
   })


;; ==================================================
;; Functions
;; ==================================================

(defn buffer->string
  ([^ByteBuffer byte-buffer]
    (buffer->string byte-buffer (Charset/forName "US-ASCII")))
  ([^ByteBuffer byte-buffer ^Charset charset]
    (.toString (.decode charset byte-buffer))))

(defn handle-message
  "Handle messages."
  [msg]
  (if-let [m (message/parse-message msg)]
    (do
      (packet/add-packet)
      (instance/add-instance m))))

(defn extractMessageFromUDP
  "Receive a UDP packet and form a message in a map."
  [^InetSocketAddress socket ^ByteBuffer buf]
  (let [ip  (net/networkbytes-to-long (-> socket .getAddress .getAddress))
        msg (buffer->string buf)]
    {:ip ip :message msg}))

(defn log-config
  [config]
  "Log configuration details."
  (log/info (str "Server configuration: " config)))

(defn create-datagram-channel
  "Create a new Datagram socket and return it."
  [{:keys [port reuse-address receive-buffer-size]}]
  (let [inet (InetSocketAddress. port)
        channel (DatagramChannel/open)]
    (doto (.socket channel)
      (.setReuseAddress reuse-address)
      (.setReceiveBufferSize receive-buffer-size)
      (.setBroadcast false)
      (.bind inet))
    (.configureBlocking channel true)
    channel))

(defn process-channel
  [^Executor executor ^DatagramChannel channel]
  (let [buf (ByteBuffer/allocateDirect (* 2 1024))] ; 2KB
    (loop []
      (when (.isOpen channel)
        (let [sock (.receive channel (.clear buf))]
          (cond
            (not (.hasRemaining buf)) (log/warn (str "Ignored too large packet: " (buffer->string buf)))
            (zero? (-> buf
                     .flip
                     .limit))         (log/warn "Failed to read packet.")
            :else                     (let [msg (extractMessageFromUDP sock buf)]
                                        (.execute executor (fn [] (handle-message msg))))))
        (recur)))))

(defn new-server
  "Return a new server object, which receives UDP messages from gpsusi."
  [config]
  (let [config (merge default-server-config config)]
    (log-config config)
    (letfn [(create-thread [^DatagramChannel channel]
              (Thread.
                (fn []
                  (log/info (format "Listening port... %d" (:port config)))
                  (try
                    (let [executor (Executors/newSingleThreadExecutor)]
                      (process-channel executor channel))
                    (catch AsynchronousCloseException ae
                      (log/info "Socket closed by the peer"))
                    (catch Exception se
                      (log/error (.getMessage se)))
                    (finally
                      (log/info "Stopping the server...")
                      (if (.isOpen channel)
                        (.close channel)))))))]
      (let [^DatagramChannel channel (create-datagram-channel config)
            ^Thread thread (create-thread channel)]
        (reify ServerProtocol ; return an object
          (start [self]
            (.start thread))
          (ready? [self]
            (.isBound (.socket channel)))
          (join [_]
            (.join thread))
          (shutdown  [self]
            (.interrupt thread)
            (.join thread)))))))

