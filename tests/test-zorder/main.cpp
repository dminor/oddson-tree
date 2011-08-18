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

#include "zorder.h"

#include <float.h>
#include <cstdio>
#include <cstdlib>

struct Point {

    double v[2]; 
    int id;

    double &operator[](const int &index) 
    { 
        return v[index]; 
    } 

    const double &operator[](const int &index) const 
    { 
        return v[index]; 
    } 
};

Point *read_points(FILE *f, int *pt_count)
{
    char buf[80];
    fscanf(f, "%d", pt_count);
    fgets(buf, 80, f);

    if (pt_count < 0) {
        printf("error: invalid point count %d\n", *pt_count);
        return 0;
    }

    Point *pts = new Point[*pt_count];

    double x, y;
    for (int i = 0; i < *pt_count; ++i) {
        fscanf(f, "%lf, %lf", &x, &y);
        pts[i][0] = x;
        pts[i][1] = y; 
        pts[i].id = i;
    }

    return pts; 
}

int main(int argc, char **argv)
{ 
    if (argc != 3) {
        printf("usage: render_tree <points> <output>\n");
        exit(1);
    }

    int n;

    FILE *f = fopen(argv[1], "r");

    if (!f) {
        printf("error: could not open sample file: %s\n", argv[2]);
        exit(1); 
    }

    Point *sample = read_points(f, &n);
    if (!sample) exit(1);

    fclose(f);

    ZOrder<Point, double> comp(2);
    std::sort(&sample[0], &sample[n], comp);

    f = fopen(argv[2], "w");
    if (!f) {
        printf("error: could not open output file: %s\n", argv[2]);
        exit(1); 
    }

    fprintf(f, "%\n");

    //define point function for later
    fprintf(f, "/draw-point {\n");
    fprintf(f, "    /y exch def\n");
    fprintf(f, "    /x exch def\n");
    fprintf(f, "    gsave\n");
    fprintf(f, "    newpath\n");
    fprintf(f, "    1.0 0.5 0.7 setrgbcolor\n");
    fprintf(f, "    x y 1 0 360 arc\n");
    fprintf(f, "    closepath\n");
    fprintf(f, "    stroke\n");
    fprintf(f, "    grestore\n");
    fprintf(f, "} def\n");

    //draw sample points
    for (int i = 0; i < n; ++i) {
        fprintf(f, "%.1f %.1f draw-point\n", sample[i][0], sample[i][1]);
    }

    //draw lines between points adjacent in z-order
    fprintf(f, "0.7 0.1 0.1 setrgbcolor\n");
    fprintf(f, "newpath\n");
    fprintf(f, "%.1f %.1f moveto\n", sample[1][0], sample[1][1]);
    for (int i = 1; i < n; ++i) {
        fprintf(f, "%.1f %.1f lineto\n", sample[i][0], sample[i][1]);
    }
    fprintf(f, "stroke \n");

    fclose(f);

    delete[] sample; 

}
