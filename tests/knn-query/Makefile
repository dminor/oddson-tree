
INCS = -I../../include -I../../../kdtree/include
LIBS = 
CFLAGS = -g -O1
LDFLAGS = -L../../bin 
OBJS = main.o
TARGET = ../../bin/knn-query

all: $(OBJS)
	g++ $(LDFLAGS) $(LIBS) $(OBJS) -o $(TARGET) 

.cpp.o:
	g++ $(INCS) $(CFLAGS) -c $< -o $@

main.o: ../../include/oddson_tree.h ../../include/zorder.h


clean:
	rm *.o $(TARGET) 
