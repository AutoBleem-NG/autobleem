#!/bin/bash

export HOME=/media/Apps/amiberry
cd /media/Apps/amiberry
cp -f conf/default/* conf/

/media/Autobleem/bin/autobleem/log_elf.sh ./amiberry

./amiberry  -config=./conf/AB-A500.uae

