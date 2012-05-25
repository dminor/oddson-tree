/*
Copyright (c) 2011 Daniel Minor 

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include <cstdio>
#include <cstdlib>

#include <algorithm>
#include <fstream>
#include <iostream>

#include "oddson_tree.h"

struct Point {

    double v[2]; 
    long id;

    double &operator[](const int &index) 
    { 
        return v[index]; 
    } 

    const double &operator[](const int &index) const 
    { 
        return v[index]; 
    } 
};

Point *read_points(FILE *f, int *pt_count)
{
    int err;
    char buf[80];
    err = fscanf(f, "%d", pt_count);
    fgets(buf, 80, f);

    if (err != 1 || pt_count < 0) {
        fprintf(stderr, "error: invalid point count %d\n", *pt_count);
        return 0;
    }

    Point *pts = new Point[*pt_count];

    double x, y;
    for (int i = 0; i < *pt_count; ++i) {
        err = fscanf(f, "%lf, %lf", &x, &y);
        if (err != 2) {
            fprintf(stderr, "error: invalid points in file\n"); 
        }
        pts[i][0] = x;
        pts[i][1] = y; 
        pts[i].id = i;
    }

    return pts; 
}

const int dim = 2;

int main(int argc, char **argv)
{ 
    if (argc < 3) {
        fprintf(stderr,
            "usage: knn <pts> <samples> [queries] [nn] [epsilon]\n)");
        return 1;
    }

    int n, m, p;

    FILE *f = fopen(argv[1], "r");

    if (!f) {
        fprintf(stderr, "error: could not open points file: %s\n", argv[1]);
        exit(1); 
    }

    Point *pts = read_points(f, &n);
    if (!pts) exit(1);

    fclose(f);

    f = fopen(argv[2], "r");

    if (!f) {
        fprintf(stderr, "error: could not open sample file: %s\n", argv[2]);
        exit(1); 
    }

    Point *sample = read_points(f, &m);
    if (!sample) exit(1);

    fclose(f);

    OddsonTree<Point> oot(2, pts, n, sample, m);

    if (argc < 4) {
        return 1;
    }

    f = fopen(argv[3], "r");

    if (!f) {
        fprintf(stderr, "error: could not open query file: %s\n", argv[3]);
        exit(1); 
    }

    Point *queries = read_points(f, &p); 

    //how many nearest neighbours to retrieve
    int nn = 1;
    if (argc >= 5) nn = atoi(argv[4]);

    //read query epsilon
    double epsilon = 0.0;
    if (argc == 6) epsilon = atof(argv[5]);

    //run queries
    if (nn == 1) {

        for (int i = 0; i < p; ++i) { 

            std::list<std::pair<Point *, double> > qr = oot.nn(queries[i], epsilon);  

            std::cout << "query " << i << ": (";
            for (int d = 0; d < dim; ++d) { 
                std::cout << queries[i][d];
                if (d + 1 < dim) std::cout << ", ";
            }
            std::cout << ")\n";

            for (std::list<std::pair<Point *, double> >::iterator itor = qr.begin(); itor != qr.end(); ++itor) {
                std::cout << "("; 
                for (int d = 0; d < dim; ++d) {
                    std::cout << (*itor->first)[d];
                    if (d + 1 < dim) std::cout << ", ";
                }
                std::cout << ") " << itor->second << "\n"; 
            } 
        }

    } else {

        for (int i = 0; i < p; ++i) { 

            std::list<std::pair<Point *, double> > qr = oot.knn(nn, queries[i], epsilon);  

            std::cout << "query " << i << ": (";
            for (int d = 0; d < dim; ++d) { 
                std::cout << queries[i][d];
                if (d + 1 < dim) std::cout << ", ";
            }
            std::cout << ")\n";

            for (std::list<std::pair<Point *, double> >::iterator itor = qr.begin(); itor != qr.end(); ++itor) {
                std::cout << "("; 
                for (int d = 0; d < dim; ++d) {
                    std::cout << (*itor->first)[d];
                    if (d + 1 < dim) std::cout << ", ";
                }
                std::cout << ") " << itor->second << "\n"; 
            } 
        }
    }

    std::cout << "done." << std::endl;

    delete[] pts;
    delete[] queries; 

    return 0;
}
