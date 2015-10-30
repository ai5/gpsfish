;;; Copyright (C) 2011-2014 Team GPS.
;;; 
;;; This program is free software; you can redistribute it and/or modify
;;; it under the terms of the GNU General Public License as published by
;;; the Free Software Foundation; either version 2 of the License, or
;;; (at your option) any later version.
;;; 
;;; This program is distributed in the hope that it will be useful,
;;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;; GNU General Public License for more details.
;;; 
;;; You should have received a copy of the GNU General Public License
;;; along with this program; if not, write to the Free Software
;;; Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

(ns twitter_clj.twitter
  (:import (twitter4j StatusUpdate Twitter TwitterFactory TwitterException))
  (:import (twitter4j.conf ConfigurationBuilder))
  (:import (java.io File))
  (:require [twitter_clj.draw_image :as draw]
            [twitter_clj.common :as common]
            [twitter_clj.title :as title]
            [clojure.java.io :as io]
            [clojure.tools.logging :as log]))

;; ==================================================
;; Global variables
;; ==================================================

; At most n images to tweet
(def at-most 4)

;; ==================================================
;; Functions
;; ==================================================

(defn twitter-instance
  "Return an instance of Twitter class by using keys read from environment varibles:
    * GPSUSI_CONSUMER_KEY
    * GPSUSI_CONSUMER_SECRET
    * GPSUSI_ACCESS_TOKEN
    * GPSUSI_TOKEN_SECRET"
  []
  (let [env (System/getenv)
        [consumer-key
         consumer-secret
         access-token
         token-secret] (map #(get env %) ["GPSUSI_CONSUMER_KEY"
                                          "GPSUSI_CONSUMER_SECRET"
                                          "GPSUSI_ACCESS_TOKEN"
                                          "GPSUSI_TOKEN_SECRET"])]
    (when (some nil? [consumer-key consumer-secret access-token token-secret])
      (log/fatal "Twitter configuration not found")
      (throw (IllegalStateException.)))
    (let [conf (doto (ConfigurationBuilder.)
                 (.setOAuthConsumerKey       consumer-key)
                 (.setOAuthConsumerSecret    consumer-secret)
                 (.setOAuthAccessToken       access-token)
                 (.setOAuthAccessTokenSecret token-secret))]
     (.getInstance (TwitterFactory. (.build conf))))))

(defn log-message
  "Log a message to a file if the file does not exist.
   Return true if writting the message is successful;
   false otherwise (ex. the file exists)"
  [f str]
  (if (.exists ^File f)
    (do
      (log/warn (format "Found an existing file: %s" f))
      false)
    (do
      (spit f str)
      true)))

(defn gen-logger
  "Returns a logger fn which writes a str to a fn f."
  [f]
  (fn [str]
    (log-message f str)))

(defn parse-nmove
  "Parse a twitter message and return n-th of moves."
  [str]
  (let [m (re-find #"^\[\((\d+)\) .*\]" str)
        _ (assert m)]
    (Integer. ^String (nth m 1))))

(defn moves-file
  "Return a File instance into which moves tweets are recorded."
  [str]
  (let [m (re-find #"^\[\((\d+)\) .*\] <(\d+)s>" str)
        _ (assert m)
        nmove (Integer. ^String (nth m 1))
        sec   (Integer. ^String (nth m 2))
        file-name (format "analyses%03d_%05d.txt" nmove sec)]
    (io/file file-name)))

(defn moves-file-exists?
  [nmove]
  (let [re (re-pattern (format "analyses%03d_.*.txt$" nmove))]
    (some #(re-find re %) (.list (File. "./")))))

(defn trancate-140-characters
  "Trancate a string str if it includes too many characters."
  [str]
  (if (< 140 (count str))
   (let [to-str (subs str 0 140)]
     (log/warnf "Trancated characters from %s to %s" str to-str)
     to-str)
   str))

(defn upload-images
  "Upload image files and return an array of their media IDs. The images
  files can contain nil."
  [^Twitter twitter & files]
  (loop [files (remove nil? files)
         ret   []]
    (if (seq files)
      (let [f (first files)]
        (if (and (not (nil? f)) (.exists ^File f))
          (let [id (-> twitter
                  (.uploadMedia f)
                  (.getMediaId))]
            (log/infof "Uploaded an image %s [%d]" f id)
            (recur (rest files) (conj ret id)))
          (do
            (log/warnf "File not found: %s" f)
            (recur (rest files) ret))))
      ret))); base

(defn post-twitter
  "Tweet a message, attaching image files as a list of java.io.File. Before
  sending a tweet, it first attempts to log it to a file through a logger
  fn. Only if it succeeds, it then sends a tweet."
  [logger str & media-files]
  (let [str ^String (trancate-140-characters str)]
    (log/debugf "Tweeting...  %s [with %d images]" str (count media-files))
    (if (logger str)
      (try
        (if-not (:dry-run @common/options)
          (let [twitter ^Twitter (twitter-instance)
                update  (StatusUpdate. str)]
            ; Prepare images to be sent with a message
            (->> (apply upload-images twitter media-files)
              (long-array)
              (.setMediaIds update))
            (let [status (.updateStatus twitter update)]
              (if (.isTruncated status)
                (log/warn "Message was truncated"))
              (log/infof "Tweeted [%d]: %s" (.getId status) (.getText status))
              status)))
        (catch TwitterException e
          (log/warnf "Failed to tweet: %s %s" str e))))))

(defn post-move
  "Tweet a move message. If this is the first message of a position, attach
  a board image. In addition PV board images, if any, are also attached. 
   @param msg a move message
   @param nmove this message for the nmove-th move
   @param sec thinking time for this move
   @param i For this move this post is the i-th messages, starting with 0"
  [msg nmove sec i]
  (let [current-board-image (if (zero? i)
                              (io/file (-> (parse-nmove msg)
                                        (draw/file-name)))
                              nil)
        pv-images (common/glob "." (format "board_image_%d_%d_*.png" nmove sec))
        image-files (take at-most (cons current-board-image pv-images))]
    (apply post-twitter (gen-logger (moves-file msg))
                        msg
                        image-files)))

(defn post-version
  "Tweet an engine version"
  [str]
  (if (empty? str)
    (log/warn "Empty version.")
    (post-twitter (gen-logger (io/file "tweet-version.txt")) str)))

(defn post-title
  "Tweet a title."
  []
  (title/parse-kif "target.kif")
  (post-twitter (gen-logger (io/file "tweet-title.txt"))
                (title/twitter-title)))

(defn -main [& args]
  "To test a tweet."
  (post-move (first args)))
