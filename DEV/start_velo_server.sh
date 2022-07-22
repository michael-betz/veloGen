#!/bin/bash
screen -r velo -X quit

# CMD = 'python3 velo_server.py --topic "velo1/raw,velo2/raw"'
CMD = 'python3 velo_server2.py -t velo1/raw'

screen -dmS velo -L velo_log.txt bash -c $CMD
