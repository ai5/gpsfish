(ns gpsdashboard.test.common
  (:require [gpsdashboard.common :as common])
  (:use [midje.sweet]))

(fact "delimiter? nil"
  (common/delimiter? nil) => false)

