#!/bin/bash
screen -r velo -X quit

# CMD='./velo_server.py --topic "velo1/raw,velo2/raw"'
CMD='./velo_server2.py -t velo1/raw'

screen -dmLS velo -Logfile velo_log.txt $CMD
