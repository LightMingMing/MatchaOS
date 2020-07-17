#!/bin/bash

# core2duo-v1: Intel(R) Core(TM)2 Duo CPU     T7700  @ 2.40GHz
qemu-system-x86_64  \
  -cpu core2duo-v1 \
  -smp cpus=4,cores=2,threads=2,sockets=1  \
  -m 2048 \
  -boot a -fda bin/matcha.img -hdc bin/matcha.img