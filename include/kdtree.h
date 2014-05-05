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

#ifndef KD_TREE_H_
#define KD_TREE_H_

#include <cmath>
#include <cstdlib>
#include <cstdio>

#include <limits>
#include <list>
#include <vector>

#include <sys/mman.h>

#include "fixed_size_priority_queue.h"
#include "priority_queue.h"

template<class Point, class Number> class KdTree {

public:

    struct Node {
        Point *pt;
        Number median;
        Node *children;
        int axis;

        inline Node *left()
        {
            return (long)children & 0xA0000000 ? this + 1 : 0;
        }

        inline Node *right()
        {
            Node *n = this + ((long)children & ~0xA0000000);
            return this == n ? 0 : n;
        }
    };

    KdTree(size_t dim, Point *pts, size_t n)
        : dim(dim)
        , arena(0)
        , searchpq(std::max(32, (int)log(n)))
    {
        arena = (Node *)mmap(0, n*sizeof(Node), PROT_READ|PROT_WRITE,
            MAP_PRIVATE|MAP_ANON, -1, 0);  
        arena_offset = 0;
        root = build_kdtree(pts, n, 0);
        this->n = n;
    }

    struct EndBuildFn {
        virtual bool operator()(Node *, Number *, size_t)
        {
            return false;
        }
    };

    KdTree(size_t dim, Point *pts, size_t n, Number *range, EndBuildFn &fn)
        : dim(dim)
        , arena(0)
        , searchpq(std::max(32, (int)log(n)))
    {
        arena = (Node *)mmap(0, n*sizeof(Node), PROT_READ|PROT_WRITE,
            MAP_PRIVATE|MAP_ANON, -1, 0);  
        arena_offset = 0;

        root = build_kdtree(pts, n, 0, range, fn);
        total_searches = 0;
        knn_nodes_visited = 0;
        knn_nodes_visited_backtrack = 0;

        this->n = n;
    }


    virtual ~KdTree()
    {
        if (arena) munmap(arena, n*sizeof(Node));

    }

    void dump_stats()
    {
        fprintf(stderr, "info: kdtree total searches: %ld\n", total_searches); 
        fprintf(stderr, "info: kdtree nodes visited: %ld\n", knn_nodes_visited); 
        fprintf(stderr, "info: kdtree backtrack nodes visited: %ld\n", knn_nodes_visited_backtrack);
    }
 
    std::vector<Point *> range_search(Number *range)
    {
        //set up region
        Number *region = new Number[2 * dim]; 
        for (int i = 0; i < dim; ++i) {
            region[i] = -std::numeric_limits<Number>::max();
            region[i + 1] = std::numeric_limits<Number>::max(); 
        }

        //run query
        std::vector<Point *> qr = range_search(root, range, region, 0);

        //clean up and return result;
        delete[] region;

        return qr;

    }

    size_t range_count(Number *range)
    {
        //set up region
        Number *region = new Number[2 * dim]; 
        for (int i = 0; i < dim; ++i) {
            region[i] = -std::numeric_limits<Number>::max();
            region[i + 1] = std::numeric_limits<Number>::max(); 
        }

        //run query
        size_t qr = range_count(root, range, region, 0);

        //clean up and return result;
        delete[] region;

        return qr;

    }

    /** This function searches for the k nearest neighbours to a query point. 
       
        \param k The number of nearest neighbours to find.
        \param pt The point for which to find the nearest neighbour.
        \param eps The epsilon for approximate nearest neighbour searches.
        \return A list containing points and distances of the k nearest neighbours
                to the query point. 
    */ 
    std::list<std::pair<Point *, Number> > knn(size_t k, const Point &pt, Number eps) 
    {
        FixedSizePriorityQueue<Node *> pq(k);

        searchpq.clear(); 
        knn_search(pq, pt, eps);

        std::list<std::pair<Point *, Number> > qr; 
        while(pq.length) {
            typename FixedSizePriorityQueue<Node *>::Entry e = pq.pop();
            qr.push_front(std::make_pair<Point *, Number>(e.data->pt,
                e.priority));
        }

        return qr;
    }

    /** This function searches for the k nearest neighbours to a query point. 
        It takes an initial set of nodes which may be nearest neighbours
        of the query point, which potentially reduces how much of the tree
        must be searched. 
       
        \param pq A priority queue containing potential nearest neighbours to the
                  query point. 
        \param pt The point for which to find the nearest neighbour.
        \param eps The epsilon for approximate nearest neighbour searches.
        \return A list containing points and distances of the k nearest neighbours
                to the query point. 
    */ 
    std::list<std::pair<Point *, Number> > knn(size_t k, PriorityQueue<Node *> &searchnodes, const Point &pt, Number eps) 
    { 
        searchpq = searchnodes;

        FixedSizePriorityQueue<Node *> pq(k);
        knn_search(pq, pt, eps);

        std::list<std::pair<Point *, Number> > qr; 
        while(pq.length) {
            typename FixedSizePriorityQueue<Node *>::Entry e = pq.pop();
            qr.push_front(std::make_pair<Point *, Number>(e.data->pt,
                e.priority));
        }

        return qr;
    }

    /** This function searches for a single exact nearest neighbour and returns
        the Node containing it.  This is useful for building caches on top of
        the kd-tree.
       
        \param pt The point for which to find the nearest neighbour.
        \return The Node containing the nearest neighbour. 
    */
    Node *nn(const Point &pt) 
    {
        FixedSizePriorityQueue<Node *> pq(1); 
        searchpq.clear(); 
        knn_search(pq, pt, 0.0); 
        typename FixedSizePriorityQueue<Node *>::Entry e = pq.pop(); 
        return e.data;
    }

    /** This function searches for the node containing a query point.
        Since we don't track the bounds of the original point set, this will
        return incorrect results if the query point is outside of the bounds
        of the kd-tree.  
       
        \param pt The point for which to locate the node. 
        \return The Node containing the query point. 
    */ 
    Node *locate(const Point &pt) 
    { 
        Node *node = root; 

        size_t depth = 1;

        while (node->children) { 

            if (pt[depth % dim] < node->median) { 
                node = node->left(); 
            } else { 
                node = node->right(); 
            }

            ++depth; 
        } 

        return node; 
    }
    
    Node *root;

    long int total_searches;
    long int knn_nodes_visited;
    long int knn_nodes_visited_backtrack;

private:

    size_t n;
    size_t dim;

    Node *arena;
    size_t arena_offset;

    PriorityQueue<Node *> searchpq;

    Node *build_kdtree(Point *pts, size_t pt_count, size_t depth)
    {
        Node *result = 0;

        if (pt_count == 0) {
            //empty branch
        } else if (pt_count == 1) {
            //leaf node, store point and return
            result = new (arena + arena_offset) Node; 
            ++arena_offset;
            result->pt = pts;
            result->median = 0;
            result->children = 0;
        } else {

            result = new (arena + arena_offset) Node; 
            ++arena_offset;

            //branch coordinate
            result->axis = depth % dim; 

            //find median (has side effect of partitioning input array around median)
            size_t median_index = (pt_count / 2) >> 1 << 1;
            Number median = select_order(median_index, pts, pt_count, result->axis);

            //recursively build tree
            result->children = 0;
            Node *left = build_kdtree(pts, median_index, depth + 1); 
            Node *right = build_kdtree(&pts[median_index + 1],
                pt_count - median_index - 1, depth + 1);

            result->children = (Node *)(right - result);
            if (left) result->children = (Node *)((long)result->children | 0xA0000000);

            //store point and median value
            result->pt = &pts[median_index];
            result->median = median;
        } 

        return result;
    }


    Node *build_kdtree(Point *pts, size_t pt_count, size_t depth,
        Number *range, EndBuildFn &fn)
    {
        Node *result = 0;

        if (pt_count == 0) {
            //empty branch
        } else if (pt_count == 1) {
            //leaf node, store point and return
            result = new (arena + arena_offset) Node; 
            ++arena_offset;
            result->pt = pts;
            result->median = 0;
            result->children = 0;
            fn(result, range, depth);
        } else {

            result = new (arena + arena_offset) Node; 
            ++arena_offset;

            //branch coordinate
            result->axis = depth % dim; 

            //find median (has side effect of partitioning input array around median)
            size_t median_index = (pt_count / 2) >> 1 << 1;
            Number median = select_order(median_index, pts, pt_count, result->axis); 

            //store point and median value
            result->pt = &pts[median_index];
            result->median = median; 
            result->children = 0;

            //if not terminal, recursively build tree
            if (!fn(result, range, depth)) { 
                double t;
                size_t range_coord = (depth%dim)*2;

                t = range[range_coord+1]; 
                range[range_coord+1] = result->median;
                Node *left = build_kdtree(pts, median_index, depth + 1, range, fn); 
                range[range_coord+1] = t; 

                t = range[range_coord]; 
                range[range_coord] = result->median; 
                Node *right = build_kdtree(&pts[median_index + 1],
                    pt_count - median_index - 1, depth + 1, range, fn);
                range[range_coord] = t; 

                result->children = (Node *)(right - result);
                if (left) result->children = (Node *)((long)result->children | 0xA0000000);
            }

        } 

        return result;
    }

    size_t partition(size_t start, size_t end, Point *pts, size_t coord)
    { 
        //choose pivot and place at end
        size_t pivot = start + rand() % (end - start); 
        std::swap(pts[pivot], pts[end]);

        //get pivot value
        Number value = pts[end][coord];

        //move values around pivot
        size_t i = start;
        for (size_t j = start; j < end; ++j) { 
            if (pt_lt(coord, pts[j], pts[end])) {
                std::swap(pts[i], pts[j]);
                ++i;
            }
        }
        
        std::swap(pts[i], pts[end]);

        return i; 
    } 

    Number select_order(size_t i, Point *pts, size_t pt_count, size_t coord)
    {
        size_t start = 0;
        size_t end = pt_count - 1; 

        while (1) {

            if (start == end) return pts[start][coord];
     
            size_t pivot = partition(start, end, pts, coord);

            if (i == pivot) {
                return pts[pivot][coord];
            } else if (i < pivot) {
                end = pivot - 1;
            } else {
                start = pivot + 1;
            } 
        } 
    } 

    int pt_lt(size_t coord, const Point &a, const Point &b) const
    {
        //if points are unequal, do direct comparison
        if (a[coord] != b[coord]) {
            return a[coord] < b[coord];
        } else {
            //otherwise, compare in lexicographic order
            size_t i = (coord + 1) % dim; 
            while (a[i] == b[i] && i != coord) i = (i + 1) % dim; 
            return a[i] <= b[i];
        }
    } 

    int point_in_range(Point *p, Number *range)
    {
        for (int i = 0; i < dim; ++i) {
            if (range[i*2] > (*p)[i] || range[i*2+1] < (*p)[i]) return 0;
        }

        return 1; 
    }

    int range_contains_region(Number *range, Number *region)
    { 
        for (int i = 0; i < dim; ++i) {
            if (range[i*2] > region[i*2] || range[i*2+1] < region[i*2+1]) return 0;
            if (range[i*2] > region[i*2+1] || range[i*2+1] < region[i*2]) return 0;
        }

        return 1; 
    }

    int region_intersects_range(Number *range, Number *region)
    {
        int intersects = 0;
        for (int i = 0; i < dim; ++i) {
            if (range[i*2] < region[i*2] || range[i*2+1] > region[i*2+1]) intersects = 1; 
            if (range[i*2] < region[i*2+1] || range[i*2+1] > region[i*2]) intersects = 1; 
        }

        return intersects; 
    }

    void report_subtree(Node *tree, std::vector<Point *> &qr)
    { 
        qr.push_back(tree->pt);

        //recurse through tree
        if (tree->left()) report_subtree(tree->left(), qr);
        if (tree->right()) report_subtree(tree->right(), qr);
    }

    size_t report_subtree(Node *tree)
    { 
        size_t result = 1;

        //recurse through tree
        if (tree->left()) result += report_subtree(tree->left());
        if (tree->right()) result += report_subtree(tree->right());

        return result;
    }


    std::vector<Point *> range_search(Node *tree, Number *range, Number *region, size_t depth)
    {
        std::vector<Point *> qr;

        //check for empty branch
        if (!tree) return qr;

        //leaf node
        if (point_in_range(tree->pt, range)) {
            qr.push_back(tree->pt);
        }

        //not leaf node
        if (tree->children) {

            std::vector<Point *> lqr, rqr;

            Number split_value = tree->median;

            //left subtree -- update region
            int changed_index = 2 * (depth % dim) + 1;

            Number changed_value = region[changed_index];    
            region[changed_index] = split_value; 

            if (range_contains_region(range, region)) {
                if (tree->left()) {
                    report_subtree(tree->left(), lqr);
                }
            } else if (region_intersects_range(region, range)) {
                lqr = range_search(tree->left(), range, region, depth+1); 
            }

            //restore region 
            region[changed_index] = changed_value; 

            //right subtree -- update region 
            changed_index = 2 * (depth % dim);
            changed_value = region[changed_index];    
            region[changed_index] = split_value; 

            if (range_contains_region(range, region)) {
                if (tree->right()) {
                    report_subtree(tree->right(), rqr);
                }
            } else if (region_intersects_range(region, range)) {
                rqr = range_search(tree->right(), range, region, depth+1); 
            }

            //restore region 
            region[changed_index] = changed_value; 

            //collect results
            qr.insert(qr.end(), lqr.begin(), lqr.end());
            qr.insert(qr.end(), rqr.begin(), rqr.end()); 
        }

        return qr; 
    }
    
    size_t range_count(Node *tree, Number *range, Number *region, size_t depth)
    {
        size_t qr = 0;

        //check for empty branch
        if (!tree) return qr;

        //leaf node
        if (point_in_range(tree->pt, range)) {
            ++qr;
        }

        //not leaf node
        if (tree->children) {

            size_t lqr = 0, rqr = 0;

            Number split_value = tree->median;

            //left subtree -- update region
            int changed_index = 2 * (depth % dim) + 1;

            Number changed_value = region[changed_index];    
            region[changed_index] = split_value; 

            if (range_contains_region(range, region)) {
                if (tree->left()) {
                    lqr = report_subtree(tree->left()); 
                }
            } else if (region_intersects_range(region, range)) {
                lqr = range_count(tree->left(), range, region, depth+1); 
            }

            //restore region 
            region[changed_index] = changed_value; 

            //right subtree -- update region 
            changed_index = 2 * (depth % dim);
            changed_value = region[changed_index];    
            region[changed_index] = split_value; 

            if (range_contains_region(range, region)) {
                if (tree->right()) {
                    rqr = report_subtree(tree->right()); 
                }
            } else if (region_intersects_range(region, range)) {
                rqr = range_count(tree->right(), range, region, depth+1); 
            }

            //restore region 
            region[changed_index] = changed_value; 

            //collect results
            qr += lqr + rqr;
        }

        return qr; 
    }
    
    void knn_search(FixedSizePriorityQueue<Node *> &resultpq,
        const Point &pt, Number eps)
    {
        searchpq.push(0, root);
        int number_popped = 0;

        ++total_searches;

        while (searchpq.length) {

            typename PriorityQueue<Node *>::Entry entry = searchpq.pop();
            ++number_popped;

            Node *node = entry.data;

            //need to square distance since resultpq distances are squared
            Number distance = entry.priority*entry.priority;

            if (!resultpq.full() || (1.0 + eps)*distance < resultpq.peek().priority) {

                while (node) {

                    //we only want to count nodes while backtracking
                    if (number_popped >= 2) {
                        ++knn_nodes_visited_backtrack;
                    } else {
                        ++knn_nodes_visited;
                    }

                    //calculate distance from query point to this point
                    Number distance = 0; 
                    for (int i = 0; i < dim; ++i) {
                        distance += ((*(node->pt))[i]-pt[i]) * ((*(node->pt))[i]-pt[i]); 
                    }

                    if (!resultpq.full() || distance < resultpq.peek().priority) {
                        resultpq.push(distance, node); 
                    }

                    if (pt[node->axis] < node->median) { 

                        if (node->right()) {
                            Number d = (1.0 + eps)*abs(node->median - pt[node->axis]);
                            if (d < resultpq.peek().priority) {
                                searchpq.push(abs(node->median - pt[node->axis]), node->right()); 
                            }
                        }

                        node = node->left(); 
                    } else {
                        if (node->left()) {
                            Number d = (1.0 + eps)*abs(node->median - pt[node->axis]);
                            if (d < resultpq.peek().priority) {
                                searchpq.push(abs(node->median - pt[node->axis]), node->left()); 
                            }
                        }

                        node = node->right();
                    } 
                } 
            } 
        } 
    } 
};

#endif

