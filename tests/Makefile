
DIRS = test-oddson-tree render-tree kdtree-knn-query knn-query 

all:
	for dir in $(DIRS); do cd $$dir; make; cd ..; done

clean:
	for dir in $(DIRS); do cd $$dir; make clean; cd ..; done

