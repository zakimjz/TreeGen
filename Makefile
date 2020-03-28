CC = g++
CFLAGS = -O3
LIBS = 
OBJS = dist.o ran1.o

default: tree_gen tree_classifier

clean:
	rm -rf *~ *.o $(default)

tree_gen: tree_gen.o $(OBJS)
	$(CC) $(CFLAGS) -o tree_gen tree_gen.o $(OBJS) $(LIBS)

tree_classifier: tree_classifier.o $(OBJS)
	$(CC) $(CFLAGS) -o tree_classifier tree_classifier.o $(OBJS)

.SUFFIXES: .o .cpp

.cpp.o:
	$(CC) $(CFLAGS) -c $<

# dependencies
# $(OBJS): $(HDRS)
ran1.o : ran1.cpp dist.h
dist.o : dist.cpp ran1.cpp dist.h
tree_gen.o : tree_gen.cpp tree_gen.h
tree_classifier.o : tree_classifier.cpp
