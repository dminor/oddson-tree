
INCS = -I../../include -I../../../kdtree/include
LIBS = 
CFLAGS = -g -O2
LDFLAGS = -L../../bin 
OBJS = main.o
TARGET = ../../bin/test-oddson-tree-zorder

all: $(OBJS)
	g++ $(LDFLAGS) $(LIBS) $(OBJS) -o $(TARGET) 

.cpp.o:
	g++ $(INCS) $(CFLAGS) -c $< -o $@

main.o: ../../include/oddson_tree_zorder.h ../../../kdtree/include/kdtree.h

clean:
	rm *.o $(TARGET) 
