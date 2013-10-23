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

"""
python parse_data.py --database dim2.db --filter "select pts, sample, kdtree, build_depth, sigma, avg(qtime), max(qtime), min(qtime) from data where pts=1000 and sample=2000 group by pts, sample, kdtree, build_depth, sigma" > pts1000_sample2000.csv
"""
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
    parser.add_argument('--pts', dest='pts', type=int,
                        help='Point size to use.')
    parser.add_argument('--sample', dest='sample', type=int,
                        help='Sample size to use.')
    args = parser.parse_args()


    conn = sqlite3.connect(args.database)
    c = conn.cursor()

    # get distinct sigmas
    sigmas = []
    c.execute("select distinct(sigma) from data where pts=%d and sample=%d order by sigma" % (args.pts, args.sample))

    for row in c.fetchall():
        sigmas.append(row[0])

    # get distinct build depths
    build_depths = []
    c.execute("select distinct(build_depth) from data where pts=%d and sample=%d order by build_depth" % (args.pts, args.sample))

    for row in c.fetchall():
        build_depths.append(row[0])

    results = {} 

    # get kdtree data
    results['kdtree'] = {}
    for sigma in sigmas:
        c.execute("select qtime from data where pts=%d and sample=%d and kdtree=1 and sigma=%s order by build_depth" % (args.pts, args.sample, sigma))

        run_data = []
        for row in c.fetchall():
            run_data.append(float(row[0]))
        mn, cf = mean_confidence_interval(run_data)
        results['kdtree'].setdefault('means', []).append(mn)
        results['kdtree'].setdefault('errs', []).append(cf)

    # get odds-on tree data by build depth
    for depth in build_depths:
        results[depth] = {}
        for sigma in sigmas:
            c.execute('select qtime from data where pts=%d and sample=%d and build_depth=%s and sigma=%s order by build_depth' % (args.pts, args.sample, depth, sigma))

            run_data = []
            for row in c.fetchall():
                run_data.append(float(row[0]))
            mn, cf = mean_confidence_interval(run_data)
            results[depth].setdefault('means', []).append(mn)
            results[depth].setdefault('errs', []).append(cf)

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
            labels.append('depth %s ' % result) 
        else:
            labels.append(result)

    ax.set_xlabel('sigma')
    ax.set_xticks(ind+width)
    ax.set_xticklabels(tuple(sigmas))
    ax.set_ylabel('query time (msec)')

    ax.legend(tuple(rects), tuple(labels))

    plt.show()
