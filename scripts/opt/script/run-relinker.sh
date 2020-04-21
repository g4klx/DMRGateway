#!/bin/bash

# This is just a nice way to start the script 


while /bin/true; do
    /opt/script/./relink-repeater.sh >> /var/log/script/checkBMAPI.log 2>&1
done &