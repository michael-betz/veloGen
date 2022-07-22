#!/usr/bin/env python3
'''
MQTT client which subscribes to the velogen/raw topic, takes the data,
sanity checks and massages it a bit and sends it to grafana / database
'''
from argparse import ArgumentParser
from paho.mqtt.client import Client
from struct import unpack
from datetime import datetime

from influxdb_client import InfluxDBClient
from influxdb_client.client.write_api import SYNCHRONOUS

from secret import token, organization, bucket

args = None
write_api = None


def on_disconnect(client, userdata, rc):
    print("Disconnected", rc)
    client.connect(args.host)


def on_connect(client, userdata, flags, rc):
    print("Connected", rc)

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    for t in args.t:
        print('subscribing to', t)
        client.subscribe(t)


# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    if args.d:
        print(msg.topic, msg.payload)

    ll = len(msg.payload)
    if (ll % 16) > 0:
        print('truncated message', ll)
        return

    for i in range(ll // 16):
        d = unpack("Iiii", msg.payload[i * 16: (i + 1) * 16])
        # only accept timestamps +- 1 year from now
        if abs(datetime.now().timestamp() - d[0]) < (365 * 24 * 60):
            write_api.write(
                bucket,
                organization,
                f'{msg.topic} vbatt={d[1]},ibatt={d[2]},ticks={d[3]} {d[0] * 1e9:.0f}'
            )
        else:
            print('invalid ts', d)


def main():
    global args, write_api
    parser = ArgumentParser(description=__doc__)
    parser.add_argument(
        "--host", default='localhost',
        help='hostname of mqtt broker'
    )
    parser.add_argument(
        "-t", action='append', default=['velogen/raw'],
        help='MQTT topic to subscribe to. Can be put multiple times.'
    )
    parser.add_argument(
        "-d", action='store_true',
        help='enable debug output'
    )

    args = parser.parse_args()

    c = Client()
    c.on_connect = on_connect
    c.on_disconnect = on_disconnect
    c.on_message = on_message
    c.connect(args.host)

    # For influxdb2 only
    db = InfluxDBClient(
        url='http://localhost:8086',
        token=token,
        debug=args.d
    )
    write_api = db.write_api(write_options=SYNCHRONOUS)

    while True:
        c.loop(1.0)


if __name__ == '__main__':
    main()
