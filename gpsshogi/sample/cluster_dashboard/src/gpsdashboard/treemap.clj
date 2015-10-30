(ns gpsdashboard.treemap
  (:import [java.awt Color Container BorderLayout]
           [javax.swing JFrame JPanel SwingUtilities]
           [ch.randelshofer.tree.sunburst IcicleDraw IcicleModel IcicleView])
  (:require [gpsdashboard.usi_tree_node :as usi_tree_node]
            [gpsdashboard.usi_tree_node_info :as usi_tree_node_info]
            [clj-time.core :as cjtcore]
            [clojure.contrib.logging :as log]))

;; ==================================================
;; Global variables
;; ==================================================

(def model-builder-future (atom nil))


;; ==================================================
;; Functions
;; ==================================================

(defn treemap-panel
  [^JFrame main-frame ^Container parent cols]
  {:pre [(not-any? nil? [main-frame parent])]}
  (letfn [(get-view [^IcicleModel model]
            (let [view (.getView model)]
              (.setToolTipEnabled view true)
              (.setDraw view 
                        (IcicleDraw. (.getModel view)))
              view))
          (swap-view-fn [^JFrame main-frame ^Container parent ^IcicleView view]
            (fn []
              (let [^java.awt.BorderLayout layout (.getLayout parent)
                    previous (.getLayoutComponent layout BorderLayout/CENTER)]
                (if previous (.remove parent previous))
                (.add parent view BorderLayout/CENTER)
                (.validate parent)))) ; repaint was not needed
          (new-future []
            (future 
              (try
                (log/debug "Constructing a tree model....")
                (let [root-node (usi_tree_node/make-tree cols)
                      info      (usi_tree_node_info/new-usi-tree-node-info root-node)
                      model     (IcicleModel. root-node info)]
                  (log/debug "Displaying the new view...")
                  (let [view (get-view model)]
                    (SwingUtilities/invokeAndWait (swap-view-fn main-frame parent view)))
                  root-node)
                (catch Error e
                  (do 
                    (log/error (str "Failed to construct a tree mode: " (.getMessage e)))
                    nil)))))]
    (if (or (nil? @model-builder-future)
            (future-done? @model-builder-future)) 
      (reset! model-builder-future (new-future))
      (log/warn "Previous process is still running. Skip the new requrest"))
    @model-builder-future))

