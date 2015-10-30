(ns gpsdashboard.ema)


(defprotocol EMAProtocol
  "Protocol for Exponential Moving Average"
  (update [self v] "Append a new value")
  (value [self]    "Return the current value"))

(defrecord EMA [#^double alpha #^double value]
  EMAProtocol
  (update [_ v]
    (let [new-v (+ (* alpha v)
                   (* (- 1 alpha) value))]
      (EMA. alpha new-v)))
  (value [self] ; Protocol hides the original .value method !?
    (:value self))) 

(defn new-EMA
  "Return a new EMA instance."
  [alpha]
  (EMA. alpha 0))

