#!/bin/bash

./log_elf.sh ./autobleem-gui

LD_LIBRARY_PATH=/tmp/lib ./autobleem-gui  /media > /media/System/Logs/AB_out.txt 2> /media/System/Logs/AB_err.txt

