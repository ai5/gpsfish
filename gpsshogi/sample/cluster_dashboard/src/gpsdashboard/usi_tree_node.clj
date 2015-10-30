(ns gpsdashboard.usi_tree_node
  (:require [gpsdashboard.network_util :as network_util]
            [gpsdashboard.util :as util] 
            [clojure.contrib.logging :as log])
  (:import [ch.randelshofer.tree TreeNode]))

(defn server-instance?
  "Check whether a col (map) is the server instance."
  [col]
  (= "server" (:host col)))

(defn find-server
  "Find the server instance from cols."
  [cols]
  (first (filter server-instance? cols)))

(defn count<?
  "Compare counts of sequeces."
  [x1 x2 & xs]
  (apply < (map count (list* x1 x2 xs))))

(defn diff-moves
  "Returns differences in an array of moves from root-moves to this-moves."
  [root-moves this-moves]
  (letfn [(throw-error []
           (log/warn (format "Unmatched diff moves: %s and %s." root-moves this-moves))
            nil)]
    (cond
      (= [] root-moves this-moves)    []
      (empty? root-moves)             (throw-error)
      (empty? this-moves)             (throw-error)
      (count<? this-moves root-moves) []
      (= root-moves
         (take (count root-moves) this-moves))
                                      (drop (count root-moves) this-moves)
      :else                           (throw-error))))

(defprotocol IUsiTreeNode
  (getMovesStr [this])
  (getUsiMovesStr [this])
  (getMoves [this])
  (getChild [this move])
  (addChild [this move node])
  (replaceChild [this move node])
  (getNPS [this])
  (getCumulativeNPS [this])
  (setCumulativeNPS [this cnps])
  (getIP [this])
  (getHostName [this])
  (getUpdated [this])
  (getDepth [this])
  (getScore [this])
  (setScore [this s])
  (getPvStr [this])
  (getPv [this])
  (getIgnoreMovesStr [this])
  (getIgnoreMoves [this])
  (blackOnMove? [this]))

(deftype UsiTreeNode [^{:unsynchronized-mutable true} children
                      ^String moves-str
                      ^String usi-moves-str
                      ^int nps
                      ^{:unsynchronized-mutable true :tag int} cumulative-nps
                      ^long ip
                      ^String host-name
                      ^long updated
                      ^int depth
                      ^{:unsynchronized-mutable true :tag int} score
                      ^String pv-str
                      ^String ignore-moves-str]
  TreeNode
  (children [this] (vals children))
  (getAllowsChildren [this]
    (if (empty? children) false true))
  IUsiTreeNode
  (getMovesStr [this] moves-str)
  (getUsiMovesStr [this] usi-moves-str)
  (getMoves [this] (util/parse-moves-str moves-str))
  (getChild [this move] (get children move))
  (addChild [this move node]
    (set! children (assoc children move node)))
  (replaceChild [this move node]
    (let [^UsiTreeNode old (get children move)]
      (assert old)
      (if (< (.getUpdated old) (.getUpdated ^UsiTreeNode node))
        (set! children (assoc children move node)))))
  (getNPS [this] nps)
  (getCumulativeNPS [this] cumulative-nps)
  (setCumulativeNPS [this cnps]
    (set! cumulative-nps (int cnps)))
  (getIP [this] ip)
  (getHostName [this] host-name)
  (getUpdated [this] updated)
  (getDepth [this] depth)
  (getScore [this]
    ; 76fu 34fu -> even moves -> black's teban
    (if (.blackOnMove? this) score (* -1 score)))
  (setScore [this s]
    (set! score (int s)))
  (getPvStr [this] pv-str)
  (getPv [this] (util/parse-moves-str pv-str))
  (getIgnoreMovesStr [this] ignore-moves-str)
  (getIgnoreMoves [this] (util/parse-moves-str ignore-moves-str))
  (blackOnMove? [this]
    (let [c (count (.getMoves this))]
      (even? (if (empty? (.getIgnoreMovesStr this))
               c
               (dec c))))))
   
(defn ^UsiTreeNode new-usi-tree-node
  "Constructor for UsiTreeNode"
  ([{:keys [ip host updated
            moves-str usi-moves-str nps cumulative-nps
            depth score pv-str
            ignore-moves-str]
     :or {ip 0, host "_", updated 0, 
          moves-str "", usi-moves-str "", nps 0, cumulative-nps 0,
          depth 0, score 0, pv-str "",
          ignore-moves-str ""}}]
   (UsiTreeNode. {}
                 moves-str
                 usi-moves-str
                 (int nps)
                 (int cumulative-nps)
                 (long ip) host updated
                 (int depth)
                 (int score)
                 pv-str
                 ignore-moves-str)))

