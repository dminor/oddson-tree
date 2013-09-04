/*
Copyright (c) 2010 Daniel Minor 

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

#include "kdtree.h"

struct Point {
    static int dim;
    double *coords;

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
    int res; 
    FILE *f = fopen(filename, "r");
    if (!f) {
        fprintf(stderr, "error: could not open file: %s", filename);
        exit(1); 
    }

    res = fscanf(f, "%d %d\n", &count, &dim);
    if (res != 2) {
        fprintf(stderr, "error: invalid header: %s", filename);
        exit(1); 
    }

    if (count < 0) {
        fprintf(stderr, "error: invalid point count: %s: %d", filename, count);
        exit(1);
    }

    if (dim < 2) {
        fprintf(stderr, "error: invalid dimension: %s: %d", filename, dim);
        exit(1);
    }

    Point::dim = dim;

    Point *pts = new Point[count]; 
    char buffer[256];
    for (int i = 0; i < count; ++i) { 
        double value;

        if (fgets(buffer, 255, f) == 0) {
            fprintf(stderr, "error: short file: %s: %d", filename, dim);
            exit(1); 
        }

        char *start = buffer;
        char *end = start;
        for (int d = 0; d < dim; ++d) {
            while (*end != ',' && *end != ' ' && *end != 0) ++end;
            *end = 0;
            value = atof(start); 
            start = end + 1; 
            pts[i][d] = value;
        }
    }

    fclose(f);

    return pts; 
}

int main(int argc, char **argv)
{ 
    if (argc < 2) {
        std::cout << "usage: knn <pts> [queries] [nn] [epsilon]" << std::endl;
        exit(1);
    }

    int pt_count, dim;
    Point *pts = read_points(argv[1], pt_count, dim); 
   
    KdTree<Point, double> kt(dim, pts, pt_count);

    if (argc < 3) {
        return 1;
    }

    int q_count, q_dim;
    Point *queries = read_points(argv[2], q_count, q_dim); 
    if (dim != q_dim) {
        std::cerr << "error: query dim: " << q_dim;
        std::cerr << " does not match point dim: " << dim << std::endl;
        exit(1);
    }

    //how many nearest neighbours to retrieve
    int nn = 5;
    if (argc >= 4) nn = atoi(argv[3]);

    //read query epsilon
    double epsilon = 0.0;
    if (argc == 5) epsilon = atof(argv[4]);

    //run queries
    for (int i = 0; i < q_count; ++i) { 

        std::list<std::pair<Point *, double> > qr = kt.knn(nn, queries[i], epsilon);  

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

    std::cout << "done." << std::endl;

    delete[] pts;
    delete[] queries; 

    return 0;
}
