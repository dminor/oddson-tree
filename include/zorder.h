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

#ifndef ZORDER_H_
#define ZORDER_H_

#include <algorithm>
#include <limits>
#include <cstdlib>

#include <stdint.h>

template<class P, class T> struct ZOrder {

	int dim;

	ZOrder(int dim) : dim(dim)
	{
	}

	bool operator()(const P &a, const P &b)
	{
		int j = 0;
		T x = 0;

		for (int k = 0; k < dim; ++k) {
			T y = a[k] ^ b[k];
			if (x < y && x < (x ^ y)) {
				j = k;
				x = y;
			}
		}

		return a[j] < b[j];
	}

};

template<class P> struct ZOrder<P, double> {

	int dim;

	ZOrder(int dim) : dim(dim)
	{
	}

	/*
	Return exponent of highest order bit that differs in a and b
	*/
	int msb(uint64_t a, uint64_t b)
	{
		if (a == b) {
			return std::numeric_limits<int>::min();
		} else {

			uint64_t z = a ^ b;

			int count = 53;
			while (z) {
				z >>= 1;
				--count;
			}

			return count;
		}

	}

	int sign(const double &d)
	{
		char *c = (char *)&d;
		return (c[7] & 0x80) >> 7;
	}

	int exponent(const double &d)
	{
		char *c = (char *)&d;
        short s = c[7];
        s <<= 8;
        s |= c[6]; 
		return ((s & 0x7FF0) >> 4);
	}

	uint64_t mantissa(const double &d)
	{
		return *((uint64_t *)&d) & 0x000FFFFFFFFFFFFF;
	}

	int xor_msb(double a, double b)
	{
		int exp_a = exponent(a);
		int exp_b = exponent(b);

		if (exp_a == exp_b) {
			return exp_a - msb(mantissa(a), mantissa(b));
		}

		if (exp_b < exp_a) return exp_a;
		else return exp_b;
	}

	bool operator()(const P &a, const P &b)
	{
		int j = 0;
		int x = 0;

		for (int k = 0; k < dim; ++k) {
			int y = xor_msb(a[k], b[k]);
			if (x < y) {
				j = k;
				x = y;
			}
		}

		return a[j] < b[j];
	}

};

#endif
