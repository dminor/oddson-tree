
INCS = -I ../inc
LIBS = -lCGAL -lgmp
CFLAGS = -g -O2
OBJS = voronoi.o
TARGET = ../../bin/voronoi

all: $(OBJS)
	g++ $(LDFLAGS) $(LIBS) $(OBJS) -o $(TARGET) 

.cpp.o:
	g++ $(INCS) $(CFLAGS) -c $< -o $@

clean:
	rm *.o $(TARGET) 

