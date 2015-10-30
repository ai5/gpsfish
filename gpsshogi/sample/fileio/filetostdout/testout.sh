#!/usr/bin/zsh

for i in {0000..1000}; do echo `date` > tmp; mv tmp "hoge/$i.txt"; done
