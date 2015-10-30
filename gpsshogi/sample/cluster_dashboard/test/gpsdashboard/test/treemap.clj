(ns gpsdashboard.test.treemap
  (:import [ch.randelshofer.gui ProgressTracker]
           [ch.randelshofer.tree.circlemap CirclemapNode CirclemapCompositeNode])
  (:require [gpsdashboard.instance :as instance]
            [gpsdashboard.usi_tree_node :as usi_tree_node]
            [gpsdashboard.usi_tree_node_info :as usi_tree_node_info])
  (:use [gpsdashboard.treemap]
        [midje.sweet]))


(def i123 (instance/new-instance 123 "macpro123" {:updated 1 :nps 1230 :moves-str "a1 b1 c1"    :moves ["a1" "b1" "c1"]}))
(def i124 (instance/new-instance 124 "macpro124" {:updated 1 :nps 1240 :moves-str "a1 b1 c2"    :moves ["a1" "b1" "c2"]}))
(def i125 (instance/new-instance 125 "macpro125" {:updated 1 :nps 1250 :moves-str "a1 b1 c2 d1" :moves ["a1" "b1" "c2" "d1"]}))
(def i126 (instance/new-instance 126 "macpro126" {:updated 1 :nps 1260 :moves-str "a1 b1 c2 d2" :moves ["a1" "b1" "c2" "d2"]}))
(def i127 (instance/new-instance 127 "macpro127" {:updated 1 :nps 1270 :moves-str "a1 b1 c3"    :moves ["a1" "b1" "c3"]}))
(def cols [i123 i124 i125 i126 i127])

(fact "integration test; making a tree"
  (let [root  (usi_tree_node/make-tree cols)
        node (CirclemapCompositeNode. nil root)]
    (count (.children node)) => 3
    (let [cs (.children node)
          c0 (.get cs 0)
          c1 (.get cs 1)
          c2 (.get cs 2)]
      (.getMovesStr (.getDataNode c0)) => "a1 b1 c3"
      (.getDescendantCount c0) => 0
      (.getMovesStr (.getDataNode c1)) => "a1 b1 c2"
      (count (.children c1)) => 2
      (.getDescendantCount c1) => (dec 2)
      (.getMovesStr (.getDataNode c2)) => "a1 b1 c1"
      (.getDescendantCount c2) => 0)
    (.getDescendantCount node) => (- 5 1 1)))
