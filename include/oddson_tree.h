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
#include "zorder.h"

#include <algorithm>
#include <cstdio>
#include <list>

/*
Odds-on Tree implementation based upon descriptions in: 
    Bose, P. et al (2010) Odds-on Trees retrieved from: http://arxiv.org/abs/1002.1092 
and 
    P. Afshani, J. Barbay, and T. M. Chan (2009) Instance-optimal geometric algorithms
    in Proceedings of FOCS 2009.  
*/

template<class Point> class OddsonTree {

public:

    struct CacheNode : Point { 
        size_t dim;
        Point *nn;
        Point a, b;

        CacheNode *left, *right;

        CacheNode(size_t dim) : nn(0), dim(dim), left(0), right(0)
        { 
        }

        Point *locate(const Point &pt)
        {
            CacheNode *node = this; 
            Point *result = 0; 

            bool found = true; 

            for (size_t d = 0; d < dim; ++d) { 
                if ((a[d] > pt[d] || pt[d] > b[d])) { 
                    found = false; 
                    break; 
                } 
            }

            if (found) { 
                //if a child, return nn - otherwise search children
                if (nn) { 
                    result = nn; 
                } else { 
                    if (left) result = left->locate(pt); 
                    if (!result && right) result = right->locate(pt); 
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

        std::list<CacheNode *> cache;

        Point *last_nn = 0; 
        int run = 0;

        for (size_t i = 0; i < m; ++i) { 
            std::vector<std::pair<Point *, double> > result = backup->knn(1, sample[i], 0.0); 
            Point *nn = result.back().first; 

            if (nn) { 
                if (nn == last_nn) { 
                    ++run; 
                } else { 
                    if (run >= 4) {         
                        total_useful += run; 

                        CacheNode *node = new CacheNode(dim); 
                        node->nn = last_nn; 
                        node->a = sample[i - run - 1]; 
                        node->b = sample[i - 1]; 

                        for (size_t d = 0; d < dim; ++d) { 
                            node[d] = sample[i][d]; 
                            if (node->a[d] > node->b[d]) {
                                std::swap(node->a[d], node->b[d]);
                            } 
                        }

                        cache.push_back(node); 
                    } 

                    run = 0; 
                } 
            } 

            last_nn = nn; 
        }

        while (cache.size() > 1) { 

            //keep track of initial size
            size_t size = cache.size(); 

            //we combine neighbouring nodes two at a time
            for (size_t i = 0; i < size; i += 2) { 
                CacheNode *node = new CacheNode(dim);

                node->left = cache.front(); 
                cache.pop_front(); 

                node->right = cache.front(); 
                cache.pop_front(); 

                for (size_t d = 0; d < dim; ++d) { 
                    node->a[d] = std::min(node->left->a[d], node->right->a[d]); 
                    node->b[d] = std::max(node->left->b[d], node->right->b[d]); 
                } 

                cache.push_back(node); 
            }

            // if odd size, move remaining node to back to preserve z-order
            if (size % 2  == 1) {
                CacheNode *n = cache.front();
                cache.pop_front();
                cache.push_back(n);
            } 
        }

        root = cache.front();

        fprintf(stderr, "total terminal: %d of %d percent: %0.2f cache size: %d\n", total_useful, m, (double)total_useful/(double)m, cache.size());

        delete[] sample;
    }

    virtual ~OddsonTree()
    {
        delete backup;
    }

    std::vector<std::pair<Point *, double> > knn(size_t k, const Point &pt, double eps) 
    {

        std::vector<std::pair<Point *, double> > result; 

        //check cache
        Point *nn = root->locate(pt);
 
        if (nn) {
            result.push_back(std::make_pair<Point *, double>(nn, -1.0)); 
        } else { 
            result = backup->knn(k, pt, eps); 
        }

        return result; 
    }

private:

    size_t dim;
    KdTree<Point> *backup; 
    CacheNode *root;
    ZOrder<Point, double> comp;
};

#endif
