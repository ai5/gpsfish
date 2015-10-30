(ns gpsdashboard.monitor_swing
  (:import [gpsdashboard.server_protocol ServerProtocol]
           [gpsdashboard.usi_tree_node UsiTreeNode]
           [java.awt BorderLayout Color Component Dimension FlowLayout Font]
           [java.awt.event KeyEvent]
           [java.text NumberFormat]
           [javax.swing BoxLayout JFrame JLabel JPanel JTextArea]
           [javax.swing.border BevelBorder]
           [javax.swing.text JTextComponent]
           [javax.imageio ImageIO]
           [org.jfree.chart JFreeChart ChartFactory ChartPanel ChartUtilities]
           [org.jfree.chart.axis DateAxis NumberAxis]
           [org.jfree.chart.plot CombinedDomainXYPlot XYPlot PlotOrientation]
           [org.jfree.chart.renderer.xy StandardXYBarPainter XYLineAndShapeRenderer]
           [org.jfree.data.time TimeSeries TimeSeriesCollection Millisecond]
           [org.jfree.data.statistics HistogramDataset]
           [org.jfree.ui RectangleInsets])
  (:require [gpsdashboard.instance :as instance]
            [gpsdashboard.subscriber :as sub]
            [gpsdashboard.usi :as usi]
            [gpsdashboard.time :as gt]
            [gpsdashboard.treemap :as treemap]
            [gpsdashboard.util :as util]
            [clojure.data.json :as json]
            [clojure.java.io :as io]
            [clojure.contrib.command-line :as cmd]
            [clojure.contrib.logging :as log])
  (:gen-class))


;; ==================================================
;; Global variables
;; ==================================================

(def main-frame (promise))
(def position-label (promise))
(def west-panel (promise))
(def histogram-panel (atom nil))
(def dump-file (atom nil))
(def dump-file-index (atom -1))

(defn online-mode?
  []
  (nil? @dump-file))

(defn offline-mode?
  []
  (not (online-mode?)))

;; ==================================================
;; GUI components
;; ==================================================

(def ^{:dynamic true} *max-age* (* 60 1000)) ; milliseconds

(def ^TimeSeries dataset-nps
  (doto
    (TimeSeries. "NPS" Millisecond)
    (.setMaximumItemAge *max-age*)))

(def ^TimeSeries dataset-ips
  (doto
    (TimeSeries. "#IPs" Millisecond)
    (.setMaximumItemAge *max-age*)))

(defn update-nps
  [^Integer y]
  (.add dataset-nps (Millisecond.) y))

(defn update-ips
  [^Integer y]
  (.add dataset-ips (Millisecond.) y))

(defn dynamic-time-series-plot
  [ds1 ds2]
  (let [ds-coll1 (doto
                   (TimeSeriesCollection.)
                   (.addSeries ds1))
        ds-coll2 (doto
                   (TimeSeriesCollection.)
                   (.addSeries ds2))
        domain (doto (DateAxis. "")
                 (.setAutoRange true)
                 (.setLowerMargin 0.0)
                 (.setUpperMargin 0.0)
                 (.setTickLabelsVisible true))
        nps-range (doto (NumberAxis. "Total NPS")
                  (.setStandardTickUnits (NumberAxis/createIntegerTickUnits)))
        ips-range (doto (NumberAxis. "Unique IPs")
                  (.setStandardTickUnits (NumberAxis/createIntegerTickUnits)))
        renderer1 (XYLineAndShapeRenderer. true false)
        renderer2 (XYLineAndShapeRenderer. true false)
        comb (doto (CombinedDomainXYPlot. domain)
               (.setGap 10.0))
        plot1 (doto (XYPlot. ds-coll1 nil nps-range renderer1)
                (.setBackgroundPaint     (Color/lightGray))
                (.setDomainGridlinePaint (Color/white))
                (.setRangeGridlinePaint  (Color/white))
                (.setAxisOffset (RectangleInsets. 5.0 5.0 5.0 5.0)))
        plot2 (doto (XYPlot. ds-coll2 nil ips-range renderer2)
                (.setBackgroundPaint     (Color/lightGray))
                (.setDomainGridlinePaint (Color/white))
                (.setRangeGridlinePaint  (Color/white))
                (.setAxisOffset (RectangleInsets. 5.0 5.0 5.0 5.0)))]
    (.add comb plot1)
    (.add comb plot2)
    comb))

(defn set-application-icon
  [^JFrame frame]
  (let [image (-> (io/resource "gpsshogi.png")
                  (io/input-stream)
                  (ImageIO/read))]
    (.setIconImage frame image)))

