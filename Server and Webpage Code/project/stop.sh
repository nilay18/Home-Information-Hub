#!/usr/bin/env bash

sudo kill $(ps -ef | grep 'sudo python3.7 app.py' | grep -v grep | awk -F' ' '{print $2}')
