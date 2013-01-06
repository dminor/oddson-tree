
INCS = -I../../include
CFLAGS = -g

all: kdtree quadtree 

kdtree: ../../include/oddson_tree.h ../../include/kdtree.h
	g++ -DODDSON_TREE_KDTREE_IMPLEMENTATION $(INCS) $(CFLAGS) render_tree.cpp -o ../../bin/render-tree-kt

quadtree: ../../include/oddson_tree.h ../../include/compressed_quadtree.h
	g++ -DODDSON_TREE_QUADTREE_IMPLEMENTATION $(INCS) $(CFLAGS) render_tree.cpp -o ../../bin/render-tree-qt

clean:
	rm *.o
