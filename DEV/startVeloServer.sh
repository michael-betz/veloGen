#!/bin/bash
screen -S velo -L veloLog.txt bash -c 'python3 veloServer.py --topic "velo1/raw,velo2/raw"'
