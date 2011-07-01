
INCS = -I../../include -I../../../kdtree/include
LIBS = 
CFLAGS = -g
LDFLAGS = -g
OBJS = render_tree.o
TARGET = ../../bin/render-tree

all: $(OBJS)
	g++ $(LDFLAGS) $(LIBS) $(OBJS) -o $(TARGET) 

.cpp.o:
	g++ $(INCS) $(CFLAGS) -c $< -o $@

render_tree.o: ../../include/oddson_tree.h

clean:
	rm *.o $(TARGET) 
