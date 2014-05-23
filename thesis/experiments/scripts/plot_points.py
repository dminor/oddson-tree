# Copyright (c) 2013 Daniel Minor
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

import argparse
import numpy
import scipy.stats
import sqlite3

import numpy as np
import matplotlib.pyplot as plt


if __name__ == '__main__':

    # parse arguments
    parser = argparse.ArgumentParser(description='Plot experiment data.')
    parser.add_argument('--pts', dest='pts',
                        help='Points file to use.')
    parser.add_argument('--sample', dest='sample',
                        help='Sample file to use.')
    parser.add_argument('--show', dest='show', action='store_true',
                        help='Show the resulting plot.')
    args = parser.parse_args()

    # read points
    xs = []
    ys = []
    with open(args.pts, 'r') as pts:
        pts.readline()
        for line in pts:
            x, y = line.strip().split(',')
            xs.append(float(x))
            ys.append(float(y))

    plt.scatter(xs, ys)

    # read samples
    xs = []
    ys = []
    with open(args.sample, 'r') as pts:
        pts.readline()
        for line in pts:
            x, y = line.strip().split(',')
            xs.append(float(x))
            ys.append(float(y))

    plt.scatter(xs, ys, c='r')

    plt.savefig('pts_plot.eps')
    if args.show:
        plt.show()
