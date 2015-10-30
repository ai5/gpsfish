#!/usr/bin/zsh


coproc ../../../bin/gpsusi --udp-logging localhost:4120

print -p "usi"
for i in {1..11}
do
  read -ep
done

print -p "isready"
read -ep

print -p "usinewgame"

print -p "position startpos moves 2g2f 3c3d 7g7f 4c4d 3i4h 8b4b 5i6h 5a6b 6h7h 6b7b 5g5f 3d3e 2f2e 4d4e 4h5g 4a3b 7i6h 7b8b 9g9f 9c9d 8h2b+ 3a2b 6g6f 4b4d 7h8h 7a7b 6i7h 2a3c 6h6g 2b3a 8i7g 3a4b 6f6e 4b4c 7f7e 4c5b 2h2f 1c1d 1g1f 4d3d 5g6f 3b3a 6f5g 5c5d 5g6f 3a3b 5f5e 5d5e 6f5e 3e3f 3g3f B*5g P*5d 2c2d 6e6d 2d2e 2f2h 6c6d 7e7d 7c7d 5e6d 3d5d 2h5h P*5f P*5c 5b4c 6d5e 5d3d 5e6f 5g1c+ 5h5f 3b4b P*6d 4c5d 1f1e P*5e 6f5e 5d5e 5f5e P*6f 6g6f 3d3f B*5g S*6i 6d6c+ 7b6c 5g1c+ 6i7h+ 8h7h 1a1c P*6g G*7f S*6h B*6d 5e6e 4b5c B*3a 6a7b S*5e 6d5e"

print -p "go infinite"

cat <&p

#print -p "quit"

