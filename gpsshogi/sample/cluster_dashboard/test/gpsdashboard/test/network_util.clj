(ns gpsdashboard.test.packet
  (:use gpsdashboard.network_util
        midje.sweet))

(fact "test-bytes-to-long"
  (let [localhost (+ (* 127 256 256 256) 1)
        localhost-bytes (-> (BigInteger/valueOf localhost)
                            (.toByteArray))]
    (networkbytes-to-long localhost-bytes) => localhost))

(fact "test-long-to-ip-string"
  (let [localhost (+ (* 127 256 256 256) 1)]
    (long-to-ip-string localhost) => "127.0.0.1"))

