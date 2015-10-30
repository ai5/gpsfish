(ns ^{:doc "Utility functions related with network things"}
  gpsdashboard.network_util
  (:require [clojure.string :as str]))


(defn networkbytes-to-long
  "Convert a byte array of network byte order into a long"
  [#^bytes network-bytes]
  {:pre [(= 4 (count network-bytes))]}
  (.longValue (BigInteger. network-bytes))) 

(defn long-to-ip-string
  "Convert a long into a text representation of IP address"
  [network-int]
  (let [bs (-> (BigInteger/valueOf network-int)
               .toByteArray)]
    (str/join "."
              (map #(format "%03d" (long (bit-and 0xFF (long %)))) bs))))

