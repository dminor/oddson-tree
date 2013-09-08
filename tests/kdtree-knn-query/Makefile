
INCS = -I../../include 
LIBS = -lrt
CFLAGS = -g -O2
LDFLAGS = -L../../bin 
OBJS = main.o
TARGET = ../../bin/kdtree-knn-query

all: $(OBJS)
	g++ $(LDFLAGS) $(OBJS) -o $(TARGET) $(LIBS)

.cpp.o:
	g++ $(INCS) $(CFLAGS) -c $< -o $@

knn.o: ../../include/kdtree.h

clean:
	rm *.o $(TARGET) 
