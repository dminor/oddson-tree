INCS = -I../../include
CFLAGS = -g -O2

all: kdtree quadtree 

kdtree: ../../include/oddson_tree.h ../../include/kdtree.h
	g++ -DODDSON_TREE_KDTREE_IMPLEMENTATION $(INCS) $(CFLAGS) main.cpp -o ../../bin/knn-query-kt

quadtree: ../../include/oddson_tree.h ../../include/compressed_quadtree.h
	g++ -DODDSON_TREE_QUADTREE_IMPLEMENTATION $(INCS) $(CFLAGS) main.cpp -o ../../bin/knn-query-qt

clean:
	rm *.o
