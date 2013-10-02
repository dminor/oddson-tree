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
import datetime
import glob
import gzip
import math
import os
import random
import re
import subprocess
import sys

###############################################################################
#
# Experimental Design Parameters
#
###############################################################################
MAXIMUM_BUILD_DEPTH = [0.5, 1.0, 2.0]

###############################################################################
def gunzip(i_name, o_name):
    with gzip.open(i_name, 'rb') as i:
        with open(o_name, 'wb') as o:
            o.write(i.read())

def do_single_run(pts, searches, samples, args):

    print('running: %s|%s|%s' % (pts, searches, samples))

    #uncompress files
    gunzip(pts, 'pts.txt')
    gunzip(samples, 'samples.txt')
    gunzip(searches, 'searches.txt')

    #pull parameters out of filename
    dim = int(re.search('dim_(\d+)', pts).groups()[0])
    npoints = int(re.search('count_(\d+)', pts).groups()[0])
    sigma = float(re.search('sigma_(\d.\d+)', searches).groups()[0])
    sample = int(re.search('sample_(\d+)', samples).groups()[0])
    search = int(re.search('count_(\d+)', searches).groups()[0])
    run = int(re.search('num_(\d+)', samples).groups()[0])

    log = open('log.txt', 'ab')
    log.write('====================\n')
    log.write('%s|%s|%s\n' % (pts, searches, samples))
    log.write('dim: %d\n' % dim)
    log.write('pts: %d\n' % npoints)
    log.write('search size: %d\n' % search)
    log.write('sigma: %f\n' % sigma)
    log.write('sample size: %d\n' % sample)
    log.write('run: %d\n' % run)
    log.write('kdtree: %s\n' % args.kdtree)
    log.write('odds-on tree: %s\n' % args.oddson_tree)
    log.write('k: %d\n' % args.k)

    # run kdtree
    log.write('running kdtree\n')
    log.flush()
    cmd = [args.kdtree, 'pts.txt', 'searches.txt', str(args.k)]
    with open('kdtree.txt', 'wb') as kdtree_out:
        result = subprocess.call(cmd, stdout=kdtree_out, stderr=log)
    log.write('done: %d\n' % result)

    # run odds-on tree for each build depth
    for depth in MAXIMUM_BUILD_DEPTH: 
        actual_depth = int(depth*math.log(float(npoints)))
        log.write('--------------------\n')
        log.write('build depth: %d\n' % actual_depth)
       
        log.write('running odds-on tree\n')
        log.flush()
        cmd = [args.oddson_tree, 'pts.txt', 'samples.txt', str(actual_depth), 'searches.txt', str(args.k)]
        with open('oddson.txt', 'wb') as oddson_out:
            result = subprocess.call(cmd, stdout=oddson_out, stderr=log)
        log.write('done: %d\n' % result)

        # ensure output matches if we are validating
        if args.validate:
            cmd = ['diff', 'kdtree.txt', 'oddson.txt']
            result = subprocess.call(cmd)
            if result != 0:
                log.write('validation error: differences found between kdtree and odds-on tree results\n')
                log.write('aborting...\n')
                log.close()
                print('aborting after validation error.')
                sys.exit(1)

    log.write('\n\n')

    # clean up
    os.remove('pts.txt')
    os.remove('samples.txt')
    os.remove('searches.txt')

    log.close()

if __name__ == '__main__':

    # parse arguments
    parser = argparse.ArgumentParser(description='Run experiments.')
    parser.add_argument('--dim', dest='dim', type=int, default=2,
                        help='Dimension for which to run experiments.')
    parser.add_argument('--k', dest='k', type=int, default=1,
                        help='Number of nearest neighbours for which to search.')
    parser.add_argument('--kdtree', dest='kdtree', required=True,
                        help='Path to kdtree implementation.')
    parser.add_argument('--oddson-tree', dest='oddson_tree', required=True,
                        help='Path to odds-on tree implementation.')
    parser.add_argument('--random-seed', dest='random_seed',
                        help='Random seed.')
    parser.add_argument('--validate', dest='validate', action='store_true',
                        help='Validate results.')
    parser.add_argument('--working-dir', dest='working_dir', default='../data',
                        help='Working directory.')
    args = parser.parse_args()

    if args.working_dir:
        working_dir = os.path.abspath(args.working_dir)
        if not os.path.exists(working_dir):
            print('error: bad working dir: ' + working_dir)
            exit(1)
        os.chdir(working_dir)

    if args.random_seed:
        random.seed(float(args.random_seed))

    if not os.path.exists(args.kdtree):
        print('error: kd-tree path is not valid: ' + args.kdtree)
        sys.exit(1)

    if not os.path.exists(args.oddson_tree):
        print('error: oddson tree path is not valid: ' + args.oddson_tree)
        sys.exit(1)

    searches = glob.glob('search_dim_%02d*' % args.dim)
    pts = glob.glob('pts_dim_%02d*' % args.dim)

    with open('log.txt', 'wb') as log:
        log.write('started: %s\n' % str(datetime.datetime.now()))

    for pt in pts:
        for search in searches:
            # find samples corresponding to search
            sample_glob = 'sample' + search[6:search.find('.txt.gz')] + '_sample*'
            samples = glob.glob(sample_glob)
            for sample in samples:
                do_single_run(pt, search, sample, args)

    with open('log.txt', 'ab') as log:
        log.write('finished: %s\n' % str(datetime.datetime.now()))
