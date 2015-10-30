(ns gpsdashboard.test.usi_tree_node
  (:use [gpsdashboard.usi_tree_node]
        [midje.sweet])
  (:require [gpsdashboard.instance :as instance]))


(fact "test relative-moves"
  (diff-moves ["a" "b"] ["a" "b" "c"]) => ["c"]
  (diff-moves ["a" "b"] ["a" "b"]) => []
  (diff-moves [] []) => []
  (diff-moves ["a" "b"] ["a" "c" "d"]) => nil
  (diff-moves ["a" "b"] ["x" "b"]) => nil) 

(def i123 (instance/new-instance 123 "macpro123" {:updated 1 :nps 1230 :moves-str "a1 b1 c1"}))
(def i124 (instance/new-instance 124 "macpro124" {:updated 1 :nps 1240 :moves-str "a1 b1 c2"}))
(def i125 (instance/new-instance 125 "macpro125" {:updated 1 :nps 1250 :moves-str "a1 b1 c2 d1"}))
(def i126 (instance/new-instance 126 "macpro126" {:updated 1 :nps 1260 :moves-str "a1 b1 c2 d2"}))
(def i127 (instance/new-instance 127 "macpro127" {:updated 1 :nps 1270 :moves-str "a1 b1 c3"}))
(def cols [i123 i124 i125 i126 i127])

(fact "test sort"
  (sort-by-moves-str [i123]) => [i123]
  (sort-by-moves-str [i123 i124]) => [i123 i124]
  (sort-by-moves-str [i124 i123]) => [i123 i124]
  (sort-by-moves-str [i126 i127 i123 i125 i124]) => [i123 i124 i125 i126 i127])

(fact "test estimate-root"
  (estimate-root "a1 b1"    [i123]) => "a1 b1"
  (estimate-root "a1 b1 c1" [i123]) => "a1 b1"
  (estimate-root "a1 b1 c1" cols)   => "a1 b1")

(fact "test make-root-node"
  (let [node (make-root-node "a1 b1")]
    (.getMovesStr node) => "a1 b1"
    (.getMoves node) => ["a1" "b1"]))

(fact "test add-node"
  (let [node (make-root-node "a1 b1")]
    (add-node node i123)
    (let [cs (.children node)]
      (count cs) => 1
      (.getMovesStr (first cs)) => (:moves-str i123)
      (.getNPS      (first cs)) => (:nps i123))
    (.getChild node "c1") => truthy)
  (let [node (make-root-node "a1 b1")]
    (add-node node i125)
    (let [cs (.children node)]
      (count cs) => 1
      (.getMovesStr (first cs)) => "a1 b1 c2"
      (.getMoves (first cs)) => ["a1" "b1" "c2"]
      (.getChild node "c2") => truthy
      (.getAllowsChildren (first cs)) => true
      (let [ccs (.children (first cs))]
        (count ccs) => 1
        (.getMovesStr (first ccs)) => "a1 b1 c2 d1"
        (.getMoves (first ccs)) => ["a1" "b1" "c2" "d1"]
        (.getChild (first cs) "d1") => truthy
        (.getAllowsChildren (first ccs)) => false)))
  (let [node (make-root-node "a1 b1")
        i200 (instance/new-instance 200 "macpro200" {:updated 1 :nps 2000 :moves-str "a1 b1 *"
                                                     :ignore-moves-str "i1"})]
    (add-node node i123)
    (add-node node i200)
    (let [cs (.children node)]
      (count cs) => 2
      (.getMovesStr (first  cs)) => (:moves-str i200)
      (.getMovesStr (second cs)) => (:moves-str i123))))

(fact "test make-root-node"
  (make-root-node "a1 b1") => truthy)

(fact "test construct-nodes"
  (let [root (make-root-node "a1 b1")]
    (construct-nodes root [i123 i124]) => truthy))
    ;(construct-nodes root [i123 i123]) => (throws IllegalArgumentException)))

(fact "integration test; making a tree"
  (let [root (make-tree cols)]
    (count (.children root)) => 3
    (let [c1 (.getChild root "c1")]
      (count (.children c1)) => 0
      (.getCumulativeNPS c1) => 1230)
    (let [c2 (.getChild root "c2")]
      (count (.children c2)) => 2
      (let [d1 (.getChild c2 "d1")]
        (count (.children d1)) => 0
        (.getCumulativeNPS d1) => 1250)
      (let [d2 (.getChild c2 "d2")]
        (count (.children d2)) => 0
        (.getCumulativeNPS d2) => 1260)
      (.getCumulativeNPS c2) => (+ 1240 1250 1260))
    (let [c3 (.getChild root "c3")]
      (count (.children c3)) => 0
      (.getCumulativeNPS c3) => 1270)
    (.getCumulativeNPS root) => (+ 1230 1240 1250 1260 1270)))
