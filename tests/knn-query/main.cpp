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
    static int dim;

    double *coords;
    long id;

    Point()
    {
        coords = new double[dim];
    };

    virtual ~Point()
    {
        delete[] coords;
    } 

    Point(const Point &other)
    { 
        coords = new double[dim];

        for (int d = 0; d < dim; ++d) {
            coords[d] = other.coords[d];
        }
    }

    void operator=(const Point &other)
    {
        for (int d = 0; d < dim; ++d) {
            coords[d] = other.coords[d];
        } 
    }

    double operator[](size_t idx) const {return coords[idx];}
    double &operator[](size_t idx) {return coords[idx];}
};

int Point::dim = 0;

Point *read_points(const char *filename, int &count, int &dim)
{ 
    std::ifstream ptf(filename);

    if (!ptf) {
        std::cout << "error: could not open file: " << filename << std::endl;
        exit(1); 
    }

    ptf >> count;
    ptf >> dim;

    if (count < 0) {
        std::cerr << "error: invalid point count: " << count << std::endl;
        exit(1);
    }

    if (dim < 2) {
        std::cerr << "error: invalid dimension: " << dim << std::endl;
        exit(1);
    }

    Point::dim = dim;

    Point *pts = new Point[count]; 
    for (int i = 0; i < count; ++i) { 
        char c;
        double value;

        for (int d = 0; d < dim; ++d) {
            ptf >> value; pts[i][d] = value;
            if (d < dim - 1) ptf >> c; 
        }
    }

    ptf.close();

    return pts; 
}

int main(int argc, char **argv)
{ 
    if (argc < 4) {
        fprintf(stderr,
            "usage: knn <pts> <samples> <maxdepth> [queries] [nn] [epsilon]\n)");
        return 1;
    }

    int n, pt_dim, sample_dim, m, p, query_dim;

    Point *pts = read_points(argv[1], n, pt_dim);

    if (!pts) {
        fprintf(stderr, "error: could not read points file: %s\n", argv[1]);
        exit(1); 
    }

    Point *sample = read_points(argv[2], m, sample_dim);

    if (!sample) {
        fprintf(stderr, "error: could not read sample file: %s\n", argv[2]);
        exit(1); 
    }

    if (pt_dim != sample_dim) {
        fprintf(stderr, "error: point dim does not match sample dim\n");
        exit(1); 
    } 

    //max depth
    size_t maxdepth = (size_t)atoi(argv[3]);

    OddsonTree<Point> oot(Point::dim, pts, n, sample, m, maxdepth);

    if (argc < 5) {
        return 1;
    }

    Point *queries = read_points(argv[4], p, query_dim); 

    if (!queries) {
        fprintf(stderr, "error: could not read query file: %s\n", argv[3]);
        exit(1); 
    }

    if (pt_dim != query_dim) {
        fprintf(stderr, "error: query dim does not match point dim\n");
        exit(1); 
    } 

    //how many nearest neighbours to retrieve
    int nn = 1;
    if (argc >= 6) nn = atoi(argv[5]);

    //read query epsilon
    double epsilon = 0.0;
    if (argc == 7) epsilon = atof(argv[6]);

    //run queries
    if (nn == 1) {

        for (int i = 0; i < p; ++i) { 

            std::list<std::pair<Point *, double> > qr = oot.nn(queries[i], epsilon);  

            std::cout << "query " << i << ": (";
            for (int d = 0; d < Point::dim; ++d) { 
                std::cout << queries[i][d];
                if (d + 1 < Point::dim) std::cout << ", ";
            }
            std::cout << ")\n";

            for (std::list<std::pair<Point *, double> >::iterator itor = qr.begin(); itor != qr.end(); ++itor) {
                std::cout << "("; 
                for (int d = 0; d < Point::dim; ++d) {
                    std::cout << (*itor->first)[d];
                    if (d + 1 < Point::dim) std::cout << ", ";
                }
                std::cout << ") " << itor->second << "\n"; 
            } 
        }

    } else {

        for (int i = 0; i < p; ++i) { 

            std::list<std::pair<Point *, double> > qr = oot.knn(nn, queries[i], epsilon);  

            std::cout << "query " << i << ": (";
            for (int d = 0; d < Point::dim; ++d) { 
                std::cout << queries[i][d];
                if (d + 1 < Point::dim) std::cout << ", ";
            }
            std::cout << ")\n";

            for (std::list<std::pair<Point *, double> >::iterator itor = qr.begin(); itor != qr.end(); ++itor) {
                std::cout << "("; 
                for (int d = 0; d < Point::dim; ++d) {
                    std::cout << (*itor->first)[d];
                    if (d + 1 < Point::dim) std::cout << ", ";
                }
                std::cout << ") " << itor->second << "\n"; 
            } 
        }
    }

    std::cout << "done." << std::endl;

    delete[] pts;
    delete[] sample;
    delete[] queries; 

    return 0;
}
