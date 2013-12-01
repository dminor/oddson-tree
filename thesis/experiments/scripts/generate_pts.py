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
import gzip
import os
import random

###############################################################################
#
# Experimental Design Parameters
#
PT_COUNT = [1000, 10000, 100000]
SEARCH_PT_COUNT = [200000]
SEARCH_SIGMA = [0.5, 0.25, 0.1, 0.05, 0.01]
SAMPLE_PT_COUNT = [0.5, 1.0, 2.0]
NUMBER_OF_RUNS = 20
###############################################################################


def generate_pts(dim, count, fn):
    pts = []
    for i in xrange(0, count):
        pts.append([fn() for d in xrange(0, dim)])

    return pts


def gaussian_pts(dim, count, mean=0.0, sigma=1.0):
    return generate_pts(dim, count, lambda: random.gauss(mean, sigma))


def uniform_pts(dim, count, min=-1.0, max=1.0):
    return generate_pts(dim, count, lambda: random.uniform(min, max))


def write_pts(f, pts):
    f.write('%d %d\n' % (len(pts), len(pts[0])))
    for pt in pts:
        f.write(','.join(map(str, pt)))
        f.write('\n')


if __name__ == '__main__':

    # parse arguments
    parser = argparse.ArgumentParser(description='Create experimental data.')
    parser.add_argument('--dim', dest='dim', type=int, default=2,
                        help='Dimension for which to generate pts.')
    parser.add_argument('--output-dir', dest='output_dir', default='../data',
                        help='Directory in which to write output.')
    parser.add_argument('--random-seed', dest='random_seed',
                        help='Random seed.')
    args = parser.parse_args()

    if args.output_dir:
        output_dir = os.path.abspath(args.output_dir)
        if not os.path.exists(output_dir):
            os.makedirs(output_dir)
        os.chdir(output_dir)

    if args.random_seed:
        random.seed(float(args.random_seed))

    # point sets
    for count in PT_COUNT:
        pts = uniform_pts(args.dim, count)
        name = 'pts_dim_%02d_count_%d.txt.gz' % (args.dim, count)
        with gzip.open(name, 'wb') as f:
            write_pts(f, pts)

    # sample and earch sets
    for sigma in SEARCH_SIGMA:
        for search_pt_count in SEARCH_PT_COUNT:
            pts = gaussian_pts(args.dim, search_pt_count, sigma=sigma)
            name = 'search_dim_%02d_count_%d_sigma_%.3f.txt.gz' % (args.dim, search_pt_count, sigma)
            with gzip.open(name, 'wb') as f:
                write_pts(f, pts)

        for pt_count in PT_COUNT:
            for sample_pt_count in SAMPLE_PT_COUNT:
                actual_pt_count = int(sample_pt_count*pt_count)
                for i in xrange(0, NUMBER_OF_RUNS):
                    pts = gaussian_pts(args.dim, actual_pt_count, sigma=sigma)
                    name = 'sample_dim_%02d_count_%d_sigma_%.3f_sample_%d_num_%d.txt.gz' % (args.dim, pt_count, sigma, actual_pt_count, i)
                    with gzip.open(name, 'wb') as f:
                        write_pts(f, pts)
