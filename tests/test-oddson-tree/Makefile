INCS = -I../../include
CFLAGS = -g -O2

all: kdtree quadtree 

kdtree: ../../include/oddson_tree.h ../../include/kdtree.h
	g++ -DODDSON_TREE_KDTREE_IMPLEMENTATION $(INCS) $(CFLAGS) main.cpp -o ../../bin/test-oddson-tree-kt

quadtree: ../../include/oddson_tree.h ../../include/compressed_quadtree.h
	g++ -DODDSON_TREE_QUADTREE_IMPLEMENTATION $(INCS) $(CFLAGS) main.cpp -o ../../bin/test-oddson-tree-qt

clean:
	rm *.o
