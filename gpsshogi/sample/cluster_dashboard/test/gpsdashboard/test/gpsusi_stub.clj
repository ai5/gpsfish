(ns gpsdashboard.test.gpsusi_stub
  (:import [java.net DatagramSocket DatagramPacket InetAddress]))


(defn send-info-time-message
  [port time nodes nps]
  (let [localhost (InetAddress/getByName "localhost")
        msg-str (format "macpro info time %d nodes %d nps %d" time nodes nps)
        msg-bytes (.getBytes msg-str)
        packet (DatagramPacket. msg-bytes (count msg-bytes) localhost port)
        socket (DatagramSocket.)]
    (.send socket packet)))
