
#include <fstream>
#include <iostream>

#include <boost/polygon/point_data.hpp>
#include <boost/polygon/voronoi.hpp>

using boost::polygon::voronoi_diagram;
using boost::polygon::construct_voronoi;
typedef boost::polygon::point_data<double> Point;

int main(int argc, char **argv)
{
    if (argc < 2) {
        std::cerr << "usage: voronoi <pts> [scale]" << std::endl;
        exit(1);
    }

    double scale = 1.0;
    if (argc == 3) {
        scale = atof(argv[2]);
    }

    //open point  file
    std::ifstream ptf(argv[1]);

    if (!ptf) {
        std::cerr << "error: could not open points file: " << argv[1] << std::endl;
        exit(1);
    }

    //read point count
    int pt_count;
    ptf >> pt_count;

    ptf.ignore(80, '\n');

    if (pt_count < 0) {
        std::cerr << "error: invalid point count " << pt_count << std::endl;
        exit(1);
    }

    //read points and insert into voronoi diagram
    std::vector<Point> points;

    double x, y;
    double x1 = std::numeric_limits<double>::max(), x2 = -std::numeric_limits<double>::max();
    double y1 = std::numeric_limits<double>::max(), y2 = -std::numeric_limits<double>::max();
    for (int i = 0; i < pt_count; ++i) {
        char c;
        ptf >> x;
        ptf >> c;
        ptf >> y;
        x = ceil(x*scale);
        y = ceil(y*scale);
        points.push_back(Point(x, y));

        //set up bounds
        if (x < x1) x1 = x;
        if (x > x2) x2 = x;
        if (y < y1) y1 = y;
        if (y > y2) y2 = y;
    }

    ptf.close();

    //generate voronoi diagram
    voronoi_diagram<double> vd;
    construct_voronoi(points.begin(), points.end(), &vd);

    //output to postscript
    std::cout << "%\n";

    //center
    //TODO: make this a parameter
    std::cout << 0.5*scale << " " << 0.5*scale << " translate";

    //define point function for later
    std::cout << "/draw-point {\n";
    std::cout << "    /y exch def\n";
    std::cout << "    /x exch def\n";
    std::cout << "    gsave\n";
    std::cout << "    newpath\n";
    std::cout << "    0.5 0.5 0.7 setrgbcolor\n";
    std::cout << "    x y 2 0 360 arc\n";
    std::cout << "    closepath\n";
    std::cout << "    fill\n";
    std::cout << "    newpath\n";
    std::cout << "    0.4 setgray\n";
    std::cout << "    x y 2 0 360 arc\n";
    std::cout << "    closepath\n";
    std::cout << "    stroke\n";
    std::cout << "    grestore\n";
    std::cout << "} def\n";

    //line
    std::cout << "/draw-line {\n";
    std::cout << "    /y2 exch def\n";
    std::cout << "    /x2 exch def\n";
    std::cout << "    /y1 exch def\n";
    std::cout << "    /x1 exch def\n";
    std::cout << "    gsave\n";
    std::cout << "    0.7 setgray\n";
    std::cout << "    newpath\n";
    std::cout << "    x1 y1 moveto\n";
    std::cout << "    x2 y2 lineto\n";
    std::cout << "    closepath\n";
    std::cout << "    stroke \n";
    std::cout << "    grestore\n";
    std::cout << "} def\n";

    //draw edges
    for (boost::polygon::voronoi_diagram<double>::const_vertex_iterator it = vd.vertices().begin();
        it != vd.vertices().end(); ++it) {
        const voronoi_diagram<double>::vertex_type &vertex = *it;
        const voronoi_diagram<double>::edge_type *edge = vertex.incident_edge();
        do {
            const voronoi_diagram<double>::vertex_type *v0 = edge->vertex0();
            const voronoi_diagram<double>::vertex_type *v1 = edge->vertex1();
         
            //vertices at infinity are NULL
            //TODO: intersect infinite line with bounding box
            if (v0 && v1) {
                std::cout << v0->x() << " " << v0->y() << " ";
                std::cout << v1->x() << " " << v1->y() << " draw-line\n";
            }

            edge = edge->rot_next();
        } while (edge != vertex.incident_edge());
    }

    //draw points
    for (std::vector<Point>::iterator it = points.begin(); it != points.end(); ++it) {
        std::cout << it->x() << " " << it->y() << " draw-point\n";
    }

    return 0;
}

