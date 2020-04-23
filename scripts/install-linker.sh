#!/bin/bash




cd /
rm -rf DMRGa*
cd /opt 
rm -rf script
cd /var/log
rm -rf script

cd /boot
cp DMRGateway*.tgz /
cd /
tar -xvf DMRGateway*.tgz
systemctl daemon-reload
echo -n "" > /var/log/script/static-slot1.txt
echo -n "" > /var/log/script/static-slot2.txt
echo -n "" > /var/log/script/slot1.txt
echo -n "" > /var/log/script/slot2.txt
