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

/*
Odds-on Tree implementation based upon descriptions in: 
    Bose, P. et al (2010) Odds-on Trees retrieved from: http://arxiv.org/abs/1002.1092 
*/ 

int total_nodes = 0;
int terminal_nodes = 0;
int build_nn_queries = 0;

#if defined ODDSON_TREE_KDTREE_IMPLEMENTATION

#define KDTREE_COLLECT_KNN_STATS
#include "kdtree.h"

#include <cstdio>
#include <cstring>
#include <vector>

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
        size_t max_depth;

        virtual bool operator()(typename KdTree<CachedPoint, double>::Node *node, double *range, size_t depth)
        {
            CachedPoint *pt = node->pt;
            ++total_nodes;

            if (depth > max_depth) {
                return true;
            }

            //run interference query (need to make sure all "corners" have same nearest-neighbour)
            pt->nn = 0;
            for (size_t i = 0; i < 1<<dim; ++i) {
                Point qp;
                for (size_t d = 0; d < dim; ++d) {
                    if (i & (1 << d)) qp[d] = range[d*2];
                    else qp[d] = range[d*2+1];
                }

                typename KdTree<Point, double>::Node *qr = backup->nn(qp);
                ++build_nn_queries;

                if (pt->nn == 0) {
                    pt->nn = qr;
                } else {
                    if (pt->nn != qr) {
                        return false;
                    }
                }
            } 

            pt->terminal = true;
            ++terminal_nodes;
            return true;
        } 
    };

    OddsonTree(int dim, Point *ps, int n, Point *qs, int m, size_t max_depth)
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
        fn.max_depth = max_depth;
        cache = new KdTree<CachedPoint, double>(dim, sample, m, range, fn); 

        fprintf(stderr, "info: total nodes: %d\n", total_nodes);
        fprintf(stderr, "info: terminal nodes: %d\n", terminal_nodes);
        fprintf(stderr, "info: build nn queries: %d\n", build_nn_queries);

        hits = 0;
        queries = 0;
        backup->knn_nodes_visited = 0;
        total_nodes = 0;
        terminal_nodes = 0;
        build_nn_queries = 0;
    }

    virtual ~OddsonTree()
    {
        fprintf(stderr, "info: hits: %d queries: %d percent: %0.2f\n", hits, queries, (double)hits / (double)queries);

        fprintf(stderr, "info: backup nodes visited: %d\n", backup->knn_nodes_visited);

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
        locate(pq, pt);
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

                if (d < pq.peek().priority) {
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

#elif defined ODDSON_TREE_QUADTREE_IMPLEMENTATION

#include "compressed_quadtree.h"
#include "kdtree.h"

#include <algorithm>
#include <cstdio>
#include <list>
#include <vector>

template<class Point> class OddsonTree {

public:

     struct CachedPoint : Point { 
        bool terminal;
        typename KdTree<Point, double>::Node *nn;

        CachedPoint() : nn(0)
        { 
        }

        CachedPoint(const Point &pt)
            : Point(pt)
        {

        } 
    };

    struct OddsonTreeTerminal : public CompressedQuadtree<CachedPoint>::EndBuildFn {

        KdTree<Point, double> *backup;
        int dim;
        size_t max_depth;

        virtual bool operator()(typename CompressedQuadtree<CachedPoint>::Node *node, size_t depth)
        {
            if (depth > max_depth) {
                return true;
            }

            //run interference query (need to make sure all "corners" have same nearest-neighbour)
            typename KdTree<Point, double>::Node *nn = 0;
            for (size_t i = 0; i < 1<<dim; ++i) {
                Point qp;
                for (size_t d = 0; d < dim; ++d) {
                    if (i & (1 << d)) qp[d] = node->mid[d] - node->radius;
                    else qp[d] = node->mid[d] + node->radius;
                }

                typename KdTree<Point, double>::Node *qr = backup->nn(qp);

                if (nn == 0) {
                    nn = qr;
                } else {
                    if (nn != qr) {
                        return false;
                    }
                }
            } 

            node->pt = new CachedPoint();
            node->pt->nn = nn;
            node->pt->terminal = true;

            return true;
        } 
    };

    OddsonTree(int dim, Point *ps, int n, Point *qs, int m, size_t max_depth)
        : dim(dim)
    {

        backup = new KdTree<Point, double>(dim, ps, n);

        double *range = new double[2*dim]; 

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

        OddsonTreeTerminal fn;
        fn.backup = backup;
        fn.dim = dim;
        fn.max_depth = max_depth;
        cache = new CompressedQuadtree<CachedPoint>(dim, sample, m, range, fn);
 
        delete[] range;

        hits = 0;
        queries = 0;
    }

    virtual ~OddsonTree()
    {
        fprintf(stderr, "info: hits: %d queries: %d percent: %0.2f\n", hits, queries, (double)hits / (double)queries);

        //FIXME: double delete in compressed quadtree
        //delete cache;
        delete backup; 
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

    std::list<std::pair<Point *, double> > knn(size_t k, const Point &pt, int eps) 
    {
        std::list<std::pair<Point *, double> > result;

        //check cache
        bool in_cache = false;

        CachedPoint *cp = locate(pt);

        PriorityQueue<typename KdTree<Point, double>::Node *> pq(k);
        locate(pq, pt);
        result = backup->knn(k, pq, pt, eps); 
        ++queries;
        return result; 
    }

    CompressedQuadtree<CachedPoint> *cache; 

private:

    size_t dim;
    KdTree<Point, double> *backup; 

    int hits;
    int queries;

    CachedPoint *locate(const Point &pt) 
    { 
        typename CompressedQuadtree<CachedPoint>::Node *node = 0;
        CachedPoint *qr = 0; 

        //search for node containing the query point 
        if (cache->root->in_node(pt, cache->dim)) { 
            node = cache->root; 

            while (node) {

                if (node->nodes) { 
                    size_t n = 0; 
                    for (size_t d = 0; d < dim; ++d) { 
                        if (pt[d] > node->mid[d]) n += 1 << d; 
                    } 

                    if (node->nodes[n] && node->nodes[n]->in_node(pt, cache->dim)) {
                        node = node->nodes[n]; 
                        if (node && node->pt && node->pt->terminal) {
                            qr = node->pt; 
                            break;
                        }
                    } else {
                        break;
                    } 
                } else {
                    break;
                } 
            } 
        } 

        return qr; 
    } 

    CachedPoint *locate(PriorityQueue<typename KdTree<Point, double>::Node *> &pq, const Point &pt)
    {
        typename CompressedQuadtree<CachedPoint>::Node *node = 0;
        CachedPoint *qr = 0; 

        //search for node containing the query point 
        if (cache->root->in_node(pt, cache->dim)) { 
            node = cache->root; 

            while (node) {

                if (node->nodes) { 
                    size_t n = 0; 
                    for (size_t d = 0; d < dim; ++d) { 
                        if (pt[d] > node->mid[d]) n += 1 << d; 
                    } 

                    if (node->nodes[n] && node->nodes[n]->in_node(pt, cache->dim)) {
                        node = node->nodes[n]; 

                        if (node->pt && node->pt->nn) {
                            double d = 0; 
                            for (int i = 0; i < dim; ++i) {
                                d += ((*(node->pt->nn->pt))[i]-pt[i]) * ((*(node->pt->nn->pt))[i]-pt[i]); 
                            } 

                            pq.push(d, node->pt->nn);
                        }

                        if (node->pt && node->pt->terminal) {
                            qr = node->pt; 
                            break;
                        }
                    } else {
                        break;
                    } 
                } else {
                    break;
                } 
            } 
        } 

        return qr; 
    }

};

#else

#error oddson tree implementation not defined

#endif 

#endif

