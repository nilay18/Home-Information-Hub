#!/usr/bin/env bash

PID=$(ps -ef | grep 'sudo python3.7 app.py' | grep -v grep | awk -F' ' '{print $2}')

if [ -n "$PID" ]
  then
    sudo kill $PID
fi
(nohup sudo python3.7 app.py &) > /dev/null
