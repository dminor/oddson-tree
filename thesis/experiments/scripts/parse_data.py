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
import os
import re
import sqlite3
import sys

def parse_rows(filename):

    dim = 0
    pts = 0
    sigma = 0.0
    sample = 0
    run = 0
    k = 0
    build_depth = 0
    kdtree = False
    ctime = 0.0
    qtime = 0.0
    hits = 0
    backup = 0

    rows = []

    log = open(filename, 'rb')
    for line in log: 
        if line.startswith('dim: '):
            dim = int(line[5:])
        elif line.startswith('pts: '):
            pts = int(line[5:])
        elif line.startswith('sigma: '):
            sigma = float(line[7:])
        elif line.startswith('search size: '):
            search = int(line[13:])
        elif line.startswith('sample size: '):
            sample = int(line[13:])
        elif line.startswith('run: '):
            run = int(line[5:])
        elif line.startswith('k: '):
            k = int(line[3:])
        elif line.startswith('build depth: '):
            build_depth = int(line[13:])
        elif line.startswith('running kdtree'):
            kdtree = True
            hits = 0
            backup = 0
            build_depth = 0
            total = 0
            terminal = 0
            build_nn_queries = 0
        elif line.startswith('running odds-on tree'):
            kdtree = False 
        elif line.startswith('info:'):
            m = re.match('info: tree construction took: (\d+.\d+)', line)
            if m:
                ctime = float(m.groups()[0])
                continue
            m = re.match('info: running queries took: (\d+.\d+)', line)
            if m:
                qtime = float(m.groups()[0])
                continue
            m = re.match('info: hits: (\d+)', line)
            if m:
                hits = int(m.groups()[0])
                continue
            m = re.match('info: backup nodes visited: (\d+)', line)
            if m:
                backup = int(m.groups()[0])
                continue
            m = re.match('info: total nodes: (\d+)', line)
            if m:
                total = int(m.groups()[0])
                continue
            m = re.match('info: terminal nodes: (\d+)', line)
            if m:
                terminal = int(m.groups()[0])
                continue
            m = re.match('info: build nn queries: (\d+)', line)
            if m:
                build_nn_queries = int(m.groups()[0])
                continue
        elif line.startswith('done:'):
            rows.append([dim,pts,sigma,search,sample,build_depth,run,kdtree,ctime,qtime,hits,backup,total,terminal,build_nn_queries])

    log.close()

    return rows

def write_to_csv(rows):
    for row in rows:
        print(','.join(map(str,row)))

def query_db(conn, filter):
    c = conn.cursor()
    c.execute(filter)
    result = [[descriptor[0] for descriptor in c.description]]
    for row in c.fetchall():
        result.append(row)
    c.close()
    return result

def write_to_db(conn, rows):
    try:
        conn.execute('''drop table data''')
    except sqlite3.OperationalError:
        pass

    conn.execute('''create table data
                 (dim int, pts int, sigma real, search int,
                  sample int, build_depth int, run int, 
                  kdtree int, ctime real, qtime real,
                  hits int, backup int, total int, terminal int, build_nn_queries int)''')

    conn.executemany('''insert into data values(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)''', rows)
    conn.commit()

if __name__ == '__main__':

    # parse arguments
    parser = argparse.ArgumentParser(description='Parse experiment data.') 
    parser.add_argument('--database', dest='database', default=':memory:',
                        help='Database file to use.')
    parser.add_argument('--filter', dest='filter',
                        default='select * from data order by pts, sigma, sample, build_depth, run;',
                        help='SQL query used to select data.')
    parser.add_argument('--logfile', dest='logfile',
                        help='Log file to use.')
    args = parser.parse_args()

    if args.database == ':memory:' and not args.logfile:
        print('error: one of --logfile or --database must be specified.')
        sys.exit(1)

    with sqlite3.connect(args.database) as conn:
        if args.logfile:
            if not os.path.isfile(args.logfile):
                print('error: could not find log file: ' + args.logfile)
                sys.exit(1)

            rows = parse_rows(args.logfile)
            write_to_db(conn, rows)

        rows = query_db(conn, args.filter)
        write_to_csv(rows)
