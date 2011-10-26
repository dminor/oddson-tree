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
#include <list>
#include <vector>

/*
Odds-on Tree implementation based upon descriptions in: 
Bose, P. et al (2010) Odds-on Trees retrieved from: http://arxiv.org/abs/1002.1092 
and 
P. Afshani, J. Barbay, and T. M. Chan (2009) Instance-optimal geometric algorithms
in Proceedings of FOCS 2009.  
*/

#include "zorder.h"

template<class Point> class OddsonTree {

public:

    struct CacheNode { 
        size_t dim;
        Point *nn;
        Point a, b;

        CacheNode *left, *right;

        CacheNode(size_t dim) : nn(0), dim(dim), left(0), right(0)
        { 
        }

        Point *locate(const Point &pt)
        {
            //printf("a: (%d %d) b: (%d %d) pt: (%d %d)\n", a[0], a[1], b[0], b[1], pt[0], pt[1]);

            CacheNode *node = this;
            Point *result = 0;

            bool found = true;
            for (size_t d = 0; d < dim; ++d) {
                if ((a[d] > pt[d] || pt[d] > b[d])) {
                    found = false;
                    break;
                }
            }

            //if a child
            if (found) {
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

    OddsonTree(int dim, Point *ps, int n, Point *qs, int m)
        : dim(dim), root(0) 
    {

        ZOrder<Point, double> comp(dim);
        backup = new KdTree<Point, double>(dim, ps, n);

        std::sort(&qs[0], &qs[m], comp);

        int total_useful = 0;

        std::list<CacheNode *> cache;

        Point *last_nn = 0;
        int run = 0;
        for (size_t i = 0; i < m; ++i) {
            std::list<std::pair<Point *, double> > result = backup->knn(1, qs[i], 0.0); 
            Point *nn = result.back().first; 

            if (nn && nn == last_nn) ++run;
            else run = 0;
 
            if (run >= 2) {

                total_useful += 1;

                CacheNode *pt = new CacheNode(dim);
                pt->nn = last_nn;
                pt->a = qs[i - run + 1];
                pt->b = qs[i];


                Point pt1;
                pt1[0] = pt->b[0];
                pt1[1] = pt->a[1];
                std::list<std::pair<Point *, double> > result = backup->knn(1, pt1, 0.0); 
                Point *pt1_nn = result.back().first; 

                if (pt1_nn == nn) {
                    Point pt2;
                    pt2[0] = pt->a[0];
                    pt2[1] = pt->b[1];
                    std::list<std::pair<Point *, double> > result = backup->knn(1, pt2, 0.0); 
                    Point *pt2_nn = result.back().first; 

                    if (pt2_nn == nn) {
//                        fprintf(stderr, "%f %f, %f %f, %f %f, %f %f\n", pt->a[0], pt->a[1], pt->b[0], pt->b[1], pt1[0], pt1[1], pt2[0], pt2[1]);

                        for (size_t d = 0; d < dim; ++d) { 
                            if (pt->a[d] > pt->b[d]) std::swap(pt->a[d], pt->b[d]);
                        }
                        cache.push_back(pt); 
                        run = 0;
                    } else {
                        delete pt;
                    }
                } 
            }

            last_nn = nn;
        }

        int cache_size = cache.size();
        while (cache.size() > 1) {
            
            size_t size = cache.size();

            for (size_t i = 0; i < size; i+= 2) {
                CacheNode *node = new CacheNode(dim);
                node->left = cache.front();
                cache.pop_front();
                node->right = cache.front();
                cache.pop_front();

                for (size_t d = 0; d < dim; ++d) {
                    node->a[d] = std::min(node->left->a[d], node->right->a[d]);
                    node->b[d] = std::max(node->left->b[d], node->right->b[d]);
                }
/*
                if (node->left->nn == node->right->nn) {
                    Point pt1;
                    pt1[0] = node->b[0];
                    pt1[1] = node->a[1];
                    std::list<std::pair<Point *, double> > result = backup->knn(1, pt1, 0.0); 
                    Point *pt1_nn = result.back().first; 

                    if (pt1_nn == node->left->nn) {
                        Point pt2;
                        pt2[0] = node->a[0];
                        pt2[1] = node->b[1];
                        std::list<std::pair<Point *, double> > result = backup->knn(1, pt2, 0.0); 
                        Point *pt2_nn = result.back().first; 

                        if (pt2_nn == node->left->nn) { 
                            node->nn = node->left->nn;
                            delete node->left;
                            delete node->right;
                            node->left = 0;
                            node->right = 0;
                        }
                    } 
                }
*/
                cache.push_back(node);
            }
        }

        if (!cache.empty()) root = cache.front();

        fprintf(stderr, "total terminal: %d of %d percent: %0.2f cache size: %d\n", total_useful, m, (double)total_useful/(double)m, cache_size); 

        hits = 0;
        queries = 0;
    }

    virtual ~OddsonTree()
    {
        fprintf(stderr, "hits: %d queries: %d percent: %0.2f\n", hits, queries, (double)hits / (double)queries);

        delete backup;
    }

    std::list<std::pair<Point *, double> > knn(size_t k, const Point &pt, int eps) 
    {
        std::list<std::pair<Point *, double> > result;

        //check cache
        bool in_cache = false;

        Point *nn = root->locate(pt);

        if (nn) {
            double dist = (pt[0] - (*nn)[0])*(pt[0] - (*nn)[0]) + 
            (pt[1] - (*nn)[1])*(pt[1] - (*nn)[1]); 

            result.push_back(std::make_pair<Point *, double>(nn, dist));
            ++hits;
        } else {
            result = backup->knn(k, pt, eps); 
        }

        ++queries;

        return result; 
    }

    CacheNode *root;

private:

    size_t dim;
    KdTree<Point, double> *backup; 

    int hits;
    int queries;
};

#endif

