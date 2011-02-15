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
        Point *nn;

        CachedPoint() : terminal(false), nn(0)
        { 
        }
    };

    OddsonTree(size_t dim, Point *pts, size_t n, Point (*distfn)(), size_t m) : dim(dim)
    {

        backup = new KdTree<Point>(dim, pts, n);

        //track range covered by sample
        range = new double[2*dim]; 
        for (size_t d = 0; d < dim; ++d) {
            range[d*2] = std::numeric_limits<double>::max();
            range[d*2+1] = -std::numeric_limits<double>::max();
        }

        //generate sample points
        CachedPoint *sample = new CachedPoint[m];
        for (size_t i = 0; i < m; ++i) {
            Point pt = distfn();

            //find nearest neighbour
            std::vector<std::pair<Point *, double> > result = backup->knn(1, pt, 0.0); 
            //sample[i].nn = result.back().first; 

            //copy into cached point
            for (size_t d = 0; d < dim; ++d) {
                sample[i][d] = pt[d];
                if (pt[d] < range[d*2]) range[d*2] = pt[d];
                if (pt[d] > range[d*2+1]) range[d*2+1] = pt[d];
            }

        }

        //build kdtree for cache
        cache = new KdTree<CachedPoint>(dim, sample, m); 

        //and trim nodes
        trim(cache->root, range, 1);

        //FIXME: delete sample?
    }

    virtual ~OddsonTree()
    {
        delete[] range;
        delete backup;
        delete cache;
    }

    std::vector<std::pair<Point *, double> > knn(size_t k, const Point &pt, double eps) 
    {
        std::vector<std::pair<Point *, double> > result;

        //check cache
        CachedPoint cpt;

        bool in_cache = true;
        for (size_t i = 0; i < dim; ++i) {
            if (pt[i] < range[2*i] || pt[i] > range[2*i+1]) {
                in_cache = false;
                break;
            }

            cpt[i] = pt[i];
        }

        if (in_cache) {
            CachedPoint *cache_result = cache->locate(cpt);

            //check if terminal
            if (cache_result->terminal) {
                for (size_t i = 0; i < k; ++i) {
                    result.push_back(std::make_pair<Point *, double>(cache_result->nn, -1.0));
                }
            } else {
                in_cache = false;
            }
        }

        if (!in_cache) {
            result = backup->knn(k, pt, eps); 
        } 

        return result; 
    }

private:

    void trim(typename KdTree<CachedPoint>::Node *node, double *range, size_t depth)
    {
        //bottom of recursion, non-terminal leaf
        if (node->left == 0 && node->right == 0) {
            node->pt->terminal = false;
        } else { 

            bool terminal = true;

            //set up interference query
            double *range2 = new double[2*dim];
            for (size_t d = 0; d < dim; ++d) {

                double d1 = (*node->pt)[d] - range[d*2];
                double d2 = range[d*2+1] - (*node->pt)[d];
                double dist = d1 > d2 ? d1 : d2;

                range2[d*2] = (*node->pt)[d] - dist;
                range2[d*2+1] = (*node->pt)[d] + dist; 
            }

            //run interference query (need to make sure all "corners" have same nearest-neighbour)
            Point *nn = 0;//node->pt->nn;
            for (size_t i = 0; i < 2*dim; ++i) {
                Point qp;
                for (size_t d = 0; d < dim; ++d) {
                    if (i & 1 << d) qp[d] = range2[d*2];
                    else qp[d] = range2[d*2+1];
                }

                std::vector<std::pair<Point *, double> > qr = backup->knn(1, qp, 0.0);

                if (nn == 0) {
                    nn = qr.back().first;
                } else {
                    if (nn != qr.back().first) {
                        terminal = false;
                        break;
                    }
                }
            }

            delete[] range2;

            if (terminal) {
                //std::cerr << "terminal!\n";
                if (node->left) {
                    delete_subtree(node->left);
                    node->left = 0;
                }

                if (node->right) {
                    delete_subtree(node->right);
                    node->right = 0;
                }

                node->pt->terminal = true;
                node->pt->nn = nn;

            } else {

                node->pt->terminal = false;

                //update range and query subtrees
                size_t range_coord = (depth%dim)*2;

                double t;
                if (node->left) {
                    t = range[range_coord+1]; 
                    range[range_coord+1] = node->median;
                    trim(node->left, range, depth+1);
                    range[range_coord+1] = t; 
                }

                if (node->right) {
                    t = range[range_coord]; 
                    range[range_coord] = node->median; 
                    trim(node->right, range, depth+1);
                    range[range_coord] = t; 
                }
            }
        }
    }

    void delete_subtree(typename KdTree<CachedPoint>::Node *node)
    {
        std::vector<typename KdTree<CachedPoint>::Node *> nodes;
        nodes.push_back(node);
        while (!nodes.empty()) {
            typename KdTree<CachedPoint>::Node *n = nodes.back();
            nodes.pop_back();

            if (n->left) nodes.push_back(n->left);
            if (n->right) nodes.push_back(n->right);
            delete n;
        } 
    }

    size_t dim;
    KdTree<Point> *backup; 
    KdTree<CachedPoint> *cache;
    double *range;
};

#endif
