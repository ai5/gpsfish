(ns twitter_clj.test.ki2
  (:use [twitter_clj.ki2])
  (:use [clojure.test]))


(deftest test-show []
  (start-ki2-engine "../../../gpsshogi/bin/gpsusi")
  (set-position "position startpos moves 7g7f 3c3d")
  (is (= "▲２六歩△８四歩▲２五歩" (show "2g2f 8c8d 2f2e")))
  (is (= "▲２六歩(詰めろ)△８四歩▲２五歩" (show "2g2f(^) 8c8d 2f2e")))
  (is (= "▲２六歩△８四歩(詰めろ)▲２五歩" (show "2g2f 8c8d(^) 2f2e")))
  (is (= "▲２六歩△８四歩▲２五歩(詰めろ)" (show "2g2f 8c8d 2f2e(^)")))
  (stop-ki2-engine))

