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
    parser.add_argument('--legend-loc', dest='legend_loc', type=int, default=4,
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

    # get kdtree data
    results['kdtree'] = {}
    for sigma in sigmas:
        # We combine data from all sample sizes here, even though these are
        # separate runs in the original data. Given the low variance, this
        # should not be a problem.
        c.execute("select %s from data where pts=%d and kdtree=1 and sigma=%s" % (measure, args.pts, sigma))

        run_data = []
        for row in c.fetchall():
            run_data.append(float(row[0]))
        mn, cf = mean_confidence_interval(run_data)
        results['kdtree'].setdefault('means', []).append(mn)
        results['kdtree'].setdefault('errs', []).append(cf)

    # get odds-on tree data by build depth
    for sample in samples:
        results[sample] = {}
        for sigma in sigmas:
            c.execute('select %s from data where pts=%d and sample=%d and kdtree=0 and build_depth=%s and sigma=%s order by build_depth' % (measure, args.pts, sample, build_depth, sigma))

            run_data = []
            for row in c.fetchall():
                run_data.append(float(row[0]))
            mn, cf = mean_confidence_interval(run_data)
            results[sample].setdefault('means', []).append(mn)
            results[sample].setdefault('errs', []).append(cf)

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
        rect = ax.bar(ind+i*width, results[result]['means'], width, color=colors[i], yerr=results[result]['errs'], ecolor='k')
        rects.append(rect)
        if result != 'kdtree':
            labels.append('sample %s ' % result) 
        else:
            labels.append(result)

    ax.set_xlabel('sigma')
    ax.set_xticks(ind+width)
    ax.set_xticklabels(tuple(sigmas))

    if args.measure == 'ctime':
        ax.set_ylabel('construction time (msec)')
    elif args.measure == 'qtime':
        ax.set_ylabel('query time (msec)')
    else:
        ax.set_ylabel('total time (msec)')

    ax.legend(tuple(rects), tuple(labels), loc=args.legend_loc)

    dbname = os.path.splitext(os.path.split(args.database)[1])[0]
    plt.savefig('%s_pts%s_groupbysample_%s.eps' % (dbname, args.pts, args.measure))
    if args.show:
        plt.show()
