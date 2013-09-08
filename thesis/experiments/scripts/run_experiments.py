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
import glob
import gzip
import math
import os
import random
import re
import subprocess

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
    gunzip(searches, 'samples.txt')
    gunzip(samples, 'searches.txt')

    log = open('log.txt', 'ab')
    null = open('/dev/null', 'ab')

    log.write(pts + '\n')
    log.write(searches + '\n')
    log.write(samples + '\n')
    log.write('kdtree: %s\n' % args.kdtree)
    log.write('odds-on tree: %s\n' % args.oddson_tree)
    log.write('k: %d\n' % args.k)

    npoints = float(re.search('count_(\d+)', pts).groups()[0])

    for depth in MAXIMUM_BUILD_DEPTH: 
        actual_depth = int(depth*math.log(npoints))
        log.write('build depth: %d\n' % actual_depth)

        # run kdtree
        log.write('running kdtree\n')
        log.flush()
        cmd = [args.kdtree, 'pts.txt', 'searches.txt', str(args.k)]
        result = subprocess.call(cmd, stdout=null, stderr=log)
        
        # run odds-on tree
        log.write('running odds-on tree\n')
        log.flush()
        cmd = [args.oddson_tree, 'pts.txt', 'samples.txt', str(actual_depth), 'searches.txt', str(args.k)]
        result = subprocess.call(cmd, stdout=null, stderr=log)

    # clean up
    os.remove('pts.txt')
    os.remove('samples.txt')
    os.remove('searches.txt')

    log.close()
    null.close()

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

    searches = glob.iglob('search_dim_%02d*' % args.dim)
    pts = glob.iglob('pts_dim_%02d*' % args.dim)

    log = open('log.txt', 'wb')
    log.write('start.\n')
    for pt in pts:
        for search in searches:
            # find samples corresponding to search
            sample_glob = 'sample' + search[6:search.find('.txt.gz')] + '_sample*'
            samples = glob.glob(sample_glob)
            for sample in samples:
                do_single_run(pt, search, sample, args)

            break
    log.write('done.\n')
    log.close()
