# Copyright (c) 2014 Daniel Minor
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
import glob
import numpy
import os
import scipy.stats
import sqlite3

import numpy as np
import matplotlib.pyplot as plt

def mean_confidence_interval(data, confidence=0.95):
    a = numpy.array(data)
    n = len(a)
    m, se = numpy.mean(a), scipy.stats.sem(a)
    h = se * scipy.stats.t._ppf((1+confidence)/2., n-1)
    return m, h 

if __name__ == '__main__':

    # parse arguments
    parser = argparse.ArgumentParser(description='Plot experiment data.')
    parser.add_argument('--database-pattern', dest='database',
                        help='Database glob pattern to use.')
    parser.add_argument('--legend-loc', dest='legend_loc', type=int, default=4,
                        help='Set the legend location.')
    parser.add_argument('--measure', dest='measure',
                        choices=['ctime', 'qtime', 'total'], default='total',
                        help='Sample size to use.')
    parser.add_argument('--pts', dest='pts', type=int,
                        help='Point size to use.')
    parser.add_argument('--sample', dest='sample', type=int,
                        help='Sample size to use.')
    parser.add_argument('--sigma', dest='sigma', type=float,
                        help='Search and sample set sigma to use.')
    parser.add_argument('--show', dest='show', action='store_true',
                        help='Show the resulting plot.')
    args = parser.parse_args()

    build_depth = None
    dims = []
    results = {}
    results['Kd-tree'] = {}
    results['Odds-on Tree'] = {}

    for db in sorted(glob.glob(args.database)):
        conn = sqlite3.connect(db)
        c = conn.cursor()

        # get distinct build depths
        if not build_depth:
            c.execute("select distinct(build_depth) from data where pts=%d and sample=%d and kdtree=0 order by build_depth" % (args.pts, args.sample))

            build_depths = []
            for row in c.fetchall():
                build_depths.append(row[0])
            build_depth = build_depths[len(build_depths)/2]

        # get dimension
        c.execute("select distinct(dim) from data")
        dim = c.fetchone()[0]
        dims.append(dim)

        measure = args.measure
        if args.measure == 'total':
            measure = 'ctime+qtime'

        # get kdtree data
        c.execute("select %s from data where pts=%d and sample=%d and kdtree=1 and sigma=%s" % (measure, args.pts, args.sample, args.sigma))

        run_data = []
        for row in c.fetchall():
            run_data.append(float(row[0]))
        mn, cf = mean_confidence_interval(run_data)
        results['Kd-tree'].setdefault('means', []).append(mn)
        results['Kd-tree'].setdefault('errs', []).append(cf)

        # get odds-on tree data by build depth
        c.execute('select %s from data where pts=%d and sample=%d and kdtree=0 and build_depth=%s and sigma=%s order by build_depth' % (measure, args.pts, args.sample, build_depth, args.sigma))

        run_data = []
        for row in c.fetchall():
            run_data.append(float(row[0]))
        mn, cf = mean_confidence_interval(run_data)
        results['Odds-on Tree'].setdefault('means', []).append(mn)
        results['Odds-on Tree'].setdefault('errs', []).append(cf)

        c.close()
        conn.close()

    ind = np.arange(len(dims))
    width = 0.1

    fix, ax = plt.subplots()

    rects = []
    labels = []

    colors = ['c', 'y', 'g', 'r']

    keys = sorted(results.keys(), reverse=True)
    for i, result in enumerate(keys):
        rect = ax.bar(ind+i*width, results[result]['means'], width,
                      color=colors[i], yerr=results[result]['errs'], ecolor='k')
        rects.append(rect)
        labels.append(result)

    ax.set_xlabel('dimension')
    ax.set_xticks(ind+width)
    ax.set_xticklabels(tuple([str(d) + 'd' for d in dims]))

    if args.measure == 'ctime':
        ax.set_ylabel('construction time (msec)')
    elif args.measure == 'qtime':
        ax.set_ylabel('query time (msec)')
    else:
        ax.set_ylabel('total time (msec)')

    ax.legend(tuple(rects), tuple(labels), loc=args.legend_loc)

    plt.savefig('dims_pts%s_sample%s_sigma%s_%s.eps' %
        (args.pts, args.sample, args.sigma, args.measure))
    if args.show:
        plt.show()
