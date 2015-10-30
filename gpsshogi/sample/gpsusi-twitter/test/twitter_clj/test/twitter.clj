(ns twitter_clj.test.twitter
  (:use [twitter_clj.twitter])
  (:use [clojure.test]))


(deftest test-trancate-140-characters []
  (is (= "123" (trancate-140-characters "123")))
  (let [s (str (apply str (repeat 14 "1234567890")) "XYZ")]
    (is (= (apply str (repeat 14 "1234567890"))
           (trancate-140-characters s))))
  (let [s (str (apply str (repeat 14 "あいうえおかきくけこ")) "わをん")]
    (is (= (apply str (repeat 14 "あいうえおかきくけこ"))
           (trancate-140-characters s)))))

