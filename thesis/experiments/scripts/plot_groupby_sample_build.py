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
    parser.add_argument('--database', dest='database',
                        help='Database file to use.')
    parser.add_argument('--legend-loc', dest='legend_loc', type=int, default=2,
                        help='Set the legend location.')
    parser.add_argument('--measure', dest='measure',
                        choices=['ctime', 'qtime', 'total'], default='total',
                        help='Sample size to use.')
    parser.add_argument('--pts', dest='pts', type=int,
                        help='Point size to use.')
    parser.add_argument('--show', dest='show', action='store_true',
                        help='Show the resulting plot.')
    args = parser.parse_args()


    conn = sqlite3.connect(args.database)
    c = conn.cursor()

    # get distinct build depths for this pt count
    build_depths = []
    c.execute("select distinct(build_depth) from data where pts=%d and kdtree=0 order by build_depth" % args.pts)

    for row in c.fetchall():
        build_depths.append(row[0])

    # we just use the median
    build_depth = build_depths[len(build_depths)/2]

    # get distinct sigmas
    sigmas = []
    c.execute("select distinct(sigma) from data where pts=%d order by sigma" % args.pts)

    for row in c.fetchall():
        sigmas.append(row[0])

    # get distinct samples 
    samples = []
    c.execute("select distinct(sample) from data where pts=%d and kdtree=0 order by sample" % args.pts)

    for row in c.fetchall():
        samples.append(row[0])

    results = {} 

    measure = args.measure
    if args.measure == 'total':
        measure = 'ctime+qtime'

    # get odds-on tree data by build depth
    for sample in samples:
        results[sample] = {}
        for sigma in sigmas:
            c.execute('select terminal, total from data where pts=%d and sample=%d and kdtree=0 and build_depth=%s and sigma=%s order by build_depth' % (args.pts, sample, build_depth, sigma))

            run_data = []
            row = c.fetchone()
            results[sample].setdefault('terminal', []).append(row[0])
            results[sample].setdefault('total', []).append(row[1])

    c.close()
    conn.close()

    ind = np.arange(len(sigmas))
    width = 0.1

    fix, ax = plt.subplots()

    rects = []
    labels = []

    colors = ['c', 'y', 'g', 'r']

    keys = sorted(results.keys())
    for i, result in enumerate(keys):
        p1 = ax.bar(ind+i*width, results[result]['total'], width, color=colors[i], ecolor='k')
        p2 = ax.bar(ind+i*width, results[result]['terminal'], width, color='b', ecolor='k')
        rects.append(p1)
        labels.append('sample %s ' % result) 

    rects.append(p2)
    labels.append('terminal nodes')

    ax.set_xlabel('sigma')
    ax.set_xticks(ind+width)
    ax.set_xticklabels(tuple(sigmas))

    ax.set_ylabel('Nodes')

    ax.legend(tuple(rects), tuple(labels), loc=args.legend_loc)

    dbname = os.path.splitext(os.path.split(args.database)[1])[0]
    plt.savefig('%s_pts%s_groupbysample_%s.eps' % (dbname, args.pts, args.measure))
    if args.show:
        plt.show()
