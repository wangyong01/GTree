.PHONY : all clean

all : bin/gtree-build bin/gtree-query

bin/gtree-build : bin src/gtree-build.cc
	g++ -std=c++11 src/gtree-build.cc -o bin/gtree-build -lmetis

bin/gtree-query : bin src/gtree-query.cc
	g++ src/gtree-query.cc -o bin/gtree-query

bin :
	mkdir -p bin

clean :
	rm -f bin/*

