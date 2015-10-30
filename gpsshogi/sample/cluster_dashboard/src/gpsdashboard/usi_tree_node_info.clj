(ns gpsdashboard.usi_tree_node_info
  (:import [gpsdashboard.usi_tree_node UsiTreeNode]
           [ch.randelshofer.tree NodeInfo TreeNode TreePath2 Colorizer Weighter]
           [ch.randelshofer.tree.demo RGBColorizer]
           [java.awt Color]
           [java.text DecimalFormat])
  (:require [gpsdashboard.usi_tree_node :as usi_tree_node]
            [gpsdashboard.network_util :as network_util]
            [gpsdashboard.util :as util]
            [clojure.contrib.logging :as log]))

(defn ^UsiTreeNode get-leaf
  [^TreePath2 path]
  (.getLastPathComponent path))

(defn get-parent
  [^TreePath2 path]
  (let [num-nodes (.getPathCount path)]
    (if (<= 2 num-nodes)
      (.getPathComponent path (- num-nodes 2))
      nil)))

(defn path-to-array
  "Convert TreePath2 into array (parent to child)"
  [^TreePath2 path]
  (loop [path path
         ret []]
    (if (nil? path)
      (reverse ret)
      (recur (.getParentPath path) (conj ret (get-leaf path))))))

(defn bold
  [s]
  (str "<b>" s "</b>"))

(defn sigmoid
  ([x] (sigmoid 0.002 x))
  ([a x]
    (/ 1.0 (+ 1.0 (Math/exp (* -1.0 a x))))))

(defn score-to-weight
  "Convert score to a float value between 0 and 1."
  [score]
  (sigmoid score))

(deftype ScoreWeighter []
  Weighter
  (^void init [this ^TreeNode root] nil) ; do nothing
  (^float getWeight [this ^TreePath2 path]
    (let [node (get-leaf path)]
      (float (score-to-weight (* -1 (.getScore node)))))) ; black: blue color, white: red color
  (^ints getHistogram [this] (int-array)) ; TODO
  (^String getHistogramLabel [this ^int index] "") ; TODO
  (^String getMinimumWeightLabel [this] "") ; TODO
  (^String getMaximumWeightLabel [this] "") ; TODO
  (^float getMedianWeight [this] (float 0)))

(defn new-score-weighter
  "Constructor of ScoreWeighter."
  []
  (ScoreWeighter.))

(defn partition-moves
  [moves]
  (apply str
    (map #(str (util/join-moves %) "<br>")
      (partition-all 6 moves))))

(deftype UsiTreeNodeInfo [^{:unsynchronized-mutable true :tag UsiTreeNode} root-node
                          ^Colorizer colorizer ^Weighter score-weighter]
  NodeInfo
  (init [this root] (set! root-node root))
  (getName [this path]
    (let [leaf-node (get-leaf path)
          move      (last (.getMoves leaf-node))
          num-nodes (.getPathCount path)]
      (if-let [^UsiTreeNode parent-node (get-parent path)]
        (if (= (.getScore parent-node) (.getScore leaf-node))
          (str move "^")
          move)
        move)))
  (getColor [this path]
    (let [v (.getWeight score-weighter path)]
      (.get colorizer v)))
  (getWeight [this path]
    (.getNPS (get-leaf path)))
  (getCumulatedWeight [this path]
    (.getCumulativeNPS (get-leaf path)))
  (getWeightFormatted [this path]
    (.format (DecimalFormat/getIntegerInstance)
             (.getWeight this path)))
  (getTooltip
    [this path]
    (let [node (get-leaf path)]
      (str "<html>"
           (let [nodes (path-to-array path)]
             (apply str (map #(str %2 ": " (.getHostName ^UsiTreeNode %1) "<br>") nodes (range))))
           "<br>"
           (bold (.getHostName node)) " @ " (network_util/long-to-ip-string (.getIP node)) "<br>"
           (let [relatives (usi_tree_node/diff-moves (.getMoves root-node) (.getMoves node))]
             (if (empty? relatives)
               (str (bold "ROOT NODE") ": [" (count (.getMoves node)) "]:<br>"
                    (partition-moves (.getMoves node)))
               (str (bold "depth") ": [" (count relatives) "] " (util/join-moves relatives) "<br>")))
           (bold "nps")          ": " (.getNPS   node) " / " (.getCumulativeNPS node) "<br>"
           (bold "score")        ": " (.getScore node) "<br>"
           (bold "pv")           ": " (partition-moves (.getPv node)) "<br>"
           (bold "ignore moves") ": " (partition-moves (.getIgnoreMoves node)) "<br>"
           (bold "#children")    ": " (count (.children node)) "<br>"
           "</html>")))
  (getActions [this path] [])
  (getImage [this path] nil)
  (getWeighter [this] score-weighter)
  (getColorizer [this] colorizer)
  (addChangeListener [this listener] ); do nothing
  (removeChangeListener [this listener] ); do nothing
  (toggleColorWeighter [this] )); do nothing

(defn new-usi-tree-node-info
  [root-node]
  (UsiTreeNodeInfo. root-node (RGBColorizer.) (new-score-weighter)))

