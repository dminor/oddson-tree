
INCS = -I../../include -I../../../kdtree/include
LIBS = 
CFLAGS = -g -O2
LDFLAGS = -g
OBJS = render_tree.o
TARGET = ../../bin/render-tree-zorder

all: $(OBJS)
	g++ $(LDFLAGS) $(LIBS) $(OBJS) -o $(TARGET) 

.cpp.o:
	g++ $(INCS) $(CFLAGS) -c $< -o $@

render_tree.o: ../../include/oddson_tree_zorder.h

clean:
	rm *.o $(TARGET) 
