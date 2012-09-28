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

#define KDTREE_COLLECT_KNN_STATS
#include "kdtree.h"

#include <cstdio>
#include <cstring>
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
        bool terminal;
        typename KdTree<Point, double>::Node *nn;

        CachedPoint() : nn(0)
        { 
        }

    };

    struct OddsonTreeTerminal : public KdTree<CachedPoint, double>::EndBuildFn {

        KdTree<Point, double> *backup;
        int dim;

        virtual bool operator()(typename KdTree<CachedPoint, double>::Node *node, double *range)
        {
            CachedPoint *pt = node->pt;

            //run interference query (need to make sure all "corners" have same nearest-neighbour)
            pt->nn = 0;
            for (size_t i = 0; i < 2*dim; ++i) {
                Point qp;
                for (size_t d = 0; d < dim; ++d) {
                    if (i & (1 << d)) qp[d] = range[d*2];
                    else qp[d] = range[d*2+1];
                }

                typename KdTree<Point, double>::Node *qr = backup->nn(qp);

                if (pt->nn == 0) {
                    pt->nn = qr;
                } else {
                    if (pt->nn != qr) {
                        return false;
                    }
                }
            } 

            pt->terminal = true;
            return true;
        } 
    };

    OddsonTree(int dim, Point *ps, int n, Point *qs, int m)
        : dim(dim)
    {

        backup = new KdTree<Point, double>(dim, ps, n);

        //track range covered by sample
        range = new double[2*dim]; 
        for (size_t d = 0; d < dim; ++d) {
            range[d*2] = std::numeric_limits<double>::max();
            range[d*2+1] = -std::numeric_limits<double>::max();
        }

        //generate sample points
        CachedPoint *sample = new CachedPoint[m];
        for (size_t i = 0; i < m; ++i) {
            Point &pt = qs[i];

            //copy into cached point
            for (size_t d = 0; d < dim; ++d) {
                sample[i][d] = pt[d];
                if (pt[d] < range[d*2]) range[d*2] = pt[d];
                if (pt[d] > range[d*2+1]) range[d*2+1] = pt[d];
            }
        }

        //build kdtree for cache
        OddsonTreeTerminal fn;
        fn.backup = backup;
        fn.dim = dim;
        cache = new KdTree<CachedPoint, double>(dim, sample, m, range, fn); 

        hits = 0;
        queries = 0;
        backup->knn_nodes_visited = 0;
    }

    virtual ~OddsonTree()
    {
        fprintf(stderr, "hits: %d queries: %d percent: %0.2f\n", hits, queries, (double)hits / (double)queries);

        fprintf(stderr, "average backup nodes visited / query: %0.2f\n", (double)backup->knn_nodes_visited / (double)queries);

        delete[] range;
        delete backup;
        delete cache;
    }


    std::list<std::pair<Point *, double> > nn(const Point &pt, double eps) 
    {
        std::list<std::pair<Point *, double> > result;

        CachedPoint *cache_result = locate(pt);

        //check if terminal
        if (cache_result && cache_result->nn) {
            double d = 0; 
            for (int i = 0; i < dim; ++i) {
                d += ((*(cache_result->nn->pt))[i]-pt[i]) * ((*(cache_result->nn->pt))[i]-pt[i]); 
            } 

            result.push_back(std::make_pair<Point *, double>(cache_result->nn->pt, d));

            ++hits;
        } else {
            result = backup->knn(1, pt, eps); 
        }

        ++queries;
        return result; 
    } 

    std::list<std::pair<Point *, double> > knn(size_t k, const Point &pt, double eps) 
    {
        std::list<std::pair<Point *, double> > result;

        PriorityQueue<typename KdTree<Point, double>::Node *> pq(k);
//        locate(pq, pt);
        result = backup->knn(k, pq, pt, eps); 

        ++queries;
        return result; 
    }


    KdTree<CachedPoint, double> *cache;

private:

    CachedPoint *locate(const Point &pt) 
    { 
        typename KdTree<CachedPoint, double>::Node *node = cache->root; 
        CachedPoint *qr = 0; 

        //early out, not covered by cache
        for (int i = 0; i < dim; ++i) {
            if (range[i*2] > pt[i] || range[i*2 + 1] < pt[i]) {
                return qr;
            }
        }

        size_t depth = 0; 

        while (node && node->pt && !node->pt->terminal) { 
            if (pt[depth % dim] < node->median) { 
                node = node->left(); 
            } else { 
                node = node->right(); 
            }

            if (node && node->pt && node->pt->terminal) qr = node->pt; 
            ++depth; 
        } 

        return qr; 
    }

    void locate(PriorityQueue<typename KdTree<Point, double>::Node *> &pq, const Point &pt)
    { 
        typename KdTree<CachedPoint, double>::Node *node = cache->root; 

        //early out, not covered by cache
        for (int i = 0; i < dim; ++i) {
            if (range[i*2] > pt[i] || range[i*2 + 1] < pt[i]) {
                return;
            }
        }

        size_t depth = 0; 

        while (node && node->pt && !node->pt->terminal) { 

            //calculate distance from query point to this point 
            if (node->pt->nn) {
                double d = 0; 
                for (int i = 0; i < dim; ++i) {
                    d += ((*(node->pt->nn->pt))[i]-pt[i]) * ((*(node->pt->nn->pt))[i]-pt[i]); 
                }

                if ((!pq.full() || d < pq.peek().priority)) {
                    pq.push(d, node->pt->nn);
                }
            }

            if (pt[depth % dim] < node->median) { 
                node = node->left(); 
            } else { 
                node = node->right(); 
            }

            ++depth; 
        } 
    }
 
    size_t dim;
    KdTree<Point, double> *backup; 
    double *range;

    int hits;
    int queries; 
};

#endif

