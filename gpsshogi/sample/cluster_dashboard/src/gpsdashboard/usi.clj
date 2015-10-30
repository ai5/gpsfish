(ns gpsdashboard.usi
  (:require [clojure.contrib.logging :as log]
            [gpsdashboard.util :as util])
  (:import [java.nio ByteBuffer Buffer]
           [java.nio.charset Charset CharsetEncoder CharsetDecoder])
  (:use [clojure.string :only (join split)]
        [net.n01se.clojure_jna]))

(def osl-usi-moves-to-kanji
  (try
    (jna-fn Integer osl/usiMovesToKanji)
    (catch java.lang.UnsatisfiedLinkError e
      (log/warn "libosl.so not found. Disable to use Japanese localization functionality.")
      nil)))

(def osl-usi-moves-to-position-string
  (try
    (jna-fn Integer osl/usiMovesToPositionString)
    (catch java.lang.UnsatisfiedLinkError e
      nil)))

(defn is-osl-available?
  []
  (and (not (nil? osl-usi-moves-to-kanji))
       (not (nil? osl-usi-moves-to-position-string))))

(defn buf-to-string
  [^Buffer buf size]
  (.limit buf size)
  (.rewind buf)
  (let [^Charset charset (Charset/forName "EUC-JP")]
    (-> (.decode charset buf)
        (.toString))))

(defn end-with-others?
  [usi-moves]
  (let [moves (util/parse-moves-str usi-moves)]
    (= "others" (last moves))))

(defn remove-others
  "Remove 'others' in usi-moves."
  [usi-moves]
  (if (end-with-others? usi-moves)
    (util/join-moves (butlast (util/parse-moves-str usi-moves)))
    usi-moves))

(defn append-others
  "Append 'others' to converted moves if orig ends with 'others'."
  [orig converted]
  (if (end-with-others? orig)
    (str converted " others")
    converted))

(defn usi-to-kanji
  [usi-moves]
  (if is-osl-available?
    (let [buf-size 16384
          buf (make-cbuf buf-size)
          out-size (osl-usi-moves-to-kanji (str "ki2moves " (remove-others usi-moves))
                                           (pointer buf)
                                           buf-size)
          out (buf-to-string buf out-size)]
      (append-others usi-moves
                     (join " " (rest (split (subs out (count "ki2moves "))
                                            #"[▲△]")))))
    usi-moves))

(defn usi-to-position
  [usi-moves]
  (if is-osl-available?
    (let [buf-size 2048
          buf (make-cbuf buf-size)
          out-size (osl-usi-moves-to-position-string usi-moves
                                                     (pointer buf)
                                                     buf-size)
          out (buf-to-string buf out-size)]
      out)
    ""))