(defn make-histogram
  [xs]
  (let [data-set (if (seq xs)
                   (double-array xs)
                   (double-array [0]))
        dataset (doto (HistogramDataset.)
                  (.addSeries "NPS" data-set 10))
        chart   (ChartFactory/createHistogram "" nil nil
                                              dataset
                                              PlotOrientation/VERTICAL
                                              false false false)
        plot     (.getPlot chart)
        renderer (doto (.getRenderer plot)
                       (.setDrawBarOutline false)
                       (.setBarPainter (StandardXYBarPainter.))
                       (.setShadowVisible false))]
    (doto (ChartPanel. chart)
      (.setPreferredSize (Dimension. 200 200)))))

(defn layout
  [^JFrame frame]
  (set-application-icon frame)
  (.setPreferredSize frame (Dimension. 300 300))
  (let [pane (.getContentPane frame)]
    (when (online-mode?)
      (let [chart (doto (JFreeChart. ""
                                     nil         ; default font
                                     (dynamic-time-series-plot dataset-nps dataset-ips)
                                     false)      ; legend
                        (ChartUtilities/applyCurrentTheme))
            chart-panel (doto (ChartPanel. chart true)
                              (.setPreferredSize (Dimension. 300 300)))]
        (deliver west-panel (JPanel.))
        (.setLayout @west-panel (BoxLayout. @west-panel BoxLayout/PAGE_AXIS))
        (when (usi/is-osl-available?)
          (deliver position-label (doto (JTextArea. "Current Position")
                                    (.setEditable false)
                                    (.setFont (Font. "Monospaced" Font/PLAIN 17))
                                    (.setMaximumSize (Dimension. 300 300))))
          (.add @west-panel ^Component @position-label))
        (.add @west-panel chart-panel)
        (reset! histogram-panel (make-histogram nil))
        (.add @west-panel @histogram-panel)
        (.add pane @west-panel BorderLayout/LINE_START)))
    (let [bar-panel (doto (JPanel. (FlowLayout. FlowLayout/LEFT))
                          (.setBorder (BevelBorder. BevelBorder/LOWERED))
                          (.add (JLabel.)))]
      (.add pane bar-panel BorderLayout/PAGE_END))))

(defn update-position
  [text]
  (when (realized? position-label) 
    (javax.swing.SwingUtilities/invokeLater
      (fn []
        (.setText ^JTextComponent @position-label text)))))

(defn update-histogram
  [xs]
  (when (and (realized? west-panel)
             (not (nil? @histogram-panel)))
    (.remove @west-panel @histogram-panel)
    (reset! histogram-panel (make-histogram xs))
    (.add @west-panel @histogram-panel)))

(defn show-status-bar
  [ts-str body]
  (let [msg (format "[%s] %s                     (Color Blue: first player / Red: second player, Height: nodes, Width: depth)"
                    ts-str body)]
    (javax.swing.SwingUtilities/invokeLater
      (fn []
        (let [pane (.getContentPane ^JFrame @main-frame)
              ^BorderLayout layout (.getLayout pane)
              ^JPanel panel (.getLayoutComponent layout BorderLayout/PAGE_END)
              ^JLabel label (.getComponent panel 0)]
          (.setText label msg))))))

(defn showStatusNumbers
  [ts-str nps ips]
  (let [nf  (NumberFormat/getIntegerInstance)
        body (format "NPS: %s  IPs: %s"
                    (.format nf nps)
                    (.format nf ips))]
    (show-status-bar ts-str body)))

