(ns webinterface.web
  (:import [java.io File])
  (:require [webinterface.kifToCsa :as kifToCsa] 
            [webinterface.gps :as gps]
            [webinterface.shogiserver :as shogiserver]
            [clojure.java.io :as io]
            [clojure.string :as str]
            [clojure.tools.logging :as log]
            [compojure.core :refer [defroutes GET PUT POST DELETE ANY]]
            [compojure.handler :as handler]
            [compojure.route :as route]
            [environ.core :as env]
            [hiccup.core :refer [html h]]
            [hiccup.form :as f]
            [org.httpkit.server :as hk]
            [ring.middleware.defaults   :refer [wrap-defaults site-defaults]]
            [ring.middleware.logger     :as middlelogger]
            [ring.middleware.reload     :as reload]
            [ring.middleware.stacktrace :as trace]
            [ring.util.anti-forgery :refer [anti-forgery-field]])
  (:gen-class))

(def ^:dynamic *version* "1.1.0")
(def ^:dynamic *cookie-max-age* (* 3600 24)) ;; seconds of one day
(def ^:dynamic *title* "GPS将棋 Web設定ページ")
(def ^:dynamic *docpdf* "20141011-GPSWebInterface.pdf")
(def ^:dynamic *thinktime* 30) ;; minutes
(def ^:dynamic *byoyomi*   30) ;; seconds


(defn upload-file
  [file]
  "Saving an uploaded file into a temporary file, return the temporary file
  as java.io.File."
  (let [tempfile (File/createTempFile "webInterface" ".tmp")]
    (io/copy file (io/as-file tempfile))
    tempfile))

