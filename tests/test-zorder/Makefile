
INCS = -I../../include -I../../../kdtree/include
LIBS = 
CFLAGS = -g -O2
LDFLAGS = -g
OBJS = main.o
TARGET = ../../bin/test-zorder

all: $(OBJS)
	g++ $(LDFLAGS) $(LIBS) $(OBJS) -o $(TARGET) 

.cpp.o:
	g++ $(INCS) $(CFLAGS) -c $< -o $@

main.o: ../../include/zorder.h

clean:
	rm *.o $(TARGET) 
