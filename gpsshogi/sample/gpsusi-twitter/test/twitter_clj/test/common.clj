(ns twitter_clj.test.common
  (:use [twitter_clj.common])
  (:use [clojure.test]))

(deftest test-sort-pv []
  (let [coll [{:depth 6, :cp 35, :pv "4f3e 4b5c 3e4f", :first-move "4f3e"}]]
    (is (= coll (sort-pv coll))))
  (let [coll [{:depth 6, :cp 35, :pv "4f3e 4b5c 3e4f", :first-move "4f3e"}
              {:depth 6, :cp 30, :pv "4f3e 4b5c xxxx", :first-move "4f3e"}]]
    (is (= [{:depth 6, :cp 30, :pv "4f3e 4b5c xxxx", :first-move "4f3e"}] (sort-pv coll))))
  (let [coll [{:depth 6, :cp 35, :pv "4f3e 4b5c 3e4f", :first-move "4f3e"}
              {:depth 6, :cp 30, :pv "4f3e 4b5c xxxx", :first-move "4f3e"}
              {:depth 6, :cp 40, :pv "yyyy 4b5c 3ef4", :first-move "yyyy"}
              {:depth 6, :cp 20, :pv "zzzz 4b5c 3ef4", :first-move "zzzz"}]]
    (is (= [{:depth 6, :cp 40, :pv "yyyy 4b5c 3ef4", :first-move "yyyy"}
            {:depth 6, :cp 30, :pv "4f3e 4b5c xxxx", :first-move "4f3e"}
            {:depth 6, :cp 20, :pv "zzzz 4b5c 3ef4", :first-move "zzzz"}] (sort-pv coll)))))