(defn finish-html
  [params cookies]
  (let [turn       (:turn params)
        radiomove  (:radiomove params)
        selectmove (:selectmove params)
        thinktime  (* 60 (Integer. (:thinktime params))) ; seconds
        byoyomi    (Integer. (:byoyomi params))
        moves      (:value (cookies "moves"))]
     (if radiomove
       (let [game-name (gps/generate-regular-game-name thinktime byoyomi)]
         (gps/spawn-gps game-name thinktime byoyomi turn))
       (let [cut-moves (take (inc (Integer/valueOf selectmove))
                             (str/split moves #"\s"))
             game-name (gps/generate-buoy-game-name thinktime byoyomi)]
         (shogiserver/set-specified-position turn cut-moves game-name)
         (gps/spawn-gps game-name thinktime byoyomi turn)))
    {:status  200
     :headers {"Content-Type"  "text/html; charset=utf-8"}
     :body (html
             [:html
               [:head
                 [:meta {:http-equiv "Content-Type" :content "text/html; charset=utf-8"}]
                 [:title *title*]]
               [:body
                 [:p "登録しました。"]
                 [:a {:href "./"} "戻る"]
               ]])
     :cookies (if-not radiomove
                {"moves"      {:value moves      :max-age *cookie-max-age*}
                 "selectmove" {:value selectmove :max-age *cookie-max-age*}
                 "turn"       {:value turn       :max-age *cookie-max-age*}
                 "thinktime"  {:value (/ thinktime 60) :max-age *cookie-max-age*}
                 "byoyomi"    {:value byoyomi    :max-age *cookie-max-age*}}
                {})}))

(defn index-html
  [params cookies]
  (let [[moves selectmove]
                   (cond
                     (:kifu params)    ;; A file has just been uploaded, which is a priority.
                       [(kifToCsa/convert-kifu (get-in params [:kifu :tempfile]))
                        -1]
                     (cookies "moves") ;; There exists a string in cookies
                       [(str/split (:value (cookies "moves")) #"\s")
                        (Integer/valueOf (or (:value (cookies "selectmove")) -1))]
                     :else
                       [[] -1])
        turn       (or (:turn params)
                       (:value (cookies "turn"))
                       "black")
        thinktime  (or (:thinktime params)
                       (:value (cookies "thinktime"))
                       *thinktime*)
        byoyomi    (or (:byoyomi params)
                       (:value (cookies "byoyomi"))
                       *byoyomi*)
        radiomove  (Integer/valueOf (or (:radiomove params) -1))]
   {:status  200
    :headers {"Content-Type"  "text/html; charset=utf-8"}
    ; Set cookies with a space-delimited string of moves
    :cookies {"moves" {:value (str/join " " moves) :max-age *cookie-max-age*}}
    :body (html
      [:html
        [:head
          [:meta {:http-equiv "Content-Type" :content "text/html; charset=utf-8"}]
          [:style
            "div.footer {text-align: right; font-size: 80%;}"]
          [:title *title*]]
            [:body
              [:h1 [:a {:href "./"} *title*]]
              (if (gps/gps-running?)
                (list
                  [:p {:style "color:red"}
                    "稼働中のGPS将棋があります。「送信」すると稼働中のものは強制終了されます。"]
                  [:hr]))
              [:form {:action "./post" :method "post" :enctype "multipart/form-data"}
                (anti-forgery-field) ;; returns the HTML for the anti-forgery field
                [:p [:b "1. 局面の指定"] [:br]
                  "初形" (f/radio-button "radiomove" (<= 0 radiomove) 1) [:br]
                  "ないしは"
                  (if-not (empty? moves)
                    [:div "局面を選択: "
                      [:select {:name "selectmove"}
                        (f/select-options (map-indexed (fn [idx itm]
                                                         [(format "% 3d: %s" (inc idx) itm) idx])
                                                       moves)
                                          selectmove)]])
                  [:div "棋譜ファイルから指定: " (f/file-upload "kifu")
                    [:input {:type "submit" :name "submitkifu" :value "棋譜の送信"}]]]
                [:p [:b "2. 先後の指定"] [:br]
                  "人間が"
                  [:select {:name "turn"}
                    (f/select-options [["先手" "black"] ["後手" "white"]] turn)]]
                [:p [:b "3. 対局条件の指定"] [:br]
                   "持ち時間: " (f/text-field "thinktime" thinktime) "分" [:br]
                   "秒読み: "   (f/text-field "byoyomi"   byoyomi)   "秒"]
                [:p [:input {:type "submit" :name "submitall" :value "登録"}]]]
              [:hr]
              [:div [:a {:href *docpdf*} "ドキュメント"]]
              [:div {:class "footer"} "version " *version*]
            ]])}))

(defroutes main-routes
  (GET ["/"] {params :params cookies :cookies}
    (index-html params cookies))
  (POST ["/post"] {params :params cookies :cookies}
    (cond
      (not (empty? (:submitkifu params))) ; Upload a kifu file
        (index-html params cookies)
      (not (empty? (:submitall params)))  ; Regist butten was pushed
        (if (and (not (nil? (:turn params)))
                  (or (not (nil? (:radiomove params)))
                      (not (nil? (:selectmove params)))))
          (finish-html params cookies)
          (index-html params cookies))
      :else                               ; Safety. This will never happen.
          (index-html params cookies)))
  (ANY "*" []
    (route/not-found (slurp (io/resource "404.html")))))

(defn wrap-error-page
  [handler]
  (fn [req]
    (try (handler req)
         (catch Exception e
           {:status 500
            :headers {"Content-Type" "text/html"}
            :body (slurp (io/resource "500.html"))}))))

; do not work
(defn wrap-charset
  [handler] 
  (fn [request] 
    (if-let [response (handler request)] 
      (if-let [content-type (get-in response [:headers "Content-Type"])] 
        (if (.contains content-type "charset") 
          response 
          (assoc-in response 
                    [:headers "Content-Type"] 
                    (str content-type "; charset=utf-8"))) 
        response)))) 

;; Handler for the production mode
(def prod-handler
  (-> main-routes
      middlelogger/wrap-with-logger
      (wrap-defaults site-defaults)
      wrap-error-page))

;; Handler for the development mode
(def dev-handler
  (-> main-routes
      middlelogger/wrap-with-logger
      (wrap-defaults site-defaults)
      reload/wrap-reload
      trace/wrap-stacktrace))

(defn dev?
  []
  (not (nil? (env/env :webinterface-dev))))

(defn -main [& [port]]
  (let [port (Integer. (or port (env/env :port) 8080))
        handler (if (dev?)
                  dev-handler
                  prod-handler)]
    (hk/run-server handler {:port port})
    (if (dev?)
      (log/info "DEV mode"))
    (log/infof "Server started, listening at localhost@%d" port)))

