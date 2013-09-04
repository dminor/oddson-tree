
INCS = -I../../include 
LIBS = 
CFLAGS = -g -O2
LDFLAGS = -L../../bin 
OBJS = main.o
TARGET = ../../bin/kdtree-knn-query

all: $(OBJS)
	g++ $(LDFLAGS) $(LIBS) $(OBJS) -o $(TARGET) 

.cpp.o:
	g++ $(INCS) $(CFLAGS) -c $< -o $@

knn.o: ../../include/kdtree.h

clean:
	rm *.o $(TARGET) 
