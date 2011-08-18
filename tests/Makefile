
DIRS = test-oddson-tree voronoi render-tree test-zorder

all:
	for dir in $(DIRS); do cd $$dir; make; cd ..; done

clean:
	for dir in $(DIRS); do cd $$dir; make clean; cd ..; done

