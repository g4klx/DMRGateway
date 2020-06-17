#!/bin/bash
mkdir /var/log/script
sleep 1
cp /opt/script/var-log-script.tgz /var/log/script/
sleep 1
cd /var/log/script
sleep 1
tar -xvf var-log-script.tgz
echo -n "" > /var/log/script/static-slot1.txt
echo -n "" > /var/log/script/static-slot2.txt
echo -n "" > /var/log/script/slot1.txt
echo -n "" > /var/log/script/slot2.txt



