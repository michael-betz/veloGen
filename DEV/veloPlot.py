#!/usr/bin/env python3
''' plot a velogen .csv file '''
from numpy import genfromtxt
from matplotlib.pyplot import close, subplots, show

from argparse import ArgumentParser
from pandas import DataFrame, to_datetime, Timestamp, Timedelta

args = None

lms = {
    'mA': (-750, 750, 'Battery current [mA]'),
    'mV': (5000, 9000, 'Battery voltage [mV]'),
    'cnt': (-10, 100000, 'Wheel counter'),
    'kmh': (-5, 50, 'Speed [km/h]'),
    'km': (None, None, 'Distance [km]')
}


def veloplt(dff, cols=['mA'], **kwargs):
    close()
    fig, axs = subplots(len(cols), 1, sharex=True, figsize=(9, 7))
    if len(cols) == 1:
        axs = [axs]

    for i, col in enumerate(cols):
        dff[col].plot(ax=axs[i], marker='o', **kwargs)
        if col in lms:
            axs[i].set_ylim(lms[col][:2])
            axs[i].set_ylabel(lms[col][2])

    for ax in axs:
        ax.legend()

    # show the last 6 h
    ts_l = dff.index[-1]
    ax.set_xlim(
        left=ts_l - Timedelta(hours=6),
        right=ts_l + Timedelta(minutes=5)
    )

    fig.tight_layout()
    return fig, axs


def main():
    global args
    parser = ArgumentParser(description=__doc__)
    parser.add_argument(
        "csv", type=str, help='.csv file to plot'
    )
    parser.add_argument(
        "--plot", default='kmh,mV',
        help='comma separated list of columns to plot: ' + ','.join(lms.keys())
    )
    args = parser.parse_args()

    # csv to numpy array
    vals = genfromtxt(args.csv, dtype=int, delimiter=',')

    # numpy array to pandas dataframe
    tss = to_datetime(vals[:, 0], unit='s', utc=True).tz_convert('US/Pacific')
    df = DataFrame(vals, index=tss, columns=('ts', 'mV', 'mA', 'cnt'))

    # Sort by timestamp
    dff = df.sort_index()

    # Crop trash data at beginning
    filter_inds = dff.index < Timestamp('2020-10-18 18:59:00-0700')
    filter_inds |= dff.index > Timestamp('2030-10-18 18:59:00-0700')

    # filter out duplicates (identical timestamps)
    filter_inds |= dff.ts.diff() == 0
    print(f"Filtering out {sum(filter_inds)} values")

    dff = dff[~filter_inds]

    conv = 165769 / 1000 / 1000  # pulse to meter
    dff['kmh'] = (dff['cnt'].diff() / dff.ts.diff() * conv * 60 * 60 / 1000)
    dff['km'] = (dff['cnt'] - dff['cnt'].iloc[0]) * conv / 1000

    fig, axs = veloplt(dff, args.plot.split(','))
    show()


if __name__ == '__main__':
    main()