(defn sort-by-moves-str
  [cols]
  (sort-by :moves-str cols))

(defn ^String estimate-root
  "Estimate the root moves string, extracting the longest common part among instance's moves-str"
  [first-moves-str cols]
  {:pre [(not-empty first-moves-str)
         (not-empty cols)]}
  (letfn [(subset? [s1 s2]
            {:pre [(not-empty s1)]}
            (and (count<? s1 s2)
                 (= s1 (take (count s1) s2))))]
    (let [candidates (map (comp util/parse-moves-str :moves-str) cols)]
      (loop [moves (util/parse-moves-str first-moves-str)]
        (if (seq moves)
          (if (every? #(subset? moves %) candidates)
            (util/join-moves moves)
            (recur (butlast moves)))
          (do
            (log/error (str "No root moves found: " first-moves-str))
            ""))))))

(defn make-root-node
  "Make a new root node."
  [root-moves-str]
  {:pre [(not-empty root-moves-str)]}
  (new-usi-tree-node {:moves-str root-moves-str}))

(defn add-node
  "Add a (new) node for col under root-node."
  [^UsiTreeNode root-node col]
  (letfn [(throw-error [col]
           (if-not (server-instance? col)
             (let [ip  (network_util/long-to-ip-string (:ip col))
                   msg (str "Duplicated or shorter nodes found: " ip)]
               (log/error msg))))
          (append-others [col]
            (if (not-empty (:ignore-moves-str col))
              (update-in col [:moves-str] str " others")
              col))]
    (let [col (append-others col)]
      (loop [parent    root-node
             moves     (.getMoves root-node)
             relatives (diff-moves (.getMoves root-node)
                                   (util/parse-moves-str (:moves-str col)))]
        (case (count relatives)
          0 (throw-error col)
          1 (let [next-move (first relatives)
                  new-node (new-usi-tree-node col)]
              (if-let [child (.getChild parent next-move)]
                (.replaceChild parent next-move new-node)
                (.addChild     parent next-move new-node)))
          (let [next-move (first relatives)
                moves     (conj moves next-move)]
            (if-let [child (.getChild parent next-move)]
              (recur child moves (rest relatives))
              (let [new-node (new-usi-tree-node {:moves-str (util/join-moves moves)})]
                (.addChild parent next-move new-node)
                (recur new-node moves (rest relatives))))))))))

(defn construct-nodes
  "Construct a tree of nodes"
  [^UsiTreeNode root-node cols]
  (log/debug (format "cunstructing a tree for nodes: %d" (count cols)))
  (dorun (map #(add-node root-node %) cols))
  root-node)

(defn ^UsiTreeNode calc-cumulative-nps
  "Calculate cumulated NPS for each node"
  [^UsiTreeNode node]
  (letfn [(set-get-cumulative-nps [^UsiTreeNode node]
            (calc-cumulative-nps node)
            (.getCumulativeNPS node))]
    (if (.getAllowsChildren node)
      (let [cnps (apply + (.getNPS node)
                          (map set-get-cumulative-nps (.children node)))]
        (.setCumulativeNPS node cnps))
      (.setCumulativeNPS node (.getNPS node)))
    node))

(defn ^UsiTreeNode min-max-score
  "Applies min-max on the tree to calculate evaluation values."
  [^UsiTreeNode node]
  (if (empty? (.children node))
    (.getScore node)
    (let [mm    (if (.blackOnMove? node) max min)
          score (apply mm (map min-max-score (.children node)))]
      (.setScore node (if (.blackOnMove? node) score (* -1 score)))
      score)))

(defn ^UsiTreeNode make-tree
  "Make a tree from instances then returns the root node"
  [cols]
  {:pre [(not-empty cols)]}
  (let [sorted-cols    (sort-by-moves-str cols)
        first-instance (first sorted-cols)
        root-moves-str (if-let [server (find-server cols)]
                         (do
                           (log/debug (str "INFO START NEW MASTER POSITION: " (:moves-str server)))
                           (:moves-str server))
                         (do
                           (log/debug "Estimating the root node...")
                           (estimate-root (:moves-str first-instance) sorted-cols)))]
    (let [root (doto (make-root-node root-moves-str)
                 (construct-nodes sorted-cols)
                 (calc-cumulative-nps)
                 (min-max-score))]
      root)))
