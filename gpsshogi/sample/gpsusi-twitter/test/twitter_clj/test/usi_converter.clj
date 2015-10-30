(ns twitter_clj.test.usi_converter
  (:use [twitter_clj.usi_converter])
  (:use [clojure.test]))

(deftest test-parse-move []
  (let [lines ["1 ２六歩(27)"
               "2 ８四歩(83)"
               "3 ２五歩(26)"
               "4 ８五歩(84)"
               "5 ２四歩(25)"
               "6 同　歩(23)"
               "7 同　飛(28)"
               "8 ２三歩打"
               "9 同飛成(24)"
               "10 １二香(11)"
               "11 同　龍(23)"]]
    (is (= ["2g2f" "8c8d" "2f2e" "8d8e" "2e2d" "2c2d" "2h2d"
            "P*2c" "2d2c+" "1a1b" "2c1b"]
           (parse-moves lines)))))
