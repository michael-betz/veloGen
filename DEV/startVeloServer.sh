#!/bin/bash
screen -r velo -X quit
#pkill veloServer.py  # doesn't do anything
screen -dmS velo -L veloLog.txt bash -c './veloServer.py --topic "velo1/raw,velo2/raw"'
