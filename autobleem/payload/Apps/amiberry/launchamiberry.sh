#!/bin/bash

cd /media/Apps/amiberry
cp -f conf/default/* conf/
chmod +x ./amiberry

/media/Autobleem/bin/autobleem/log_elf.sh ./amiberry

./amiberry  -config=./conf/AB-A500.uae

