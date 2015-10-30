(ns gpsdashboard.server_protocol)

(defprotocol ServerProtocol
  "Protocol for a server receiving UDP packets"
  (start    [self] "Start up a server")
  (ready?   [self] "See if a server is ready and working")
  (shutdown [self] "Shutdown a server")
  (join     [self] "Wait for this thread.join"))


