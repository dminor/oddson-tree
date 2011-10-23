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

#include <cstring>
#include <iostream>

#include "kdtree.h" 
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

    const bool operator!=(const Point &other)
    { 
        return !(v[0] == other.v[0] && v[1] == other.v[1]);
    } 
};

Point distfn() 
{

    Point pt; 

    //Box-Muller 
    double u = (double)rand()/(double)RAND_MAX; 
    double v = (double)rand()/(double)RAND_MAX; 

    pt[0] = 250.0 + 25.0*sqrt(-2.0*log(u))*cos(2.0*3.14159*v); 
    pt[1] = 250.0 + 25.0*sqrt(-2.0*log(u))*sin(2.0*3.14159*v); 

    return pt; 
}

#define N 10000
#define M 2000
#define Q 1000000

int main(int argc, char **argv) 
{ 

    //generate data points 
    Point *ps = new Point[N]; 
    for (size_t i = 0; i < N; ++i) { 
        ps[i][0] = 500*(double)rand()/(double)RAND_MAX; 
        ps[i][1] = 500*(double)rand()/(double)RAND_MAX; 
    }

    //generate query points 
    Point *qs = new Point[M]; 
    for (size_t i = 0; i < M; ++i) { 
        qs[i] = distfn();
    }

    OddsonTree<Point> oot(2, ps, N, qs, M); 
    KdTree<Point, double> kdt(2, ps, N); 

    int errors = 0;
    for (size_t i = 0; i < Q; ++i) { 
        Point pt = distfn();

        std::list<std::pair<Point *, double> > qr = oot.knn(1, pt, 0.0); 
        Point nn = *qr.back().first;

        std::list<std::pair<Point *, double> > qr2 = kdt.knn(1, pt, 0.0); 
        Point nn2 = *qr2.back().first;

        if (nn != nn2) {
            ++errors; 
        }
    }

    std::cerr << "# of errors: " << errors << " of " << Q << " : "
         << (float)errors/(float)Q*100.0f << " percent.\n";
}

