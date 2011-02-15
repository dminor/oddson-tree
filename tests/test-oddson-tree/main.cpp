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

    double &operator[](const int &index) 
    { 
        return v[index]; 
    } 

    const double &operator[](const int &index) const 
    { 
        return v[index]; 
    } 
};

Point distfn() 
{

    Point pt; 

    //Box-Muller 
    double u = (double)rand()/(double)RAND_MAX; 
    double v = (double)rand()/(double)RAND_MAX; 

    pt[0] = -1.0 + 0.25*sqrt(-2.0*log(u))*cos(2.0*3.14159*v); 
    pt[1] = -1.0 + 0.25*sqrt(-2.0*log(u))*sin(2.0*3.14159*v); 

    return pt; 
}

#define N 1000
#define M 20000
#define Q 50000 

int main(int argc, char **argv) 
{ 

    if (argc < 2) { 
        std::cerr << "usage: oddson-tree oot | kdt [nop]\n"; 
        return 0; 
    }

    //generate data points 
    Point pts[N]; 
    for (size_t i = 0; i < N; ++i) { 
        pts[i][0] = -2.0 + 4.0*(double)rand()/(double)RAND_MAX; 
        pts[i][1] = -2.0 + 4.0*(double)rand()/(double)RAND_MAX; 
    }

    //set up queries 
    Point knn[Q]; 
    for (size_t i = 0; i < Q; ++i) { 
        knn[i] = distfn(); 
    }

    //run queries on oot 
    if (!strncmp(argv[1], "oot", 3)) {

        OddsonTree<Point> oot(2, pts, N, distfn, M); 
        std::cout << "oot\n";

        if (argc < 3) { 
            for (size_t i = 0; i < Q; ++i) { 
                std::vector<std::pair<Point *, double> > qr = oot.knn(1, knn[i], 0.0);

                Point nn = *qr.back().first;
     
                std::cout << "closest point to: (" << knn[i][0] << ", " << knn[i][1] << ") is: "; 
                std::cout << "(" << nn[0] << ", " << nn[1] << "): " << qr.back().second << "\n";

            } 
        }
    } else {

        if (argc < 3) { 
            //run queries on kdtree 
            KdTree<Point> kdt(2, pts, N); 
            std::cout << "kdt\n";

            for (size_t i = 0; i < Q; ++i) { 
                std::vector<std::pair<Point *, double> > qr = kdt.knn(1, knn[i], 0.0);

                Point nn = *qr.back().first;
     
                std::cout << "closest point to: (" << knn[i][0] << ", " << knn[i][1] << ") is: "; 
                std::cout << "(" << nn[0] << ", " << nn[1] << "): " << qr.back().second << "\n"; 
            } 
        }
    } 
}
