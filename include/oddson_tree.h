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

#ifndef ODDSON_TREE_H_
#define ODDSON_TREE_H_

#include "kdtree.h"

#include <algorithm>
#include <cstdio>
#include <vector>

/*
Odds-on Tree implementation based upon descriptions in: 
    Bose, P. et al (2010) Odds-on Trees retrieved from: http://arxiv.org/abs/1002.1092 
and 
    P. Afshani, J. Barbay, and T. M. Chan (2009) Instance-optimal geometric algorithms
    in Proceedings of FOCS 2009.  
*/

template<class Point> class OddsonTree {

public:

    struct CachedPoint : Point { 
        size_t dim;
        Point *nn;
        Point a, b;

        CachedPoint(size_t dim) : nn(0), dim(dim)
        { 
        }

        bool contains(const Point &pt)
        {
            //printf("a: (%d %d) b: (%d %d) pt: (%d %d)\n", a[0], a[1], b[0], b[1], pt[0], pt[1]);

            bool result = true; 
            int dim = 2;

            for (size_t d = 0; d < dim; ++d) { 
                if (!((a[d] < pt[d] && pt[d] < b[d]) || 
                      (b[d] < pt[d] && pt[d] < a[d]))) { 
                        result = false; 
                        break; 
                } 
            }

            return result;
        }
    };

    OddsonTree(size_t dim, Point *pts, size_t n, Point (*distfn)(), size_t m) : dim(dim), comp(dim)
    {

        backup = new KdTree<Point>(dim, pts, n);

        //generate sample points 
        Point *sample = new Point[m]; 
        for (size_t i = 0; i < m; ++i) { 
            sample[i] = distfn(); 
        } 

        std::sort(&sample[0], &sample[m], comp); 

        int n3 = 0; 
        int n4 = 0; 
        int n5ormore = 0;

        int total_useful = 0; 

        cache.reserve(m/10); 

        Point *last_nn = 0; 
        int run = 0;

        for (size_t i = 0; i < m; ++i) { 
            std::vector<std::pair<Point *, int> > result = backup->knn(1, sample[i], 0.0); 
            Point *nn = result.back().first; 

            if (nn) { 
                if (nn == last_nn) { 
                    ++run; 
                } else { 
                    if (run >= 4) {         
                        total_useful += run; 

                        CachedPoint pt(dim); 
                        pt.nn = last_nn; 
                        pt.a = sample[i - run - 1]; 
                        pt.b = sample[i - 1]; 

                        for (size_t d = 0; d < dim; ++d) { 
                            pt[d] = sample[i][d]; 
                        }

                        cache.push_back(pt); 
                    } 

                    run = 0; 
                } 
            } 

            last_nn = nn; 
        }

        fprintf(stderr, "total terminal: %d of %d percent: %0.2f cache size: %d\n", total_useful, m, (double)total_useful/(double)m, cache.size());

        delete[] sample;
    }

    virtual ~OddsonTree()
    {
        delete backup;
    }

    std::vector<std::pair<Point *, int> > knn(size_t k, const Point &pt, double eps) 
    {

        std::vector<std::pair<Point *, int> > result; 

        //check cache 
        bool in_cache = false; 

        typename std::vector<CachedPoint>::iterator cpt = std::upper_bound(cache.begin(), cache.end(), pt, comp);

        if (cpt != cache.end() && cpt->contains(pt)) { 
            result.push_back(std::make_pair<Point *, int>(cpt->nn, -1.0)); 
        } else { 
            result = backup->knn(k, pt, eps); 
        }

        return result; 
    }

private:

    struct ZOrder {

        size_t dim;

        ZOrder(size_t dim) : dim(dim) 
        { 
        }

        bool operator()(const Point &a, const Point &b) 
        { 
            int j = 0; 
            int x = 0; 

            for (int k = 0; k < dim; ++k) { 
                int y = a[k] ^ b[k];

                if (x < y && x < (x ^ y)) { 
                    j = k; 
                    x = y; 
                } 
            }
 
            return a[j] < b[j]; 
        } 
    };

    size_t dim;
    KdTree<Point> *backup; 
    std::vector<CachedPoint> cache;
    ZOrder comp;
};

#endif