(defn alert-low-nps-nodes
  [cols]
  ;(let [nodes (take 3 (sort-by :nps cols))]
  (let [nodes (sort-by :nps (filter #(< 0 (:nps %) 60000) cols))]
    (doseq [n nodes]
      (if (not (.startsWith ^String (:host n) "192.168.0"))
        (log/warn (format "low nps: %s+GMT Host: %s NPS: %d" (gt/now-string (:updated n))
                                                 (:host n)
                                                 (:nps n)))))))

;; ==================================================
;; Functions
;; ==================================================

(defn kanji-converter
  [cols]
  (let [cols (map #(assoc % :usi-moves-str (:moves-str %)) cols)]
    (if (usi/is-osl-available?)
      (map #(update-in % [:moves-str] usi/usi-to-kanji) cols)
      cols)))

(defn message-handler
  "Show data in the on-line mode."
  [cols]
  (let [cols (kanji-converter cols)
        nps (instance/count-nps cols)
        ips (instance/count-unique-ips cols)]
    (log/trace (format "%s  % 3d IPs  % ,8d nps" (gt/now-string) nps ips))
    (when (realized? main-frame)
      (alert-low-nps-nodes cols)
      (update-nps nps)
      (update-ips ips)
      (update-histogram (map :nps cols))
      (showStatusNumbers (gt/now-string) nps ips)
      (when (not-empty cols)
        (let [tree-node-future (treemap/treemap-panel @main-frame
                                                      (.getContentPane ^JFrame @main-frame)
                                                      cols)]
          (if (and (usi/is-osl-available?) tree-node-future)
            (if-let [master (gpsdashboard.usi_tree_node/find-server cols)]
              (let [usi-moves (:usi-moves-str master)]
                (if (empty? usi-moves)
                  (log/warn "No usi moves to update a position")
                  (update-position (usi/usi-to-position usi-moves))))
              (log/debug "No master found. Position is not updated.")))))))
    true)

(defn show-message
  "Show data in the off-line mode."
  [data]
  (letfn [(split-message [s]
            (let [ts-str (subs data 0 19)
                  body   (subs data 19)]
              [ts-str body]))]
    (if (and (realized? main-frame)
             (not-empty data))
      (let [[ts-str body] (split-message data)]
        (cond
          (re-find #"go id (\S*) msec (\S*) (.*)" body)
            (show-status-bar ts-str body) 
          :else
            (let [cols (kanji-converter (json/read-str body :key-fn keyword :eof-error? false :eof-value ""))
                  nps  (instance/count-nps cols)
                  ips  (instance/count-unique-ips cols)]
              (showStatusNumbers ts-str nps ips)
              (treemap/treemap-panel @main-frame
                                     (.getContentPane ^JFrame @main-frame)
                                     cols)))))))

(defn show-message-at
  [index]
   (let [line (get @dump-file index)]
     (if (nil? line)
       (log/error (str "Out of range of the dump file: " index))
       (show-message line))))

(defn forward-dump-file
  []
  (if (<= -1 @dump-file-index (- (count @dump-file) 2)) 
    (show-message-at (swap! dump-file-index inc))
    (show-status-bar "-" (str "No more line: " @dump-file-index))))

(defn reverse-dump-file
  []
  (if (<= 1 @dump-file-index (- (count @dump-file) 1)) 
    (show-message-at (swap! dump-file-index dec))
    (show-status-bar "-" (str "No more line: " @dump-file-index))))

(defn key-pressed-handler
  [^KeyEvent event]
  {:pre [(= KeyEvent/KEY_PRESSED (.getID event))]}
  (condp = (.getKeyCode event)
    KeyEvent/VK_RIGHT      (forward-dump-file)
    KeyEvent/VK_UP         (forward-dump-file)
    KeyEvent/VK_ENTER      (forward-dump-file)
    KeyEvent/VK_SPACE      (forward-dump-file)
    KeyEvent/VK_LEFT       (reverse-dump-file)
    KeyEvent/VK_DOWN       (reverse-dump-file)
    KeyEvent/VK_BACK_SPACE (reverse-dump-file)
    :ignore))

(defn add-key-listener
  [^Component component handler]
  (.addKeyListener component
     (proxy [java.awt.event.KeyAdapter] []
       (keyPressed [event]
         (handler event)))))

(defn ^JFrame app []
  (let [f (doto (JFrame. "GPSShogi Cluster Dashboard")
                (.setDefaultCloseOperation JFrame/EXIT_ON_CLOSE)
                (layout))]
    (when (offline-mode?)
      (add-key-listener f key-pressed-handler))
    (deliver main-frame f)
    @main-frame))

(defn -main [& args]
  (cmd/with-command-line args
    (str "GPSShogi Clustor Dashboard - GUI Monitor\n"
         "Usage\n"
         "  gpsdashboard.monitor_swing [--host host] [--port port]\n"
         "  gpsdashboard.monitor_swing --read-dump-file file")
    [[host "host" "localhost"]
     [port "port" 8240]
     [read-dump-file "read a dump file" nil]
     remaining]
    (if (usi/is-osl-available?)
      (log/info "OSL library is available.")
      (log/info "OSL library is not available."))
    (if read-dump-file
      (do
        (log/info (format "Off-line mode to read %s" read-dump-file))
        (reset! dump-file (vec (line-seq (clojure.java.io/reader read-dump-file))))
        (when (empty? @dump-file)
          (log/error "The dump file is empty")
          (System/exit 1)))
      (do
        (log/info (format "Connecting to %s:%s" host port))
        (when-let [^ServerProtocol subscriber (sub/new-subscriber message-handler
                                                {:port (util/parseInt port) :host host})]
          (.start subscriber))))
    (javax.swing.SwingUtilities/invokeLater
      (fn []
        (doto (app)
          (.pack)
          (.setVisible true))))))
