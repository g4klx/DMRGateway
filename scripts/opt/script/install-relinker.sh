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
