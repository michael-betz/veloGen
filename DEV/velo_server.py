#!/usr/bin/env python3
'''
MQTT client which subscribes to the velogen/raw topic, takes the data,
sanity checks and massages it a bit and appends it to a .csv file.
Appends to one .csv / topic
'''
from collections import defaultdict
from argparse import ArgumentParser
from paho.mqtt.client import Client
from struct import unpack
from datetime import datetime
from numpy import array, int32, diff, savetxt
import atexit

args = None
tmpDatas = defaultdict(list)
rx_ts_dict = dict()


def on_disconnect(client, userdata, rc):
    print("Disconnected", rc)
    for tp in tmpDatas:
        clean_write(tp)
        rx_ts_dict[tp] = None
    client.connect(args.host)


def on_connect(client, userdata, flags, rc):
    print("Connected", rc)

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    for tp in args.topic.split(','):
        client.subscribe(tp)


# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    rx_ts_dict[msg.topic] = datetime.now().timestamp()
    print(msg.topic, msg.payload)

    for i in range(len(msg.payload) // 16):
        tmp = unpack("Iiii", msg.payload[i * 16: (i + 1) * 16])
        # only accept timestamps +- 1 year from now
        if abs(datetime.now().timestamp() - tmp[0]) < 31536000:
            tmpDatas[msg.topic].append(tmp)
        else:
            print('invalid ts:', tmp)


def clean_write(tp):
    ''' clean and append tmpDatas to .csv, tp = topic string '''
    # sort by timestamp and remove duplicates
    arr = array(tmpDatas[tp], dtype=int32)
    tmpDatas[tp].clear()
    if arr.size <= 4:
        return

    arr.sort(0)
    goodInds = diff(arr[:, 0], prepend=1) != 0
    arr = arr[goodInds, :]

    # append to .csv file
    fName = tp.split('/')[0] + '.csv'
    print('appending', arr.shape[0], 'records to', fName)
    with open(fName, 'a') as f:
        savetxt(f, arr, delimiter=', ', fmt='%d')


def commit():
    for tp, rts in rx_ts_dict.items():
        if rts is not None:
            # dump to file after 30 s of silence
            clean_write(tp)
            rx_ts_dict[tp] = None


def main():
    global args
    parser = ArgumentParser(description=__doc__)
    parser.add_argument(
        "--host", default='localhost',
        help='hostname of mqtt broker'
    )
    parser.add_argument(
        "--topic", default='velogen/raw',
        help='comma separated list of MQTT topics to subscribe to'
    )
    args = parser.parse_args()

    c = Client()
    c.on_connect = on_connect
    c.on_disconnect = on_disconnect
    c.on_message = on_message
    c.connect(args.host)

    atexit.register(commit)

    while True:
        ts = datetime.now().timestamp()
        for tp, rts in rx_ts_dict.items():
            if rts is not None:
                # dump to file after 30 s of silence
                if ts - rts > 30:
                    clean_write(tp)
                    rx_ts_dict[tp] = None
        c.loop(1.0)


if __name__ == '__main__':
    main()
