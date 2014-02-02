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
import scipy.stats
import sqlite3

if __name__ == '__main__':

    # parse arguments
    parser = argparse.ArgumentParser(description='Run t-test on data.')
    parser.add_argument('--database', dest='database',
                        help='Database file to use.')
    parser.add_argument('--measure', dest='measure',
                        choices=['ctime', 'qtime', 'total'], default='total',
                        help='Sample size to use.')
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
    c.execute("select distinct(build_depth) from data where pts=%d and sample=%d and kdtree=0 order by build_depth" % (args.pts, args.sample))

    for row in c.fetchall():
        build_depths.append(row[0])

    results = {} 

    measure = args.measure
    if args.measure == 'total':
        measure = 'ctime+qtime'

    # get kdtree data
    results['kdtree'] = {}
    for sigma in sigmas:
        c.execute("select %s from data where pts=%d and sample=%d and kdtree=1 and sigma=%s" % (measure, args.pts, args.sample, sigma))

        run_data = []
        for row in c.fetchall():
            run_data.append(float(row[0]))
        results['kdtree'][sigma] = run_data

    # get odds-on tree data by build depth
    for depth in build_depths:
        results[depth] = {}
        for sigma in sigmas:
            c.execute('select %s from data where pts=%d and sample=%d and kdtree=0 and build_depth=%s and sigma=%s order by build_depth' % (measure, args.pts, args.sample, depth, sigma))

            run_data = []
            for row in c.fetchall():
                run_data.append(float(row[0]))
            results[depth][sigma] = run_data

    c.close()
    conn.close()

    for result in results:
        if result != 'kdtree':
            for sigma in sigmas:
                print('t-test for build depth %d vs kdtree. sigma: %f' % (result, sigma))
                two_sample = scipy.stats.ttest_ind(results[result][sigma],
                                results['kdtree'][sigma], equal_var=False)
                print('The t-statistic is %.3f and the p-value is %f (assuming unequal variance.)' % two_sample)

