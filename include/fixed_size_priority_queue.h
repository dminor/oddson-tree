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

#ifndef FIXED_SIZE_PRIORITY_QUEUE_H_
#define FIXED_SIZE_PRIORITY_QUEUE_H_

#include <cstdlib>

template<class T> class FixedSizePriorityQueue {

public:

    struct Entry {
        double priority;
        T data;
    };

    FixedSizePriorityQueue(int size) : length(0), size(size)
    {
        //need room for dummy entries[0]
        entries = new Entry[size + 1]; 
    }

    virtual ~FixedSizePriorityQueue()
    {
        delete[] entries;
    }

    void push(double priority, const T &data)
    {

        //avoid duplicates. this obviously isn't very efficient, but it is
        //still faster than using a std::set for typical sizes.
        //this should be replaced with a custom hash table.
        for (int i = 1; i <= length; ++i) {
            if (entries[i].priority == priority) {
                return;
            }
        }

        //make room if necessary
        if (full()) pop();
    
        //adjust heap length
        ++length;

        //add to end of heap
        entries[length].priority = priority;
        entries[length].data = data;
      
        //place new entry in proper position in heap 
        size_t i = length; 
        size_t parent = i >> 1;
        while (i != 1 && entries[i].priority > entries[parent].priority) { 
            Entry t = entries[i];
            entries[i] = entries[parent];
            entries[parent] = t; 

            i = parent;
            parent = i >> 1;
        } 
    }

    Entry pop()
    {
        Entry min = entries[1]; 
        entries[1] = entries[length]; 
        --length;
        heapify(1); 

        return min; 
    }

    const Entry &peek()
    { 
        return entries[1]; 
    }

    bool full()
    {
        return length == size;
    }

    size_t length;

private:

    Entry *entries;
    size_t size;

    void heapify(size_t i)
    { 
        size_t l = i << 1;
        size_t r = l + 1;
        size_t smallest = i;

        if (l <= length && entries[l].priority > entries[i].priority) {
            smallest = l;
        } 

        if (r <= length && entries[r].priority > entries[smallest].priority) {
            smallest = r;
        }

        if (smallest != i) {
            Entry t = entries[i];
            entries[i] = entries[smallest];
            entries[smallest] = t; 

            heapify(smallest);
        } 
    } 
};

#endif

