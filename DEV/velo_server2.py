#!/usr/bin/env python3
'''
MQTT client which subscribes to the velogen/raw topic, takes the data,
sanity checks and massages it a bit and sends it to grafana / database
'''
from argparse import ArgumentParser
from paho.mqtt.client import Client, connack_string
from struct import unpack
from datetime import datetime

from influxdb_client import InfluxDBClient
from influxdb_client.client.write_api import SYNCHRONOUS

from secret import token, organization, bucket

args = None
write_api = None


def on_disconnect(client, userdata, rc):
    print("Disconnected", connack_string(rc))


def on_connect(client, userdata, flags, rc):
    print("Connected", connack_string(rc))

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
        ts, volts, amps, speed, unused, cnt = \
            unpack("IHhHHI", msg.payload[i * 16: (i + 1) * 16])
        # only accept timestamps +- 1 year from now
        if abs(datetime.now().timestamp() - ts) < (365 * 24 * 60 * 60):
            write_api.write(
                bucket,
                organization,
                f'{msg.topic} vbatt={volts},ibatt={amps},ticks={cnt},speed={speed / 100} {ts * 1e9:.0f}'
            )
        else:
            print('invalid ts', ts)


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
    parser.add_argument(
        "--user", default=None,
        help="username"
    )
    parser.add_argument(
        "--pw", default=None,
        help="password"
    )

    args = parser.parse_args()

    client = Client()
    client.on_connect = on_connect
    client.on_disconnect = on_disconnect
    client.on_message = on_message
    if args.user is not None and args.pw is not None:
        client.username_pw_set(args.user, args.pw)
    client.connect(args.host)

    # For influxdb2 only
    db = InfluxDBClient(
        url='http://localhost:8086',
        token=token,
        debug=args.d
    )
    write_api = db.write_api(write_options=SYNCHRONOUS)

    while True:
        client.loop(timeout=1.0)


if __name__ == '__main__':
    main()
