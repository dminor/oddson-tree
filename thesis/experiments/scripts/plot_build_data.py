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
    parser.add_argument('--depth', dest='depth', type=int,
                        help='Build depth to use.')
    parser.add_argument('--show', dest='show', action='store_true',
                        help='Show the resulting plot.')
    args = parser.parse_args()


    conn = sqlite3.connect(args.database)
    c = conn.cursor()

    # get distinct sigmas
    sigmas = []
    c.execute("select distinct(sigma) from data where pts=%d and sample=%d order by sigma" % (args.pts, args.sample))

    for row in c.fetchall():
        sigmas.append(row[0])

    results = {} 

    measures = ['terminal', 'total', 'build_nn_queries']
    for measure in measures:
        results[measure] = {}

    # get odds-on tree data by build depth
    for sigma in sigmas:
        c.execute('select %s from data where pts=%d and sample=%d and kdtree=0 and build_depth=%s and sigma=%s order by build_depth' % (','.join(measures), args.pts, args.sample, args.depth, sigma))

        rows = []
        for row in c.fetchall():
            rows.append(row)

        for i, measure in enumerate(measures):
            run_data = []
            for row in rows:
                run_data.append(float(row[i]))
            mn, cf = mean_confidence_interval(run_data)
            results[measure].setdefault('means', []).append(mn)
            results[measure].setdefault('errs', []).append(cf)

    c.close()
    conn.close()

    ind = np.arange(len(sigmas))
    width = 0.1

    fix, ax = plt.subplots()

    rects = []
    labels = []

    colors = ['c', 'y', 'g', 'r']

    for i, result in enumerate(measures):
        rect = ax.bar(ind+i*width, results[result]['means'], width, color=colors[i], yerr=results[result]['errs'], ecolor='k')
        rects.append(rect)
        if result == 'build_nn_queries':
            labels.append('required nn queries')
        else:
            labels.append('%s nodes' % result) 

    ax.set_xlabel('sigma')
    ax.set_xticks(ind+width)
    ax.set_xticklabels(tuple(sigmas))

    ax.set_ylabel('total nodes')

    ax.legend(tuple(rects), tuple(labels), loc=4)
    plt.savefig('pts%s_sample%s_builddata.eps' % (args.pts, args.sample))
    if args.show:
        plt.show()
